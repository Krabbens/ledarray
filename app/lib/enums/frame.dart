import 'dart:typed_data';
import 'package:typed_data/src/typed_buffer.dart';

enum FrameType {
  animationAdd,
  animationRemove,
  animationGet,
  animationPlay,
  animationClear,
  animationNames,
  animationStop,
  getSize,
  infoSize,
}

class Frame {
  FrameType type;
  int contentLength;

  Frame(this.type, this.contentLength);
  
  Uint8List toBytes() {
    final byteData = ByteData(8);
    byteData.setInt32(0, type.index, Endian.little);
    byteData.setInt32(4, contentLength, Endian.little);
    return byteData.buffer.asUint8List();
  }

  static Frame fromBytes(Uint8List bytes) {
    final buffer = ByteData.sublistView(bytes);
    FrameType type = FrameType.values[buffer.getInt32(0, Endian.little)];
    int contentLength = buffer.getInt32(4, Endian.little);
    return Frame(type, contentLength);
  }
}

class SizeInfo {
  int totalBytes;
  int usedBytes;

  SizeInfo({required this.totalBytes, required this.usedBytes});

  Uint8List toBytes() {
    final byteData = ByteData(8); // 2 x int32 (4 bytes each)
    byteData.setInt32(0, totalBytes, Endian.little);
    byteData.setInt32(4, usedBytes, Endian.little);
    return byteData.buffer.asUint8List();
  }

  static SizeInfo fromBytes(Uint8List bytes) {
    final buffer = ByteData.sublistView(bytes);
    int totalBytes = buffer.getInt32(0, Endian.little);
    int usedBytes = buffer.getInt32(4, Endian.little);
    return SizeInfo(totalBytes: totalBytes, usedBytes: usedBytes);
  }
}
