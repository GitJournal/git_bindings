package io.gitjournal.git_bindings;

import android.os.AsyncTask;

import java.io.File;

public class GitCloneTask extends AsyncTask<String, Void, Void> {
    private final static String TAG = "GitClone";
    private AnyThreadResult result;

    public GitCloneTask(AnyThreadResult _result) {
        result = _result;
    }

    protected Void doInBackground(String... params) {
        final String url = params[0];
        final String cloneDirPath = params[1];
        final String publicKey = params[2];
        final String privateKey = params[3];
        final String password = params[4];

        Git git = new Git();
        String errorStr = git.clone(url, cloneDirPath, publicKey, privateKey, password);
        if (!errorStr.isEmpty()) {
            result.error("FAILED", errorStr, null);
            return null;
        }

        result.success(null);
        return null;
    }
}
