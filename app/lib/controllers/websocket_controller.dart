import 'dart:async';
import 'dart:typed_data';
import 'dart:convert';
import 'package:web_socket_channel/web_socket_channel.dart';
import 'package:web_socket_channel/status.dart' as status;
import "package:conn_app/enums/connectivity_status.dart";

class WebSocketController {
  final String serverUrl;

  late WebSocketChannel _channel;
  final StreamController<Uint8List> _binaryMessagesController = StreamController<Uint8List>.broadcast();
  Stream<Uint8List> get binaryMessages => _binaryMessagesController.stream;

  final StreamController<String> _textMessagesController = StreamController<String>.broadcast();
  Stream<String> get textMessages => _textMessagesController.stream;

  final StreamController<ConnectivityStatus> _connectionStatusController = StreamController<ConnectivityStatus>.broadcast();
  Stream<ConnectivityStatus> get connectionStatus => _connectionStatusController.stream;
  

  final StreamController<List<String>> _fileNamesController = StreamController<List<String>>.broadcast();
  Stream<List<String>> get fileNamesStream => _fileNamesController.stream;

  final StreamController<SizeInfo> _sizeInfoController = StreamController<SizeInfo>.broadcast();
  Stream<SizeInfo> get sizeInfoStream => _sizeInfoController.stream;

  final StreamController<ConnectivityStatus> _espControllerStatus = StreamController<ConnectivityStatus>.broadcast();
  Stream<ConnectivityStatus> get espStatus => _espControllerStatus.stream;

  ConnectivityStatus _isConnected = ConnectivityStatus.disconnected;

  late Timer? _timer;

  bool _isWebSocketOpen = false;
  Timer? _reconnectTimer;

  WebSocketController(this.serverUrl) {
    print('WebSocketController initialized with URL: $serverUrl');
    _connect();
  }

  void _connect() async {
    try {
      print("Attempting to connect...");
      _channel = WebSocketChannel.connect(Uri.parse(serverUrl));

      // Nasłuchiwanie strumienia WebSocket
      _channel.stream.listen(
        (event) {
          if (!_isWebSocketOpen) {
            _isWebSocketOpen = true;
            _connectionStatusController.add(ConnectivityStatus.connected);
            print("Connected to WebSocket.");
          }
          // Obsługa wiadomości
          if (event is String) {
            _textMessagesController.add(event);
          } else if (event is Uint8List) {
            _binaryMessagesController.add(event);
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
    } on WebSocketChannelException catch (e) {
      print("SocketException: Unable to connect to server ($e)");
      _handleDisconnection();
    } catch (e) {
      print("Unexpected error during connection: $e");
      _handleDisconnection();
    }
  }

  void _handleDisconnection() {
    if (_isWebSocketOpen) {
      _isWebSocketOpen = false;
      _connectionStatusController.add(ConnectivityStatus.idle);
    }

    // Rozpocznij próby ponownego połączenia
    _startReconnectLoop();
  }

  void _startReconnectLoop() {
  int attemptCounter = 0;
  int maxAttempts = 5; // Ustaw maksymalną liczbę prób

  _reconnectTimer?.cancel(); // Usuń istniejący timer, jeśli jest
  _reconnectTimer = Timer.periodic(const Duration(seconds: 1), (timer) {
    attemptCounter++;
    if (attemptCounter > maxAttempts) {
      print("Failed to reconnect after $maxAttempts attempts.");
      timer.cancel();
      // Wyślij komunikat o niepowodzeniu
      _connectionStatusController.add(ConnectivityStatus.disconnected);
      return;
    }

    print("Attempting to reconnect... Attempt $attemptCounter");
    _connect();

    if (_isWebSocketOpen) {
      print("Reconnected successfully after $attemptCounter attempts.");
      timer.cancel();
    }
  });
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

  void sendFrame(int frameType, Uint8List? payload) {
    final contentLength = payload?.length ?? 0;
    final frameHeader = ByteData(8)
      ..setUint32(0, frameType, Endian.little)
      ..setUint32(4, contentLength, Endian.little);
    final frame = Uint8List.fromList([...frameHeader.buffer.asUint8List(), ...?payload]);
    sendBinary(frame);
    print("Sent frame type $frameType with length $contentLength");
  }

  // Additional methods to match the MQTTController structure

  void connect() {
    if (!_isWebSocketOpen) {
      _connect();
    }
  }

  void disconnect() {
    if (_isWebSocketOpen) {
      _channel.sink.close(status.normalClosure);
      _connectionStatusController.add(ConnectivityStatus.disconnected);
      print("Disconnected from WebSocket.");
    }
  }

  void subscribe() {
    // Placeholder for subscribe logic
  }

  void publish(String message, String topic) {
    // Placeholder for publish logic
  }

  void onDisconnected() {
    // Placeholder for onDisconnected logic
  }

  void onConnected() {
    // Placeholder for onConnected logic
  }

  void onSubscribed(String topic) {
    // Placeholder for onSubscribed logic
  }

  void onUnsubscribed(String? topic) {
    // Placeholder for onUnsubscribed logic
  }

  void onMessage(String topic, String message) {
    // Placeholder for onMessage logic
  }

  void sendFrameWithTopic(int frameType, String topic, Uint8List? payload) {
    // Placeholder for sending frame with topic logic
  }

  void handleMessage(Uint8List payload) {
    // Placeholder for handling incoming messages
    print("Handling message: $payload");
  }

  void dispose() {
    _channel.sink.close(status.normalClosure);
    _binaryMessagesController.close();
    _textMessagesController.close();
    _connectionStatusController.close();
    _fileNamesController.close();
    _sizeInfoController.close();
    _timer?.cancel();
  }

  // Placeholder for more functionality (like sending animation)
  void sendAnimation(Uint8List fileData, String animationName, String topic) {
    // Placeholder for sendAnimation logic
    print("Sending animation for $animationName to $topic.");
  }

  void sendString(String message) {
    // Placeholder for sending string message
    print("Sending string: $message");
  }
}

class SizeInfo {
  final int totalBytes;
  final int usedBytes;

  SizeInfo({required this.totalBytes, required this.usedBytes});

  // Placeholder constructor, you can modify it based on your needs
  factory SizeInfo.fromBytes(Uint8List bytes) {
    return SizeInfo(totalBytes: 1, usedBytes: 1); // Placeholder
  }
}