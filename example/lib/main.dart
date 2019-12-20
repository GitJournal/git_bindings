import 'package:flutter/material.dart';
import 'package:git_bindings/git.dart';

import 'gitapp.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();

  await setSshKeys(
    privateKey: """
""",
    publicKey:
        "ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABAQDI+U1Q8aja8Ir/DiM33TLGG+sC9KZoCiGFibLNPc0unD3t5b1N3YlJAT4PcxxvRArXRoV3PVDXUgAXVs/sX0Uwy/CnBPfbze76bzA19GoQu1aRsVbwXTXWHOSWtB7XgCEGuP6kvriUvkZAG/wiscyJi28MjdANsF+Ob4b5MxhpiBxK3WqObLPk2BbuQWMbxDzf8ZaWJKvIYaeRJgTe+50oe610nrzD7UWLZzvCo7EZItAS7hhUPSdRinH1/yvTBcc2/J1wTb5vWOUGVVe9GBS7ZFfK5VQs6rqhTcOZCyf/SvLV6r8DcWQE+57C7WQutMusnZvHKFoMsIgPCR+KWGCf gitjournal git testing",
  );

  runApp(GitApp());
}
