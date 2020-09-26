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

  static Future<void> clone(String folderPath, String cloneUrl) async {
    try {
      await invokePlatformMethod('gitClone', {
        'cloneUrl': cloneUrl,
        'folderPath': folderPath,
      });
    } on PlatformException catch (e) {
      throw createGitException(e.message);
    }
  }

  static Future<void> init(String folderPath) async {
    try {
      await invokePlatformMethod('gitInit', {
        'folderPath': folderPath,
      });
    } on PlatformException catch (e) {
      throw createGitException(e.message);
    }
  }

  Future<void> fetch(String remote) async {
    try {
      await invokePlatformMethod('gitFetch', {
        'folderPath': folderPath,
        'remote': remote,
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

  Future<void> push(String remote) async {
    try {
      await invokePlatformMethod('gitPush', {
        'folderPath': folderPath,
        'remote': remote,
      });
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

Future<String> generateSSHKeys({@required String comment}) async {
  String publicKey = await invokePlatformMethod('generateSSHKeys', {
    'comment': comment,
  });
  return publicKey;
}

Future<void> setSshKeys({
  @required String publicKey,
  @required String privateKey,
}) async {
  await invokePlatformMethod('setSshKeys', {
    'publicKey': publicKey,
    'privateKey': privateKey,
  });
}

Future<String> getSSHPublicKey() async {
  return await invokePlatformMethod('getSSHPublicKey');
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
