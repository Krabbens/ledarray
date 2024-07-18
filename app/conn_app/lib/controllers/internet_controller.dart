import 'dart:ffi';

import 'package:connectivity_plus/connectivity_plus.dart';
import 'package:network_info_plus/network_info_plus.dart';
import 'package:conn_app/enums/internet_status.dart';
import 'dart:async';
import 'dart:developer' as developer;
import 'package:flutter/services.dart';
import 'dart:io';


class InternetController {
  final _connectivity = Connectivity();
  late StreamSubscription<List<ConnectivityResult>> _subscription;

  final _info = NetworkInfo();

  final StreamController<InternetStatus> _connectionStatusController = StreamController<InternetStatus>();
  Stream<InternetStatus> get connectionStatus => _connectionStatusController.stream;

  InternetController() {
    _init();
    _subscription = _connectivity.onConnectivityChanged.listen(_update);
  }

  void dispose() {
    _subscription.cancel();
    _connectionStatusController.close();
  }

  Future<void> _init() async {
    late List<ConnectivityResult> result;
    try {
      result = await _connectivity.checkConnectivity();
    } on PlatformException catch (e) {
      developer.log('Couldn\'t check connectivity status', error: e);
      return;
    }

    return _update(result);
  }

  Future _update(List<ConnectivityResult> result) async {
    result = await _connectivity.checkConnectivity();

    InternetStatus status = InternetStatus.noInternet;

    if (result.contains(ConnectivityResult.wifi)) {

      final wifiName = await _info.getWifiName();
      if (wifiName != null && wifiName.contains('ledarray')) {
        status = InternetStatus.esp;
      }

      try {
        final ping = await InternetAddress.lookup('example.com');
        if (ping.isNotEmpty && ping[0].rawAddress.isNotEmpty) {
          status = InternetStatus.internet;
        }
      } on SocketException catch (_) {}
    }

    _connectionStatusController.add(status);

    print('Results: $result');
  }
}
