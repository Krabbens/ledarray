import 'package:flutter/material.dart';
import 'package:conn_app/controllers/internet_controller.dart';

class ScanWifi extends StatefulWidget {
  const ScanWifi({super.key});

  @override
  State<ScanWifi> createState() => _ScanWifiState();
}

class _ScanWifiState extends State<ScanWifi> {
  late InternetController _controller;

  @override
  void initState() {
    _controller = InternetController();
    super.initState();
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Led Array'),
        backgroundColor: Colors.deepPurpleAccent,
      ),
      body: const Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: <Widget>[
            Text('Scan Wifi'),
          ],
        ),
      ),
    );
  }
}
