import 'dart:async';

import 'package:mqtt_client/mqtt_client.dart';
import 'package:mqtt_client/mqtt_server_client.dart';

import 'package:conn_app/enums/connectivity_status.dart';

class MQTTController {
  final String server = '5686adbdc3644dca8e63a851e72c3b21.s1.eu.hivemq.cloud';
  final int port = 8883;
  final String clientIdentifier = 'android_status';
  final String _topic;

  final String username = 'esp';
  final String password = '_ledarray_nugget';

  late MqttServerClient client;

  final StreamController<String> _messagesController = StreamController<String>();
  Stream<String> get messages => _messagesController.stream;

  final StreamController<ConnectivityStatus> _connectionStatusController = StreamController<ConnectivityStatus>();
  Stream<ConnectivityStatus> get connectionStatus => _connectionStatusController.stream;

  final StreamController<ConnectivityStatus> _espControllerStatus = StreamController<ConnectivityStatus>();
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
}