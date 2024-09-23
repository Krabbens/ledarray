import 'dart:async';

import 'package:mqtt_client/mqtt_client.dart';
import 'package:mqtt_client/mqtt_server_client.dart';

import 'package:conn_app/enums/connectivity_status.dart';

import 'dart:convert';
import 'dart:typed_data';

import 'package:typed_data/src/typed_buffer.dart';

const int FRAME_TYPE_ANIMATION_ADD = 5;
const int FRAME_TYPE_ANIMATION_REMOVE = 6;
const int FRAME_TYPE_ANIMATION_GET = 7;
const int FRAME_TYPE_ANIMATION_PLAY = 8;
const int FRAME_TYPE_ANIMATION_CLEAR = 9;
const int FRAME_TYPE_ANIMATION_NAMES = 10;

class MQTTController {
  final String server = '5686adbdc3644dca8e63a851e72c3b21.s1.eu.hivemq.cloud';
  final int port = 8883;
  final String clientIdentifier = 'android_status';
  final String _topic;

  final String username = 'esp';
  final String password = '_ledarray_nugget';

  late MqttServerClient client;

  final StreamController<String> _messagesController = StreamController<String>.broadcast();
  Stream<String> get messages => _messagesController.stream;

  final StreamController<ConnectivityStatus> _connectionStatusController = StreamController<ConnectivityStatus>.broadcast();
  Stream<ConnectivityStatus> get connectionStatus => _connectionStatusController.stream;

  final StreamController<ConnectivityStatus> _espControllerStatus = StreamController<ConnectivityStatus>.broadcast();
  Stream<ConnectivityStatus> get espStatus => _espControllerStatus.stream;

  ConnectivityStatus _isConnected = ConnectivityStatus.disconnected;

  int _lastRecvMessage = -1;

  late Timer? _timer;

  MQTTController(this._topic) {
    client = MqttServerClient(server, clientIdentifier);
    client.port = port;
    client.keepAlivePeriod = 20;
    client.secure = true;
    client.onDisconnected = onDisconnected;
    client.onConnected = onConnected;
    client.onSubscribed = onSubscribed;
    client.onUnsubscribed = onUnsubscribed;
    client.logging(on: true);

    startListening();

    _timer = Timer.periodic(const Duration(seconds: 15), (timer) {
      if (_isConnected == ConnectivityStatus.disconnected) {
        connect();
      }
    });
  }

  void connect() async {
    try {
      await client.connect(username, password);
    } catch (e) {
      print('Exception: $e');
      client.disconnect();
    }
  }

  void close() {
    client.disconnect();
  }

  void disconnect() {
    client.disconnect();
  }

  void subscribe() {
    client.subscribe(_topic, MqttQos.atMostOnce);
  }

  void publish(String message, String topic) {
    final builder = MqttClientPayloadBuilder();
    builder.addString(message);
    client.publishMessage(topic, MqttQos.atMostOnce, builder.payload!);
  }

  void onDisconnected() {
    if (!_connectionStatusController.isClosed) {
      _connectionStatusController.add(ConnectivityStatus.disconnected);
    }
    _isConnected = ConnectivityStatus.disconnected;
  }

  void onConnected() {

    subscribe();

    client.updates!.listen((List<MqttReceivedMessage<MqttMessage>> event) {
      final message = event[0].payload as MqttPublishMessage;
      onMessage(event[0].topic, message);
    });

    _isConnected = ConnectivityStatus.connected;
    _connectionStatusController.add(ConnectivityStatus.connected);

    Timer.periodic(const Duration(seconds: 10), (timer) {
      publish('1', 'esp32/check_alive');
      if (DateTime.now().millisecondsSinceEpoch - _lastRecvMessage > 20000 || _lastRecvMessage == -1) {
        _espControllerStatus.add(ConnectivityStatus.disconnected);
      }
    });
  }

  void onSubscribed(String topic) {
    print('Subscribed to $topic');
  }

  void onUnsubscribed(String? topic) {
    print('Unsubscribed from $topic');
  }

  void onMessage(String topic, MqttPublishMessage message) {
    final payload = MqttPublishPayload.bytesToStringAsString(message.payload.message);
    if (payload == "1") {
      _espControllerStatus.add(ConnectivityStatus.connected);
      _lastRecvMessage = DateTime.now().millisecondsSinceEpoch;
    }
    _messagesController.add(payload);
  }

  void dispose() {
    _timer!.cancel();
    _messagesController.close();
    _connectionStatusController.close();
    client.disconnect();
  }

  void startListening() {
    _messagesController.stream.listen((message) {
      print("New message: $message");
      handleMessage(utf8.encode(message));
    });
  }

  void handleMessage(Uint8List payload) {
    if (payload.length < 8) {
      print("Payload too short");
      return;
    }

    final ByteData byteData = ByteData.sublistView(payload);
    final int frameType = byteData.getUint32(0, Endian.little);
    final int contentLength = byteData.getUint32(4, Endian.little);

    if (payload.length != contentLength + 8) {
      print("Content length mismatch");
      return;
    }

    final data = payload.sublist(8);
    print('Received frame type $frameType with data of length ${data.length}');
  }

  void sendFrame(int frameType, String topic, [Uint8List? data]) {
    int contentLength = data == null ? 0 : data.length + 1;  // +1 for the null terminator
    final ByteData frame = ByteData(8 + contentLength);
    
    frame.setUint32(0, frameType, Endian.little);
    frame.setUint32(4, contentLength, Endian.little);
    
    if (data != null) {
      frame.buffer.asUint8List().setRange(8, 8 + data.length, data);
      frame.setUint8(8 + data.length, 0);  // null terminator
    }

    final builder = MqttClientPayloadBuilder();
    builder.addBuffer(frame.buffer as Uint8Buffer);
    client.publishMessage(topic, MqttQos.atMostOnce, builder.payload!);

    print('Sent frame type $frameType with data of length $contentLength');
  }

  void sendFile(Uint8List fileData, String animationName, String topic) async {
    // Prepare the animation name with a null terminator
    final List<int> nameWithNull = utf8.encode(animationName) + [0];

    // Prepare the final data to send: [animationName + fileData]
    final Uint8List dataToSend = Uint8List.fromList(nameWithNull + fileData);

    // Send the frame with the file data
    sendFrame(FRAME_TYPE_ANIMATION_ADD, topic, dataToSend);
  }
}