import 'dart:async';
import 'package:http/http.dart';

import 'package:conn_app/enums/connectivity_status.dart';

class HttpEspController {
  final _espUrl = Uri.parse('http://192.168.4.1');

  void dispose() {
    _espStatusController.close();
  }

  final StreamController<ConnectivityStatus> _espStatusController =
      StreamController<ConnectivityStatus>.broadcast();
  Stream<ConnectivityStatus> get espStatus => _espStatusController.stream;

  Future<void> registerNetwork(String ssid, String password) async {
    _espStatusController.add(ConnectivityStatus.unknown);
    // url -> http://192.168.4.1/register?ssid=ssid&password=password
    final url = _espUrl.replace(
        path: '/register',
        queryParameters: {'ssid': ssid, 'password': password});
    print(url);
    final response = await get(url);
    print(response.statusCode);
    if (response.statusCode == 200) {
      _espStatusController.add(ConnectivityStatus.connected);
    } else {
      _espStatusController.add(ConnectivityStatus.disconnected);
    }
  }
}
