package io.gitjournal.git_bindings;

import android.util.Log;
import android.os.AsyncTask;

public class GitDefaultBranchTask extends AsyncTask<String, Void, Void> {
    private final static String TAG = "GitDefaultBranch";
    private AnyThreadResult result;

    public GitDefaultBranchTask(AnyThreadResult _result) {
        result = _result;
    }

    protected Void doInBackground(String... params) {
        String cloneDirPath = params[0];
        final String publicKey = params[1];
        final String privateKey = params[2];
        final String password = params[3];
        final String remote = params[4];

        Git git = new Git();
        String str = git.defaultBranch(cloneDirPath, remote, publicKey, privateKey, password);
        Log.i("GitJournalAndroid", "GitDefaultBranchTask: " + str);
        if (str.startsWith("gj:")) {
            String branch = "";
            if (str.length() > 3) {
                branch = str.substring(3);
            }
            result.success(branch);
            return null;
        }
        if (!str.isEmpty()) {
            result.error("FAILED", str, null);
            return null;
        }

        // FIXME: This should never occur
        result.success(null);
        return null;
    }
}
