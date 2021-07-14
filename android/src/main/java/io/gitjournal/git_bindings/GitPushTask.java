package io.gitjournal.git_bindings;

import android.os.AsyncTask;

public class GitPushTask extends AsyncTask<String, Void, Void> {
    private final static String TAG = "GitPush";
    private AnyThreadResult result;

    public GitPushTask(AnyThreadResult _result) {
        result = _result;
    }

    protected Void doInBackground(String... params) {
        String cloneDirPath = params[0];
        final String publicKey = params[1];
        final String privateKey = params[2];
        final String password = params[3];
        final String remote = params[4];
        final String statusFile = params[5];

        Git git = new Git();
        String errorStr = git.push(cloneDirPath, remote, publicKey, privateKey, password, statusFile);
        if (!errorStr.isEmpty()) {
            result.error("FAILED", errorStr, null);
            return null;
        }

        result.success(null);
        return null;
    }
}
