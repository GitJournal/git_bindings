package io.gitjournal.git_bindings;

import android.os.AsyncTask;
import android.util.Log;

import org.apache.commons.io.FileUtils;

import java.io.File;
import java.io.IOException;
import java.nio.charset.Charset;

public class GenerateSSHKeysTask extends AsyncTask<String, Void, Void> {
    private final static String TAG = "GenerateSSHKeys";
    private AnyThreadResult result;

    public GenerateSSHKeysTask(AnyThreadResult _result) {
        result = _result;
    }

    protected Void doInBackground(String... params) {
        String publicKeyPath = params[0];
        String privateKeyPath = params[1];
        String comment = params[2];

        File privateKeyFile = new File(privateKeyPath);
        if (privateKeyFile.exists()) {
            Log.d(TAG, "Private key already exists. Overwriting");
        }

        Git git = new Git();
        String errorStr = git.generateKeys(privateKeyPath, publicKeyPath, comment);
        if (!errorStr.isEmpty()) {
            result.error("FAILED", errorStr, null);
            return null;
        }

        result.success(null);
        return null;
    }
}
