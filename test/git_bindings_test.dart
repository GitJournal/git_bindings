import 'package:flutter/services.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:git_bindings/git_bindings.dart';

void main() {
  const MethodChannel channel = MethodChannel('git_bindings');

  TestWidgetsFlutterBinding.ensureInitialized();

  setUp(() {
    channel.setMockMethodCallHandler((MethodCall methodCall) async {
      return '42';
    });
  });

  tearDown(() {
    channel.setMockMethodCallHandler(null);
  });

  test('getPlatformVersion', () async {
    expect(await GitBindings.platformVersion, '42');
  });
}
