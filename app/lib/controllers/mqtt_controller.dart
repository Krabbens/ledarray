import 'dart:async';

import 'package:mqtt_client/mqtt_client.dart';
import 'package:mqtt_client/mqtt_server_client.dart';

import 'package:conn_app/enums/connectivity_status.dart';

import 'dart:convert';
import 'dart:typed_data';

import 'package:typed_data/src/typed_buffer.dart';

import 'package:conn_app/enums/frame.dart';

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

  final StreamController<List<String>> _fileNamesController = StreamController<List<String>>.broadcast();
  Stream<List<String>> get fileNamesStream => _fileNamesController.stream;
  List<String> _fileNames = [];

  final StreamController<SizeInfo> _sizeInfoController = StreamController<SizeInfo>.broadcast();
  Stream<SizeInfo> get sizeInfoStream => _sizeInfoController.stream;
  SizeInfo _sizeInfo = SizeInfo(totalBytes: 1, usedBytes: 1);

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
      if (DateTime.now().millisecondsSinceEpoch - _lastRecvMessage > 20000 || _lastRecvMessage == -1) {
        _espControllerStatus.add(ConnectivityStatus.disconnected);
      }
      else{
        sendFrame(FrameType.animationGet, "upper_esp");
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
    // final Uint8Buffer buffer = message.payload.message;
    // int size = buffer.length;
    // if(size < 8){
    //   print('Payload to short $size');
    //   return;
    // }
    // final Frame frame = Frame.fromBytes(buffer);
    // for (int i = 0; i < 8; i++) {
    //   buffer.removeAt(0);
    // }
    // final payload = MqttPublishPayload.bytesToStringAsString(message.payload.message);
    // if (frame.type == FrameType.aliveStatus) {
    //   _espControllerStatus.add(ConnectivityStatus.connected);
    //   _lastRecvMessage = DateTime.now().millisecondsSinceEpoch;
    // }
    // else if (frame.type == FrameType.animationNames){
    //   final resultString = MqttPublishPayload.bytesToStringAsString(buffer);
    //   List<String> result;
    //   if(resultString.length <= 1){
    //     result = List<String>.empty();
    //   }
    //   else{
    //     result = resultString.split(',');
    //   }
    //   _fileNames = result;
    //   _fileNamesController.add(_fileNames);
    // }
    // else if (frame.type == FrameType.infoSize){
    //   SizeInfo result = SizeInfo.fromBytes(buffer);
    //   _sizeInfo = result;
    //   _sizeInfoController.add(_sizeInfo);
    //   print(result);
    // }
    // _messagesController.add(payload);
  }

  void dispose() {
    _timer!.cancel();
    _messagesController.close();
    _connectionStatusController.close();
    _fileNamesController.close();
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

  void sendFrame(FrameType frameType, String topic, [Uint8List? data]) {
    // int contentLength = data == null ? 0 : data.length;
    // final Frame frame = Frame(frameType, contentLength);
    // final builder = MqttClientPayloadBuilder();
    // builder.addBuffer(frame.toBytes());
    // Uint8Buffer buffer = Uint8Buffer();
    // if(data != null){
    //   buffer.addAll(data);
    //   builder.addBuffer(buffer);
    // }
    

    // //print first 40 bytes of payload
    // //if (buffer.length > 40) {
    //   //print(buffer.sublist(0, 40));
    // //} else {
    //   //print(buffer);
    // //}

    // client.publishMessage(topic, MqttQos.atMostOnce, builder.payload!);

    // print('Sent frame type $frameType with data of length $contentLength');
  }

  void sendAnimation(Uint8List fileData, String animationName, String topic) async {
    
    Uint8List nameWithNull = Uint8List.fromList(utf8.encode(animationName) + [0]);

    //print("Name with null length: ${nameWithNull.length}");
    //print name with null
    //print(nameWithNull);
    
    Uint8List dataToSend = Uint8List(nameWithNull.length + fileData.length);
    dataToSend.setRange(0, nameWithNull.length, nameWithNull);
    dataToSend.setRange(nameWithNull.length, nameWithNull.length + fileData.length, fileData);

    //print('Sending animation $animationName with ${dataToSend.length} bytes');
    //print first 10 bytes
    //print(dataToSend.sublist(0, 10));

    sendFrame(FrameType.animationAdd, topic, dataToSend);
  }

  void sendString(FrameType frameType, String topic, String message){
    Uint8List uint8list = Uint8List.fromList(utf8.encode(message) + [0]);

    sendFrame(frameType, topic, uint8list);
  }
}