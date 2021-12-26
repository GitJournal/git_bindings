#include "gitjournal.h"

#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <string.h>

#include <git2.h>

#define UNUSED(x) (void)(x)

typedef struct
{
    bool first_time;
    int error_code;
    bool ssh_in_memory;
    char *status_file;
} gj_credentials_payload;

int match_cb(const char *path, const char *spec, void *payload)
{
    UNUSED(spec);
    UNUSED(payload);

    gj_log_internal("Add Match: %s\n", path);
    return 0;
}

int gj_git_add(const char *git_base_path, const char *pattern)
{
    int err;
    git_repository *repo = NULL;
    git_index *index = NULL;

    err = git_repository_open(&repo, git_base_path);
    if (err < 0)
        goto cleanup;

    err = git_repository_index(&index, repo);
    if (err < 0)
        goto cleanup;

    char *paths[] = {(char *)pattern};
    git_strarray pathspec = {paths, 1};

    err = git_index_add_all(index, &pathspec, GIT_INDEX_ADD_DEFAULT, match_cb, NULL);
    if (err < 0)
        goto cleanup;

    err = git_index_write(index);
    if (err < 0)
        goto cleanup;

cleanup:
    git_index_free(index);
    git_repository_free(repo);

    return err;
}

int rm_match_cb(const char *path, const char *spec, void *payload)
{
    UNUSED(spec);
    UNUSED(payload);

    char *git_base_path = (char *)payload;
    if (!git_base_path)
    {
        gj_log_internal("git_base_path not in payload. Why?\n");
        return 1;
    }

    int full_path_length = strlen(git_base_path) + 1 + strlen(path);
    char *full_path = (char *)malloc(full_path_length);
    if (full_path == NULL)
    {
        gj_log_internal("rm_match_cb: Malloc Failed");
        return 1;
    }
    strcpy(full_path, git_base_path);
    strcat(full_path, "/"); // FIXME: Will not work on windows!
    strcat(full_path, path);

    gj_log_internal("Removing File: %s\n", full_path);
    int err = remove(full_path);
    if (err != 0)
    {
        gj_log_internal("File could not be deleted: %s %d\n", full_path, errno);
        if (errno == ENOENT)
        {
            gj_log_internal("ENOENT\n");
        }
    }

    free(full_path);
    return 0;
}

int gj_git_rm(const char *git_base_path, const char *pattern)
{
    int err;
    git_repository *repo = NULL;
    git_index *index = NULL;

    err = git_repository_open(&repo, git_base_path);
    if (err < 0)
        goto cleanup;

    err = git_repository_index(&index, repo);
    if (err < 0)
        goto cleanup;

    char *paths[] = {(char *)pattern};
    git_strarray pathspec = {paths, 1};

    gj_log_internal("Calling git rm with pathspec %s", pattern);
    err = git_index_remove_all(index, &pathspec, rm_match_cb, (void *)git_base_path);
    if (err < 0)
        goto cleanup;

    err = git_index_write(index);
    if (err < 0)
        goto cleanup;

cleanup:
    git_index_free(index);
    git_repository_free(repo);

    return err;
}

int gj_git_reset_hard(const char *git_base_path, const char *ref)
{
    int err = 0;
    git_repository *repo = NULL;
    git_object *obj = NULL;

    err = git_repository_open(&repo, git_base_path);
    if (err < 0)
        goto cleanup;

    err = git_revparse_single(&obj, repo, ref);
    if (err < 0)
        goto cleanup;

    err = git_reset(repo, obj, GIT_RESET_HARD, NULL);
    if (err < 0)
        goto cleanup;

cleanup:
    git_object_free(obj);
    git_repository_free(repo);

    return err;
}

