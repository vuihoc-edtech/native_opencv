import 'dart:async';
import 'dart:ffi' as ffi;
import 'dart:io';
import 'dart:isolate';

import 'package:ffi/ffi.dart';
import 'package:flutter/services.dart';

class NativeOpencv {
  static const MethodChannel _channel = MethodChannel('native_opencv');

  static Future<String> get platformVersion async {
    final String version = await _channel.invokeMethod('getPlatformVersion');
    return version;
  }
}

// C function signatures
typedef _CVersionFunc = ffi.Pointer<Utf8> Function();
typedef _CProcessImageFunc = ffi.Pointer<Utf8> Function(
  ffi.Pointer<Utf8>,
  ffi.Pointer<Utf8>,
  ffi.Pointer<Utf8>?,
);

// Dart function signatures
typedef _VersionFunc = ffi.Pointer<Utf8> Function();
typedef _ProcessImageFunc = ffi.Pointer<Utf8> Function(
  ffi.Pointer<Utf8>,
  ffi.Pointer<Utf8>,
  ffi.Pointer<Utf8>?,
);

// Getting a library that holds needed symbols
ffi.DynamicLibrary _openDynamicLibrary() {
  if (Platform.isAndroid) {
    return ffi.DynamicLibrary.open('libnative_opencv.so');
  } else if (Platform.isWindows) {
    return ffi.DynamicLibrary.open('native_opencv_windows_plugin.dll');
  }

  return ffi.DynamicLibrary.process();
}

ffi.DynamicLibrary _lib = _openDynamicLibrary();

// Looking for the functions
final _VersionFunc _version =
    _lib.lookup<ffi.NativeFunction<_CVersionFunc>>('version').asFunction();
final _ProcessImageFunc _processImage = _lib
    .lookup<ffi.NativeFunction<_CProcessImageFunc>>('process_image')
    .asFunction();

String opencvVersion() {
  return _version().toDartString();
}

void processImage(SendPort sendPort, ProcessImageArguments args) {
  // Call the native function and get the result
  final res = _processImage(
    args.inputPath.toNativeUtf8(),
    args.outputPath.toNativeUtf8(),
    args.jsonArgs?.toNativeUtf8(),
  ).toDartString();

  // Send the result back to the main isolate
  sendPort.send(res);
}

class ProcessImageArguments {
  final String inputPath;
  final String outputPath;
  final String? jsonArgs;

  ProcessImageArguments(
    this.inputPath,
    this.outputPath, {
    this.jsonArgs,
  });
}
