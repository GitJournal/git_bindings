package io.gitjournal.git_bindings;

import android.os.AsyncTask;

public class GitFetchTask extends AsyncTask<String, Void, Void> {
    private final static String TAG = "GitFetch";
    private AnyThreadResult result;

    public GitFetchTask(AnyThreadResult _result) {
        result = _result;
    }

    protected Void doInBackground(String... params) {
        String cloneDirPath = params[0];
        final String publicKeyPath = params[1];
        final String privateKeyPath = params[2];
        final String remote = params[3];

        Git git = new Git();
        git.setSshKeys(publicKeyPath, privateKeyPath, "");
        String errorStr = git.fetch(cloneDirPath, remote);
        if (!errorStr.isEmpty()) {
            result.error("FAILED", errorStr, null);
            return null;
        }

        result.success(null);
        return null;
    }
}
