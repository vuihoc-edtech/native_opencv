#
# To learn more about a Podspec see http://guides.cocoapods.org/syntax/podspec.html.
# Run `pod lib lint native_opencv.podspec' to validate before publishing.
#
Pod::Spec.new do |s|
  s.name             = 'native_opencv'
  s.version          = '4.5.5'
  s.summary          = 'OpenCV dynamic framework'
  s.description      = <<-DESC
A new flutter plugin project trip bitcode for OpenCV dynamic framework.
https://github.com/eddy-lau/OpenCV-Dynamic-Framework/blob/4.5.5/OpenCV-Dynamic-Framework.podspec
                       DESC
  s.homepage         = 'https://github.com/vuihoc-edtech/OpenCV-Dynamic-Framework.git'
  s.license          = { :file => '../LICENSE' }
  s.author           = { 'VH EDTech' => 'hautv.fami@gmail.com' }
  s.source           = { :path => '.' }
  s.source_files = 'Classes/**/*'
  s.public_header_files = 'Classes/**/*.h'
  s.dependency 'Flutter'
  s.platform = :ios, '12.0'

  s.prepare_command = <<-CMD
  ./extract.sh
  CMD

  s.preserve_paths = "opencv2.framework"
  s.vendored_frameworks = "opencv2.framework"
  s.requires_arc = false
  
  s.ios.frameworks = [
    "AssetsLibrary",
    "AVFoundation",
    "CoreGraphics",
    "CoreMedia",
    "CoreVideo",
    "Foundation",
    "QuartzCore",
    "UIKit"
  ]

  s.libraries = "c++"
  s.xcconfig = {
      "OTHER_LDFLAGS" => "-framework opencv2"
  }
  s.pod_target_xcconfig = {
      "CLANG_CXX_LANGUAGE_STANDARD" => "c++17",
      "CLANG_CXX_LIBRARY" => "libc++",
      "EXCLUDED_ARCHS[sdk=iphonesimulator*]" => "arm64",
      "DEFINES_MODULE" => "YES",
      "HEADER_SEARCH_PATHS" => "$(inherited) $(PODS_TARGET_SRCROOT)/opencv2.framework/Headers",
      "OTHER_LDFLAGS" => "-framework opencv2",
      "FRAMEWORK_SEARCH_PATHS" => "$(inherited) $(PODS_TARGET_SRCROOT)"
  }
  s.user_target_xcconfig = { 
      "EXCLUDED_ARCHS[sdk=iphonesimulator*]" => "arm64",
      "OTHER_LDFLAGS" => "-framework opencv2",
      "FRAMEWORK_SEARCH_PATHS" => "$(inherited) $(PODS_TARGET_SRCROOT)"
  }
  
  # s.dependency 'OpenCV-Dynamic-Framework'
  # s.pod_target_xcconfig = {
  #   'DEFINES_MODULE' => 'YES',
  #   'EXCLUDED_ARCHS[sdk=iphonesimulator*]' => 'i386 arm64',
  #   'HEADER_SEARCH_PATHS' => '$(inherited) $(Pods/OpenCV-Dynamic-Framework/opencv2.framework/Headers)'  # Thêm dòng này
  # }
  # Flutter.framework does not contain a i386 slice.
#   s.pod_target_xcconfig = { 'DEFINES_MODULE' => 'YES', 'EXCLUDED_ARCHS[sdk=iphonesimulator*]' => 'i386 arm64' }
#
  s.swift_version = '5.0'

  # telling CocoaPods not to remove framework
  # s.preserve_paths = 'opencv2.framework' 

  # telling linker to include opencv2 framework
#   s.xcconfig = { 'OTHER_LDFLAGS' => '-framework opencv2' }
  # s.xcconfig = {
  #   'OTHER_LDFLAGS' => '-framework opencv2 -lc++'  # Thêm -lc++
  # }

  # including OpenCV framework
  # s.vendored_frameworks = 'opencv2.framework' 

  # including native framework
  # s.frameworks = 'AVFoundation'

  # including C++ library
  # s.library = 'c++'
end
