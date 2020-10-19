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
        final String publicKey = params[1];
        final String privateKey = params[2];
        final String password = params[3];
        final String remote = params[4];

        Git git = new Git();
        String errorStr = git.fetch(cloneDirPath, remote, publicKey, privateKey, password);
        if (!errorStr.isEmpty()) {
            result.error("FAILED", errorStr, null);
            return null;
        }

        result.success(null);
        return null;
    }
}
