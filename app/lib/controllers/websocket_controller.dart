import 'dart:async';
import 'dart:typed_data';
import 'dart:io';
import 'dart:convert';
import 'package:web_socket_channel/web_socket_channel.dart';
import 'package:web_socket_channel/status.dart' as status;
import "package:conn_app/enums/connectivity_status.dart";
import "package:conn_app/enums/frame.dart";

class WebSocketController {
  final String serverUrl;

  late WebSocketChannel _channel;

  final StreamController<ConnectivityStatus> _connectionStatusController =
      StreamController<ConnectivityStatus>.broadcast();
  Stream<ConnectivityStatus> get connectionStatus =>
      _connectionStatusController.stream;

  final StreamController<List<String>> _fileNamesController =
      StreamController<List<String>>.broadcast();
  Stream<List<String>> get fileNamesStream => _fileNamesController.stream;
  List<String> _fileNames = [];

  final StreamController<SizeInfo> _sizeInfoController =
      StreamController<SizeInfo>.broadcast();
  Stream<SizeInfo> get sizeInfoStream => _sizeInfoController.stream;
  SizeInfo _sizeInfo = SizeInfo(totalBytes: 1, usedBytes: 1);

  final StreamController<ConnectivityStatus> _espControllerStatus =
      StreamController<ConnectivityStatus>.broadcast();
  Stream<ConnectivityStatus> get espStatus => _espControllerStatus.stream;

  ConnectivityStatus _isConnected = ConnectivityStatus.disconnected;

  bool _isWebSocketOpen = false;
  bool _isConnecting = false;
  Timer? _reconnectTimer;
  Timer? _periodicTimer;

  int attemptCounter = 0;
  int maxAttempts = 5;

  WebSocketController(this.serverUrl) {
    print('WebSocketController initialized with URL: $serverUrl');
    _connect();
  }

  void _connect() {
    if (_isConnecting) {
      print("Already attempting to connect. Skipping...");
      return;
    }

    // Zamknięcie poprzedniego połączenia, jeśli otwarte
    if (_isWebSocketOpen) {
      _channel.sink.close(status.normalClosure);
      _isWebSocketOpen = false;
      print("Previous WebSocket connection closed.");
    }

    _isConnecting = true;

    try {
      print("Attempting to connect... Attempt #$attemptCounter");
      _channel = WebSocketChannel.connect(Uri.parse(serverUrl));

      _channel.ready.then((_) {
        _isWebSocketOpen = true;
        _isConnecting = false;
        _connectionStatusController.add(ConnectivityStatus.connected);
        _espControllerStatus.add(ConnectivityStatus.connected);

        _startPeriodicUpdates();
        print("Connected to WebSocket.");
      }).catchError((error) {
        print("Error while connecting: $error");
        _handleDisconnection();
      });

      _channel.stream.listen(
        (event) {
          if (event is Uint8List) {
            onMessage(event);
          }
        },
        onError: (error) {
          print("WebSocket error: $error");
          _handleDisconnection();
        },
        onDone: () {
          print("WebSocket connection closed.");
          _handleDisconnection();
        },
        cancelOnError: true,
      );
    } catch (e) {
      print("Unexpected error during connection: $e");
      _handleDisconnection();
    }
  }

  void _handleDisconnection() async {
    _isWebSocketOpen = false;
    _isConnecting = false;
    _connectionStatusController.add(ConnectivityStatus.idle);
    attemptCounter++;

    if (attemptCounter >= maxAttempts) {
      print("Maximum reconnection attempts reached.");
      _espControllerStatus.add(ConnectivityStatus.disconnected);
    }

    print("Reconnecting in 2 seconds...");
    await Future.delayed(Duration(seconds: 2));
    _connect();
  }

  void _startPeriodicUpdates() {
    _stopPeriodicUpdates();
    sendFrame(FrameType.animationGet, null);
    sendFrame(FrameType.getSize, null);
    _periodicTimer = Timer.periodic(const Duration(seconds: 10), (timer) {
      sendFrame(FrameType.animationGet, null);
      sendFrame(FrameType.getSize, null);
    });
  }

  void _stopPeriodicUpdates() {
    _periodicTimer?.cancel();
  }

  void sendText(String message) {
    if (_isWebSocketOpen) {
      _channel.sink.add(message);
      print("Sent text: $message");
    } else {
      print("WebSocket is not connected.");
    }
  }

  void sendBinary(Uint8List data) {
    if (_isWebSocketOpen) {
      _channel.sink.add(data);
      print("Sent binary data: ${data.length} bytes");
    } else {
      print("WebSocket is not connected.");
    }
  }

  void sendFrame(FrameType frameType, Uint8List? payload) {
    final contentLength = payload?.length ?? 0;
    final frame = Frame(frameType);
    final message = Uint8List.fromList([...frame.toBytes(), ...?payload]);
    sendBinary(message);
    print("Sent frame type $frameType with length $contentLength");
  }

  void onMessage(Uint8List buffer) {
    if (buffer.length < 8) {
      print('Payload too short: ${buffer.length} bytes');
      return;
    }

    final Frame frame = Frame.fromBytes(buffer);

    buffer = buffer.sublist(1);
    final payload = String.fromCharCodes(buffer);

    if (frame.type == FrameType.animationNames) {
      List<String> result = payload.isEmpty ? [] : payload.split(',');
      _fileNames = result;
      _fileNamesController.add(_fileNames);
    } else if (frame.type == FrameType.infoSize) {
      SizeInfo result = SizeInfo.fromBytes(buffer);
      _sizeInfo = result;
      _sizeInfoController.add(_sizeInfo);
    }
  }

  void dispose() {
    if (_isWebSocketOpen) {
      _channel.sink.close(status.normalClosure);
    }
    _stopPeriodicUpdates();
    _connectionStatusController.close();
    _fileNamesController.close();
    _sizeInfoController.close();
    _espControllerStatus.close();
  }

  void sendAnimation(Uint8List fileData, String animationName) {
    Uint8List nameWithNull =
        Uint8List.fromList(utf8.encode(animationName) + [0]);
    Uint8List dataToSend =
        Uint8List(nameWithNull.length + fileData.length);
    dataToSend.setRange(0, nameWithNull.length, nameWithNull);
    dataToSend.setRange(nameWithNull.length,
        nameWithNull.length + fileData.length, fileData);

    sendFrame(FrameType.animationAdd, dataToSend);
  }

  void playAnimation(String name) {
    final Uint8List payload = Uint8List.fromList(name.codeUnits);
    sendFrame(FrameType.animationPlay, payload);
  }

  void removeAnimation(String name) {
    final Uint8List payload = Uint8List.fromList(name.codeUnits);
    sendFrame(FrameType.animationRemove, payload);
  }

  void stopAnimation() {
    sendFrame(FrameType.animationStop, null);
  }

  void update() {
    _sizeInfoController.add(_sizeInfo);
    _fileNamesController.add(_fileNames);
  }
}
