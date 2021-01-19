package io.gitjournal.git_bindings;

public class Git {
    static {
        // FIXME: Change the name of the library
        System.loadLibrary("native-lib");
    }

    // This needs to be called once!
    public native void setupLib();

    public native String generateKeys(String privateKeyPath, String publicKeyPath, String comment);

    public native String merge(String basePath, String branch, String authorName, String authorEmail);
    public native String fetch(String basePath, String remote, String publicKey, String privateKey, String password);
    public native String push(String basePath, String remote, String publicKey, String privateKey, String password);
    public native String defaultBranch(String basePath, String remote, String publicKey, String privateKey, String password);
    public native String clone(String cloneUrl, String basePath, String publicKey, String privateKey, String password);

    public native String commit(String basePath, String authorName, String authorEmail, String message);
    public native String resetHard(String basePath, String ref);
    public native String add(String basePath, String pattern);
    public native String rm(String basePath, String pattern);
}