int gj_git_commit(const char *git_base_path, const char *author_name,
                  const char *author_email, const char *message, long long commit_time, int commit_time_offset)
{
    int err = 0;
    git_signature *sig = NULL;
    git_index *index = NULL;
    git_oid tree_id, commit_id;
    git_tree *tree = NULL;
    git_repository *repo = NULL;
    git_oid parent_id;
    git_commit *parent_commit = NULL;

    err = git_repository_open(&repo, git_base_path);
    if (err < 0)
        goto cleanup;

    err = git_repository_index(&index, repo);
    if (err < 0)
        goto cleanup;

    /*
    FIXME: This returns 0 when a file is set to be removed
            How do we figure out when the index is empty?
    int numOps = git_index_entrycount(index);
    if (numOps == 0)
    {
        err = GJ_ERR_EMPTY_COMMIT;
        goto cleanup;
    }
    */

    if (commit_time == 0)
    {
        err = git_signature_now(&sig, author_name, author_email);
        if (err < 0)
            goto cleanup;
    }
    else
    {
        err = git_signature_new(&sig, author_name, author_email, commit_time, commit_time_offset);
        if (err < 0)
            goto cleanup;
    }

    err = git_index_write_tree(&tree_id, index);
    if (err < 0)
        goto cleanup;

    err = git_tree_lookup(&tree, repo, &tree_id);
    if (err < 0)
        goto cleanup;

    err = git_reference_name_to_id(&parent_id, repo, "HEAD");
    if (err < 0)
    {
        if (err != GIT_ENOTFOUND)
            goto cleanup;

        git_error_clear();

        err = git_commit_create(&commit_id, repo, "HEAD", sig, sig, NULL, message, tree, 0, NULL);
        if (err < 0)
            goto cleanup;
    }
    else
    {
        err = git_commit_lookup(&parent_commit, repo, &parent_id);
        if (err < 0)
            goto cleanup;

        const git_commit *parents = {parent_commit};
        err = git_commit_create(&commit_id, repo, "HEAD", sig, sig, NULL, message, tree, 1, &parents);
        if (err < 0)
            goto cleanup;
    }

cleanup:
    git_index_free(index);
    git_commit_free(parent_commit);
    git_tree_free(tree);
    git_repository_free(repo);
    git_signature_free(sig);

    return err;
}

int fetch_progress(const git_transfer_progress *stats, void *payload)
{
    int fetch_percent =
        (100 * stats->received_objects) /
        stats->total_objects;
    int index_percent =
        (100 * stats->indexed_objects) /
        stats->total_objects;
    int kbytes = stats->received_bytes / 1024;

    gj_log_internal("network %3d%% (%4d kb, %5d/%5d)  /"
                    "  index %3d%% (%5d/%5d)\n",
                    fetch_percent, kbytes,
                    stats->received_objects, stats->total_objects,
                    index_percent,
                    stats->indexed_objects, stats->total_objects);

    gj_credentials_payload *gj_payload = (gj_credentials_payload *)payload;
    if (gj_payload != 0)
    {
        if (gj_payload->status_file != 0)
        {
            FILE *fp = fopen(gj_payload->status_file, "w");
            char str[180];
            memset(str, 0, sizeof(str));
            sprintf(str, "%d %d %d %d %d %d %d \n", stats->total_objects, stats->indexed_objects, stats->received_objects, stats->local_objects, stats->total_deltas, stats->indexed_deltas, (int)stats->received_bytes);
            fwrite(str, 1, sizeof(str), fp);
            fclose(fp);
        }
    }

    return 0;
}

char *g_public_key = NULL;
char *g_private_key = NULL;
char *g_public_key_path = NULL;
char *g_private_key_path = NULL;
char *g_passcode = NULL;

int write_to_path(char *path, char *value)
{
    FILE *fptr = fopen(path, "w");

    if (fptr == NULL)
    {
        return -1;
    }

    fprintf(fptr, "%s\n", value);
    fclose(fptr);
    return 0;
}

char *build_path(char *base_url, char *filename)
{
    char *str = malloc(strlen(base_url) + strlen(filename) + 10);
    strcpy(str, base_url);
    strcat(str, "/.git/");
    strcat(str, filename);

    return str;
}

