package io.gitjournal.git_bindings;

import android.os.AsyncTask;

public class GitMergeTask extends AsyncTask<String, Void, Void> {
    private final static String TAG = "GitMerge";
    private AnyThreadResult result;

    public GitMergeTask(AnyThreadResult _result) {
        result = _result;
    }

    protected Void doInBackground(String... params) {
        String cloneDirPath = params[0];
        final String branch = params[1];
        final String authorName = params[2];
        final String authorEmail = params[3];

        Git git = new Git();
        String errorStr = git.merge(cloneDirPath, branch, authorName, authorEmail);
        if (!errorStr.isEmpty()) {
            result.error("FAILED", errorStr, null);
            return null;
        }

        result.success(null);
        return null;
    }
}
