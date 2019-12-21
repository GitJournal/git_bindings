import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

import 'package:path/path.dart' as p;
import 'package:path_provider/path_provider.dart';

import 'package:git_bindings/git_bindings.dart';

void main() {
  runApp(GitApp());
}

const gitFolderName = "journal";

class GitApp extends StatefulWidget {
  @override
  _GitAppState createState() => _GitAppState();
}

class _GitAppState extends State<GitApp> {
  final GlobalKey<ScaffoldState> _scaffoldKey = GlobalKey<ScaffoldState>();
  GitRepo gitRepo;

  var cloneUrl = "git@github.com:GitJournal/journal_test.git";
  TextEditingController cloneUrlController;

  String publicKey = "";

  @override
  void initState() {
    super.initState();

    cloneUrlController = TextEditingController(text: cloneUrl);
    getSSHPublicKey().then((val) {
      setState(() {
        publicKey = val;
      });
    });

    getApplicationSupportDirectory().then((dir) async {
      var repoPath = p.join(dir.path, gitFolderName);
      gitRepo = GitRepo(
        folderPath: repoPath,
        authorName: "Vishesh Handa",
        authorEmail: "noemail@example.com",
      );
      print("GitRepo Initialized");
    });
  }

  @override
  Widget build(BuildContext context) {
    var cloneUrlWidget = TextField(
      decoration: InputDecoration(
        border: OutlineInputBorder(),
        hintText: "Clone URL",
      ),
      onChanged: (newValue) {
        setState(() {
          cloneUrl = newValue;
        });
      },
      controller: cloneUrlController,
    );

    var publicKeyWidget = Container(
      child: Text(publicKey),
      margin: const EdgeInsets.all(15.0),
      padding: const EdgeInsets.all(3.0),
      decoration: BoxDecoration(border: Border.all(color: Colors.blueAccent)),
    );

    var columns = Column(
      children: [
        cloneUrlWidget,
        ..._buildGitButtons(),
        publicKeyWidget,
      ],
      crossAxisAlignment: CrossAxisAlignment.center,
      mainAxisAlignment: MainAxisAlignment.center,
    );

    return MaterialApp(
      title: 'Git App',
      home: Scaffold(
        key: _scaffoldKey,
        appBar: AppBar(
          title: const Text('Git Test'),
        ),
        body: Padding(
          padding: EdgeInsets.all(8.0),
          child: SingleChildScrollView(child: columns),
        ),
      ),
    );
  }

  void _sendSuccess() {
    var text = "Success";
    _scaffoldKey.currentState
      ..removeCurrentSnackBar()
      ..showSnackBar(SnackBar(content: Text(text)));
  }

  void _sendError(String text) {
    _scaffoldKey.currentState
      ..removeCurrentSnackBar()
      ..showSnackBar(SnackBar(content: Text("ERROR: " + text)));
  }

  List<Widget> _buildGitButtons() {
    return <Widget>[
      RaisedButton(
        child: const Text("Generate Keys"),
        onPressed: () async {
          var key = await generateSSHKeys(comment: "Git Sample App");
          setState(() {
            publicKey = key;
          });
        },
      ),
      RaisedButton(
        child: const Text("Copy Key to Clipboard"),
        onPressed: () async {
          await Clipboard.setData(ClipboardData(text: publicKey));
          _sendSuccess();
        },
      ),
      RaisedButton(
        child: const Text("Git Clone"),
        onPressed: () async {
          try {
            await GitRepo.clone(gitRepo.folderPath, cloneUrl);
            _sendSuccess();
          } on GitException catch (ex) {
            print(ex);
            _sendError(ex.toString());
          }
        },
      ),
      RaisedButton(
        child: const Text("Git Pull"),
        onPressed: () async {
          gitRepo.pull();
        },
      ),
      RaisedButton(
        child: const Text("Git Add"),
        onPressed: () async {
          gitRepo.add(".");
        },
      ),
      RaisedButton(
        child: const Text("Git Push"),
        onPressed: () async {
          gitRepo.push();
        },
      ),
      RaisedButton(
        child: const Text("Git Commit"),
        onPressed: () async {
          gitRepo.commit(
            message: "Default message from GitJournal",
            when: "2017-10-20T01:21:10+02:00",
          );
        },
      ),
      RaisedButton(
        child: const Text("Git Reset Last"),
        onPressed: () async {
          gitRepo.resetLast();
        },
      ),
    ];
  }
}