void write_keys_to_file(char *base_url)
{
    if (g_public_key_path != NULL)
    {
        free(g_public_key_path);
        g_public_key_path = NULL;
    }
    if (g_private_key_path != NULL)
    {
        free(g_private_key_path);
        g_private_key_path = NULL;
    }

    g_private_key_path = build_path(base_url, "id_ed25519");
    g_public_key_path = build_path(base_url, "id_ed25519.pub");

    gj_log_internal("Public Key Path: %s\n", g_public_key_path);
    gj_log_internal("Private Key Path: %s\n", g_private_key_path);

    write_to_path(g_public_key_path, g_public_key);
    write_to_path(g_private_key_path, g_private_key);

    chmod(g_private_key_path, 0600);
}

void delete_keys_from_file()
{
    if (g_public_key_path != NULL)
    {
        remove(g_public_key_path);

        free(g_public_key_path);
        g_public_key_path = NULL;
    }

    if (g_private_key_path != NULL)
    {
        remove(g_private_key_path);

        free(g_private_key_path);
        g_private_key_path = NULL;
    }
}

bool file_exists(char *filename)
{
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

int credentials_cb(git_cred **out, const char *url, const char *username_from_url,
                   unsigned int allowed_types, void *payload)
{
    if (!payload)
    {
        gj_log_internal("credentials_cb has no payload\n");
        return -1;
    }
    gj_credentials_payload *gj_payload = (gj_credentials_payload *)payload;
    if (!gj_payload->first_time)
    {
        gj_payload->error_code = GJ_ERR_INVALID_CREDENTIALS;
        gj_log_internal("GitJournal: Credentials have been tried and they failed\n");
        return GJ_ERR_INVALID_CREDENTIALS;
    }

    gj_log_internal("Url: %s\n", url);

    if (!((allowed_types & GIT_CREDTYPE_SSH_KEY) | (allowed_types & GIT_CREDTYPE_USERPASS_PLAINTEXT)))
    {
        gj_log_internal("Some other auth mechanism is being used: %d\n", allowed_types);
        return -1;
    }

    if (gj_payload->ssh_in_memory)
    {
        if (strlen(g_public_key) == 0)
        {
            gj_payload->error_code = GJ_ERR_MISSING_PUBLIC_KEY;
            gj_log_internal("Public Key is empty\n");
            return GJ_ERR_MISSING_PUBLIC_KEY;
        }

        if (strlen(g_private_key) == 0)
        {
            gj_payload->error_code = GJ_ERR_MISSING_PRIVATE_KEY;
            gj_log_internal("Private Key is empty\n");
            return GJ_ERR_MISSING_PRIVATE_KEY;
        }

        gj_payload->first_time = false;
        return git_cred_ssh_key_memory_new(out, username_from_url,
                                           g_public_key, g_private_key, g_passcode);
    }
    else
    {
        if (strlen(g_public_key_path) == 0)
        {
            gj_payload->error_code = GJ_ERR_MISSING_PUBLIC_KEY;
            gj_log_internal("Public Key is empty\n");
            return GJ_ERR_MISSING_PUBLIC_KEY;
        }

        if (strlen(g_private_key_path) == 0)
        {
            gj_payload->error_code = GJ_ERR_MISSING_PRIVATE_KEY;
            gj_log_internal("Private Key is empty\n");
            return GJ_ERR_MISSING_PRIVATE_KEY;
        }

        gj_payload->first_time = false;
        return git_cred_ssh_key_new(out, username_from_url,
                                    g_public_key_path, g_private_key_path, g_passcode);
    }
}

int certificate_check_cb(git_cert *cert, int valid, const char *host, void *payload)
{
    gj_log_internal("Valid: %d\n", valid);
    gj_log_internal("CertType: %d\n", cert->cert_type);

    if (valid == 0)
    {
        gj_log_internal("%s: Invalid certificate\n", host);
    }

    if (cert->cert_type == GIT_CERT_HOSTKEY_LIBSSH2)
    {
        gj_log_internal("LibSSH2 Key: %p\n", payload);
        return 0;
    }
    if (cert->cert_type == GIT_CERT_X509)
    {
        gj_log_internal("Cert X509 Key: %p\n", payload);
        return 0;
    }

    // FIXME: We should be checking the certificate
    gj_log_internal("Unknown Certificate Accepted\n");
    return 0;
}

int gj_git_push(const char *git_base_path, const char *remote_name, char *public_key, char *private_key, char *passcode, bool ssh_in_memory, char *status_file)
{
    g_public_key = public_key;
    g_private_key = private_key;
    g_passcode = passcode;

    if (!ssh_in_memory)
    {
        write_keys_to_file((char *)git_base_path);
    }

    int err = 0;
    git_repository *repo = NULL;
    git_remote *remote = NULL;
    git_reference *ref_head = NULL;
    char *name = NULL;

    err = git_repository_open(&repo, git_base_path);
    if (err < 0)
        goto cleanup;

    err = git_remote_lookup(&remote, repo, remote_name);
    if (err < 0)
        goto cleanup;

    // Get current branch
    err = git_repository_head(&ref_head, repo);
    if (err < 0)
        goto cleanup;

    git_reference_t head_type = git_reference_type(ref_head);
    if (head_type != GIT_REFERENCE_DIRECT)
    {
        // vHanda: Why are you a direct reference!!
        gj_log_internal("git push: head is not a direct ref: %d\n", head_type);
        goto cleanup;
    }

    char *ref_head_name = (char *)git_reference_name(ref_head);
    const git_strarray refs = {&ref_head_name, 1};

    git_push_options options = GIT_PUSH_OPTIONS_INIT;

    if (status_file != 0 && strlen(status_file) == 0)
    {
        status_file = 0;
    }

    gj_credentials_payload gj_payload = {true, 0, ssh_in_memory, status_file};
    options.callbacks.payload = (void *)&gj_payload;
    options.callbacks.credentials = credentials_cb;

    err = git_remote_push(remote, &refs, &options);
    if (err < 0)
        goto cleanup;

cleanup:
    free(name);
    git_reference_free(ref_head);
    git_remote_free(remote);
    git_repository_free(repo);

    if (!ssh_in_memory)
    {
        delete_keys_from_file();
    }

    return err;
}

int gj_git_default_branch(const char *git_base_path, const char *remote_name, char *public_key, char *private_key, char *passcode, bool ssh_in_memory, char *default_branch)
{
    g_public_key = public_key;
    g_private_key = private_key;
    g_passcode = passcode;

    if (!ssh_in_memory)
    {
        write_keys_to_file((char *)git_base_path);
    }

    int err = 0;
    git_repository *repo = NULL;
    git_remote *remote = NULL;
    git_buf buf = {NULL};

    err = git_repository_open(&repo, git_base_path);
    if (err < 0)
        goto cleanup;

    err = git_remote_lookup(&remote, repo, remote_name);
    if (err < 0)
        goto cleanup;

    git_remote_callbacks callbacks = GIT_REMOTE_CALLBACKS_INIT;

    gj_credentials_payload gj_payload = {true, 0, ssh_in_memory, 0};
    callbacks.payload = (void *)&gj_payload;
    callbacks.credentials = credentials_cb;

    err = git_remote_connect(remote, GIT_DIRECTION_FETCH, &callbacks, NULL, NULL);
    if (err < 0)
        goto cleanup;

    err = git_remote_default_branch(&buf, remote);
    if (err == GIT_ENOTFOUND)
    {
        default_branch[0] = 0;
    }
    else if (err < 0)
        goto cleanup;
    else
    {
        strncpy(default_branch, buf.ptr, buf.size);
        default_branch[buf.size] = 0;
        gj_log_internal("git_default_branch %s : %d\n", default_branch, buf.size);
    }
cleanup:
    git_buf_dispose(&buf);
    git_remote_free(remote);
    git_repository_free(repo);

    if (!ssh_in_memory)
    {
        delete_keys_from_file();
    }

    return err;
}

// Taken from merge.c libgit2 examples
static int perform_fastforward(git_repository *repo, const git_oid *target_oid)
{
    git_checkout_options ff_checkout_options = GIT_CHECKOUT_OPTIONS_INIT;
    git_reference *target_ref = NULL;
    git_reference *new_target_ref = NULL;
    git_object *target = NULL;
    int err = 0;

    err = git_repository_head(&target_ref, repo);
    if (err != 0)
        goto cleanup;

    /* Lookup the target object */
    err = git_object_lookup(&target, repo, target_oid, GIT_OBJECT_COMMIT);
    if (err != 0)
        goto cleanup;

    /* Checkout the result so the workdir is in the expected state */
    ff_checkout_options.checkout_strategy = GIT_CHECKOUT_SAFE;
    err = git_checkout_tree(repo, target, &ff_checkout_options);
    if (err != 0)
        goto cleanup;

    /* Move the target reference to the target OID */
    err = git_reference_set_target(&new_target_ref, target_ref, target_oid, NULL);
    if (err != 0)
        goto cleanup;

cleanup:
    git_reference_free(target_ref);
    git_reference_free(new_target_ref);
    git_object_free(target);

    return err;
}

void gj_resolve_conflict(const char *git_base_path, git_index_entry *ancesstor_index,
                         git_index_entry *our_index, git_index_entry *their_index)
{
    gj_log_internal("gj_resolve_conflict %x %x %x\n", ancesstor_index, our_index, their_index);

    int err = 0;
    git_odb *odb = NULL;
    char *objects_path = NULL;
    git_odb_object *odb_object = NULL;
    char *file_full_path = NULL;
    FILE *file = NULL;

    char *objects_suffix = "/.git/objects";
    objects_path = malloc(strlen(git_base_path) + strlen(objects_suffix));
    objects_path[0] = 0;

    strcat(objects_path, git_base_path);
    strcat(objects_path, objects_suffix);

    err = git_odb_open(&odb, objects_path);
    if (err != 0)
        goto cleanup;

    err = git_odb_read(&odb_object, odb, &our_index->id);
    if (err != 0)
        goto cleanup;

    // We should check that it is a blob
    const char *file_data = (char *)git_odb_object_data(odb_object);

    const char *file_path = our_index->path;
    file_full_path = malloc(strlen(git_base_path) + 1 + strlen(file_path));
    file_full_path[0] = 0;

    strcat(file_full_path, git_base_path);
    strcat(file_full_path, "/");
    strcat(file_full_path, file_path);

    file = fopen(file_full_path, "w");
    fwrite((void *)file_data, strlen(file_data), 1, file);

cleanup:
    if (file != NULL)
        fclose(file);
    free(file_full_path);
    git_odb_object_free(odb_object);
    free(objects_path);
    git_odb_free(odb);
}

int gj_git_fetch(const char *git_base_path, const char *remote_name, char *public_key, char *private_key, char *passcode, bool ssh_in_memory, char *status_file)
{
    g_public_key = public_key;
    g_private_key = private_key;
    g_passcode = passcode;

    if (!ssh_in_memory)
    {
        write_keys_to_file((char *)git_base_path);
    }

    int err = 0;
    git_repository *repo = NULL;
    git_remote *remote = NULL;

    err = git_repository_open(&repo, git_base_path);
    if (err < 0)
        goto cleanup;

    git_repository_state_t state = git_repository_state(repo);
    if (state != GIT_REPOSITORY_STATE_NONE)
    {
        err = GJ_ERR_PULL_INVALID_STATE;
        goto cleanup;
    }

    err = git_remote_lookup(&remote, repo, remote_name);
    if (err < 0)
        goto cleanup;

    git_fetch_options options = GIT_FETCH_OPTIONS_INIT;

    if (status_file != 0 && strlen(status_file) == 0)
    {
        status_file = 0;
    }

    gj_credentials_payload gj_payload = {true, 0, ssh_in_memory, status_file};
    options.callbacks.payload = (void *)&gj_payload;
    options.callbacks.credentials = credentials_cb;
    options.callbacks.transfer_progress = fetch_progress;

    err = git_remote_fetch(remote, NULL, &options, NULL);
    if (err < 0)
        goto cleanup;

cleanup:
    git_remote_free(remote);
    git_repository_free(repo);

    if (!ssh_in_memory)
    {
        delete_keys_from_file();
    }

    return err;
}

int gj_git_clone(const char *clone_url, const char *git_base_path, char *public_key, char *private_key, char *passcode, bool ssh_in_memory, char *status_file)
{
    g_public_key = public_key;
    g_private_key = private_key;
    g_passcode = passcode;

    if (!ssh_in_memory)
    {
        write_keys_to_file((char *)git_base_path);
    }

    int err = 0;
    git_repository *repo = NULL;
    git_clone_options options = GIT_CLONE_OPTIONS_INIT;
    options.fetch_opts.callbacks.transfer_progress = fetch_progress;
    options.fetch_opts.callbacks.certificate_check = certificate_check_cb;

    if (status_file != 0 && strlen(status_file) == 0)
    {
        status_file = 0;
    }

    gj_credentials_payload gj_payload = {true, 0, ssh_in_memory, status_file};
    options.fetch_opts.callbacks.payload = (void *)&gj_payload;
    options.fetch_opts.callbacks.credentials = credentials_cb;

    err = git_clone(&repo, clone_url, git_base_path, &options);
    if (err < 0)
        goto cleanup;

cleanup:
    git_repository_free(repo);

    if (!ssh_in_memory)
    {
        delete_keys_from_file();
    }

    if (gj_payload.error_code != 0)
        return gj_payload.error_code;
    return err;
}

int gj_git_merge(const char *git_base_path, const char *source_branch, const char *author_name, const char *author_email)
{
    int err = 0;
    git_repository *repo = NULL;
    git_annotated_commit *origin_annotated_commit = NULL;
    git_reference *head_ref = NULL;
    git_reference *origin_head_ref = NULL;
    git_index *index = NULL;
    git_index_conflict_iterator *conflict_iter = NULL;
    git_signature *sig = NULL;

    git_commit *head_commit = NULL;
    git_commit *origin_head_commit = NULL;
    git_tree *tree = NULL;
    git_oid tree_id;
    char *name = NULL;

    err = git_repository_open(&repo, git_base_path);
    if (err < 0)
        goto cleanup;

    git_repository_state_t state = git_repository_state(repo);
    if (state != GIT_REPOSITORY_STATE_NONE)
    {
        err = GJ_ERR_PULL_INVALID_STATE;
        goto cleanup;
    }

    err = git_repository_head(&head_ref, repo);
    if (err < 0)
        goto cleanup;

    err = git_branch_lookup(&origin_head_ref, repo, source_branch, GIT_BRANCH_REMOTE);
    if (err < 0)
        goto cleanup;

    err = git_annotated_commit_from_ref(&origin_annotated_commit, repo, origin_head_ref);
    if (err < 0)
        goto cleanup;

    git_merge_analysis_t merge_analysis;
    git_merge_preference_t merge_preference;

    git_annotated_commit *annotated_commits[] = {origin_annotated_commit};
    err = git_merge_analysis(&merge_analysis, &merge_preference, repo,
                             (const git_annotated_commit **)annotated_commits, 1);
    if (err < 0)
        goto cleanup;

    if (merge_analysis & GIT_MERGE_ANALYSIS_UP_TO_DATE)
    {
        goto cleanup;
    }
    else if (merge_analysis & GIT_MERGE_ANALYSIS_UNBORN)
    {
        err = 5000;
        gj_log("GitPull merge_analysis unborn\n");
        goto cleanup;
    }
    else if (merge_analysis & GIT_MERGE_ANALYSIS_FASTFORWARD)
    {
        gj_log("GitPull: Performing FastForward\n");

        err = perform_fastforward(repo, git_annotated_commit_id(origin_annotated_commit));
        if (err < 0)
            goto cleanup;
    }
    else if (merge_analysis & GIT_MERGE_ANALYSIS_NORMAL)
    {
        gj_log("GitPull: Performing Normal Merge\n");

        git_merge_options merge_options = GIT_MERGE_OPTIONS_INIT;
        git_checkout_options checkout_options = GIT_CHECKOUT_OPTIONS_INIT;

        err = git_merge(repo, (const git_annotated_commit **)annotated_commits, 1,
                        &merge_options, &checkout_options);
        if (err < 0)
            goto cleanup;

        err = git_repository_index(&index, repo);
        if (err < 0)
            goto cleanup;

        err = git_index_conflict_iterator_new(&conflict_iter, index);
        if (err < 0)
            goto cleanup;

        // Handle Conflicts
        while (1)
        {
            git_index_entry *ancestor_out;
            git_index_entry *our_out;
            git_index_entry *their_out;
            err = git_index_conflict_next((const git_index_entry **)&ancestor_out,
                                          (const git_index_entry **)&our_out,
                                          (const git_index_entry **)&their_out,
                                          conflict_iter);

            if (err == GIT_ITEROVER)
            {
                gj_log_internal("    Done with Conflicts\n");
                break;
            }
            if (err < 0)
                goto cleanup;

            gj_log_internal("GitPull: Conflict on file %s\n", their_out->path);

            // 1. We resolve the conflict by choosing their changes
            gj_resolve_conflict(git_base_path, ancestor_out, our_out, their_out);
            gj_log_internal("Resolved conflict on file %s\n", their_out->path);

            // 2. We remove it from the list of conflicts
            err = git_index_conflict_remove(index, our_out->path);
            if (err < 0)
                goto cleanup;

            // 3. We add it to the index
            git_strarray paths = {(char **)&our_out->path, 1};
            err = git_index_add_all(index, &paths, 0, NULL, NULL);
            if (err < 0)
                goto cleanup;

            err = git_index_write(index);
            if (err < 0)
                goto cleanup;
        }

        err = git_index_write_tree(&tree_id, index);
        if (err < 0)
            goto cleanup;

        err = git_tree_lookup(&tree, repo, &tree_id);
        if (err < 0)
            goto cleanup;

        //
        // Make the commit
        //
        err = git_signature_now(&sig, author_name, author_email);
        if (err < 0)
            goto cleanup;

        // Get the parents
        git_oid head_id;
        err = git_reference_name_to_id(&head_id, repo, "HEAD");
        if (err < 0)
            goto cleanup;

        err = git_commit_lookup(&head_commit, repo, &head_id);
        if (err < 0)
            goto cleanup;

        err = git_commit_lookup(&origin_head_commit, repo,
                                git_annotated_commit_id(origin_annotated_commit));
        if (err < 0)
            goto cleanup;

        const git_commit *parents[] = {head_commit, origin_head_commit};
        char *message = "Custom Merge commit";
        git_oid commit_id;

        err = git_commit_create(&commit_id, repo, "HEAD", sig, sig, NULL, message, tree, 2, parents);
        if (err < 0)
            goto cleanup;
    }

cleanup:
    git_tree_free(tree);
    git_commit_free(head_commit);
    git_commit_free(origin_head_commit);
    git_signature_free(sig);
    if (repo != 0)
        git_repository_state_cleanup(repo);
    git_index_conflict_iterator_free(conflict_iter);
    git_index_free(index);
    git_reference_free(head_ref);
    git_reference_free(origin_head_ref);
    git_annotated_commit_free(origin_annotated_commit);
    git_repository_free(repo);
    free(name);

    return err;
}

int gj_init()
{
    return git_libgit2_init();
}

int gj_shutdown()
{
    return git_libgit2_shutdown();
}
