import 'dart:async';
import 'dart:convert';
import 'dart:developer';
import 'dart:io';
import 'dart:isolate';
import 'dart:typed_data';

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:native_opencv/native_opencv.dart';
import 'package:path_provider/path_provider.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      theme: ThemeData(
        colorScheme: ColorScheme.fromSeed(seedColor: Colors.deepPurple),
        useMaterial3: true,
      ),
      home: MyHomePage(),
    );
  }
}

class MyHomePage extends StatefulWidget {
  @override
  State<MyHomePage> createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {
  bool _isProcessing = false;
  String? _processedImagePath;
  String? _originalImagePath;
  Map<String, dynamic>? _jsonResult;

  Future<void> _processImageFromAssets() async {
    if (_isProcessing) return;
    setState(() => _isProcessing = true);

    try {
      // Load image from assets
      final ByteData data = await rootBundle.load('assets/1733735192763.jpg');
      final List<int> bytes = data.buffer.asUint8List();

      // Create temporary file for original image
      final Directory tempDir = await getTemporaryDirectory();
      final String originalFilePath = '${tempDir.path}/original_input.jpg';
      final File originalFile = File(originalFilePath);
      await originalFile.writeAsBytes(bytes);

      // Store original image path
      setState(() {
        _originalImagePath = originalFilePath;
      });

      // Process the image
      _processImage(originalFile);
    } catch (e) {
      log('Error processing image: $e');
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(content: Text('Error processing image: $e')),
      );
    } finally {
      setState(() {
        _isProcessing = false;
      });
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Phiếu Chấm Trắc Nghiệm'),
        backgroundColor: Colors.deepPurple,
        foregroundColor: Colors.white,
      ),
      body: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          children: <Widget>[
            // Button to process image
            SizedBox(
              width: double.infinity,
              child: ElevatedButton(
                onPressed: _isProcessing ? null : _processImageFromAssets,
                style: ElevatedButton.styleFrom(
                  backgroundColor: Colors.deepPurple,
                  foregroundColor: Colors.white,
                  padding: const EdgeInsets.symmetric(vertical: 16),
                ),
                child: _isProcessing
                    ? const Row(
                        mainAxisSize: MainAxisSize.min,
                        children: [
                          SizedBox(
                            width: 20,
                            height: 20,
                            child: CircularProgressIndicator(
                              strokeWidth: 2,
                              valueColor:
                                  AlwaysStoppedAnimation<Color>(Colors.white),
                            ),
                          ),
                          SizedBox(width: 10),
                          Text('Đang xử lý...'),
                        ],
                      )
                    : const Text('Xử lý phiếu trắc nghiệm'),
              ),
            ),

            // Clear results button
            if (_originalImagePath != null ||
                _processedImagePath != null ||
                _jsonResult != null)
              Padding(
                padding: const EdgeInsets.only(top: 8.0),
                child: SizedBox(
                  width: double.infinity,
                  child: OutlinedButton(
                    onPressed: () {
                      setState(() {
                        _originalImagePath = null;
                        _processedImagePath = null;
                        _jsonResult = null;
                      });
                    },
                    style: OutlinedButton.styleFrom(
                      foregroundColor: Colors.red,
                      side: const BorderSide(color: Colors.red),
                      padding: const EdgeInsets.symmetric(vertical: 12),
                    ),
                    child: const Text('Xóa kết quả'),
                  ),
                ),
              ),

            const SizedBox(height: 20),

            // Results section
            if (_originalImagePath != null ||
                _processedImagePath != null ||
                _jsonResult != null)
              Expanded(
                child: SingleChildScrollView(
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      // Images section
                      if (_originalImagePath != null ||
                          _processedImagePath != null)
                        Column(
                          crossAxisAlignment: CrossAxisAlignment.start,
                          children: [
                            const Text(
                              'Kết quả xử lý:',
                              style: TextStyle(
                                fontSize: 18,
                                fontWeight: FontWeight.bold,
                              ),
                            ),
                            const SizedBox(height: 10),

                            // Original and processed images
                            Row(
                              children: [
                                // Original image
                                if (_originalImagePath != null)
                                  Expanded(
                                    child: Column(
                                      crossAxisAlignment:
                                          CrossAxisAlignment.start,
                                      children: [
                                        const Text(
                                          'Phiếu gốc:',
                                          style: TextStyle(
                                            fontWeight: FontWeight.bold,
                                            color: Colors.blue,
                                          ),
                                        ),
                                        const SizedBox(height: 5),
                                        ClipRRect(
                                          borderRadius:
                                              BorderRadius.circular(8),
                                          child: Image.file(
                                            File(_originalImagePath!),
                                            fit: BoxFit.cover,
                                          ),
                                        ),
                                      ],
                                    ),
                                  ),

                                if (_originalImagePath != null &&
                                    _processedImagePath != null)
                                  const SizedBox(width: 10),

                                // Processed image
                                if (_processedImagePath != null)
                                  Expanded(
                                    child: Column(
                                      crossAxisAlignment:
                                          CrossAxisAlignment.start,
                                      children: [
                                        const Text(
                                          'Phiếu đã xử lý:',
                                          style: TextStyle(
                                            fontWeight: FontWeight.bold,
                                            color: Colors.green,
                                          ),
                                        ),
                                        const SizedBox(height: 5),
                                        ClipRRect(
                                          borderRadius:
                                              BorderRadius.circular(8),
                                          child: Image.file(
                                            File(_processedImagePath!),
                                            fit: BoxFit.cover,
                                          ),
                                        ),
                                      ],
                                    ),
                                  ),
                              ],
                            ),
                          ],
                        ),

                      // JSON result section
                      if (_jsonResult != null) ...[
                        const SizedBox(height: 20),
                        const Text(
                          'Kết quả đáp án (JSON):',
                          style: TextStyle(
                            fontSize: 16,
                            fontWeight: FontWeight.bold,
                            color: Colors.deepPurple,
                          ),
                        ),
                        const SizedBox(height: 10),
                        Container(
                          width: double.infinity,
                          padding: const EdgeInsets.all(12),
                          decoration: BoxDecoration(
                            color: Colors.grey[100],
                            borderRadius: BorderRadius.circular(8),
                            border: Border.all(color: Colors.grey[300]!),
                          ),
                          child: Text(
                            _formatJsonResult(_jsonResult!),
                            style: const TextStyle(
                              fontFamily: 'monospace',
                              fontSize: 13,
                            ),
                          ),
                        ),
                      ],
                    ],
                  ),
                ),
              ),
          ],
        ),
      ),
    );
  }

  String _formatJsonResult(Map<String, dynamic> json) {
    // Format JSON for better readability
    const JsonEncoder encoder = JsonEncoder.withIndent('  ');
    String formatted = encoder.convert(json);

    // If the JSON contains answers, format them nicely
    if (json.containsKey('answers') && json['answers'] is List) {
      StringBuffer buffer = StringBuffer();
      buffer.writeln('Kết quả chấm bài:');
      buffer.writeln('================');

      List answers = json['answers'];
      for (int i = 0; i < answers.length; i++) {
        buffer.writeln('Câu ${i + 1}: ${answers[i]}');
      }

      if (json.containsKey('score')) {
        buffer.writeln('================');
        buffer.writeln('Điểm: ${json['score']}');
      }

      buffer.writeln('\nJSON chi tiết:');
      buffer.writeln(formatted);

      return buffer.toString();
    }

    return formatted;
  }

  void _processImage(File file) async {
    final Directory tempDir = await getTemporaryDirectory();
    final String tempPath = '${tempDir.path}/temp.jpg';

    final port = ReceivePort();
    final args = ProcessImageArguments(
      file.path,
      tempPath,
      jsonArgs: jsonEncode({}),
    );

    try {
      await Isolate.spawn(
        (List args) {
          final sendPort = args[0] as SendPort;
          final processArgs = args[1] as ProcessImageArguments;
          processImage(sendPort, processArgs);
        },
        [port.sendPort, args],
        onError: port.sendPort,
        onExit: port.sendPort,
      );
    } catch (e) {
      log('Isolate spawn failed: $e');
      return;
    }

    late StreamSubscription sub;

    sub = port.listen((message) async {
      await sub.cancel();
      port.close();

      if (message == null) return;

      final result = jsonDecode(message);

      handleResult(result, tempPath);
    });
  }

  Future<void> handleResult(result, tempPath) async {
    result = result is String ? jsonDecode(result) : result;
    setState(() {
      _processedImagePath = tempPath;
      _jsonResult =
          result is Map<String, dynamic> ? result : {'result': result};
    });
    print('Result: $result\nProcessed Image Path: $tempPath');
  }
}
