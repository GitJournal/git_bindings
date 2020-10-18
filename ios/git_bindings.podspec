#
# To learn more about a Podspec see http://guides.cocoapods.org/syntax/podspec.html.
# Run `pod lib lint git_bindings.podspec' to validate before publishing.
#
Pod::Spec.new do |s|
  s.name             = 'git_bindings'
  s.version          = '0.0.1'
  s.summary          = 'A new flutter plugin project.'
  s.description      = <<-DESC
A new flutter plugin project.
                       DESC
  s.homepage         = 'https://gitjournal.io'
  s.license          = { :file => '../LICENSE' }
  s.author           = { 'Vishesh Handa' => 'vhanda@gitjournal.io' }
  s.source           = { :path => '.' }
  s.source_files = 'Classes/**/*', '../gj_common/gitjournal.c', '../gj_common/common.c'
  s.public_header_files = 'Classes/**/*.h', '../gj_common/gitjournal.h'
  s.dependency 'Flutter'
  s.platform = :ios, '8.0'

  # Flutter.framework does not contain a i386 slice. Only x86_64 simulators are supported.
  s.pod_target_xcconfig = { 'DEFINES_MODULE' => 'YES', 'VALID_ARCHS[sdk=iphonesimulator*]' => 'x86_64' }

  s.vendored_frameworks = 'Frameworks/libssl.framework', 'Frameworks/libcrypto.framework', 'Frameworks/libssh2.framework', 'Frameworks/libgit2.framework'
end
