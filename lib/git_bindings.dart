import 'dart:async';
import 'dart:io';

import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

// Hack because I don't understand how to link libraries in ios
var channelName =
    Platform.isIOS ? "gitjournal.io/git" : 'io.gitjournal.git_bindings';
var _platform = MethodChannel(channelName);

Future invokePlatformMethod(String method, [dynamic arguments]) async {
  return _platform.invokeMethod(method, arguments);
}

class GitRepo {
  final String folderPath;

  const GitRepo({
    @required this.folderPath,
  });

  Future<void> fetch({
    @required String remote,
    @required String publicKey,
    @required String privateKey,
    @required String password,
  }) async {
    try {
      await invokePlatformMethod('gitFetch', {
        'folderPath': folderPath,
        'remote': remote,
        'publicKey': publicKey,
        'privateKey': privateKey,
        'password': password,
      });
    } on PlatformException catch (e) {
      throw createGitException(e.message);
    }
  }

  Future<void> merge({
    String branch,
    String authorName,
    String authorEmail,
  }) async {
    try {
      await invokePlatformMethod('gitMerge', {
        'folderPath': folderPath,
        'branch': branch,
        'authorName': authorName,
        'authorEmail': authorEmail,
      });
    } on PlatformException catch (e) {
      throw createGitException(e.message);
    }
  }

  Future<void> add(String filePattern) async {
    try {
      await invokePlatformMethod('gitAdd', {
        'folderPath': folderPath,
        'filePattern': filePattern,
      });
    } on PlatformException catch (e) {
      throw createGitException(e.message);
    }
  }

  Future<void> rm(String filePattern) async {
    try {
      await invokePlatformMethod('gitRm', {
        'folderPath': folderPath,
        'filePattern': filePattern,
      });
    } on PlatformException catch (e) {
      throw createGitException(e.message);
    }
  }

  Future<void> push({
    @required String remote,
    @required String publicKey,
    @required String privateKey,
    @required String password,
  }) async {
    try {
      await invokePlatformMethod('gitPush', {
        'folderPath': folderPath,
        'remote': remote,
        'publicKey': publicKey,
        'privateKey': privateKey,
        'password': password,
      });
    } on PlatformException catch (e) {
      throw createGitException(e.message);
    }
  }

  Future<String> defaultBranch({
    @required String remote,
    @required String publicKey,
    @required String privateKey,
    @required String password,
  }) async {
    try {
      String br = await invokePlatformMethod('gitDefaultBranch', {
        'folderPath': folderPath,
        'remote': remote,
        'publicKey': publicKey,
        'privateKey': privateKey,
        'password': password,
      });
      if (br != null && br.startsWith('refs/heads/')) {
        br = br.substring(11);
      }
      return br;
    } on PlatformException catch (e) {
      throw createGitException(e.message);
    }
  }

  // FIXME: Change this method to just resetHard
  Future<void> resetLast() async {
    try {
      await invokePlatformMethod('gitResetLast', {
        'folderPath': folderPath,
      });
    } on PlatformException catch (e) {
      throw createGitException(e.message);
    }
  }

  // FIXME: Change the datetime
  // FIXME: Actually implement the 'when'
  Future<void> commit({
    @required String message,
    @required String authorName,
    @required String authorEmail,
    String when,
  }) async {
    try {
      await invokePlatformMethod('gitCommit', {
        'folderPath': folderPath,
        'authorName': authorName,
        'authorEmail': authorEmail,
        'message': message,
        'when': when,
      });
    } on PlatformException catch (e) {
      throw createGitException(e.message);
    }
  }
}

class GitException implements Exception {
  final String cause;
  GitException(this.cause);

  @override
  String toString() {
    return "GitException: " + cause;
  }
}

GitException createGitException(String msg) {
  if (msg.contains("ENETUNREACH")) {
    return GitException("No Connection");
  }
  if (msg.contains("Remote origin did not advertise Ref for branch master")) {
    return GitException("No master branch");
  }
  if (msg.contains("Nothing to push")) {
    return GitException("Nothing to push.");
  }
  return GitException(msg);
}

Future<String> generateSSHKeys(
    {@required String privateKeyPath,
    @required String publicKeyPath,
    @required String comment}) async {
  String publicKey = await invokePlatformMethod('generateSSHKeys', {
    'privateKeyPath': privateKeyPath,
    'publicKeyPath': publicKeyPath,
    'comment': comment,
  });
  return publicKey;
}
