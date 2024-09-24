import 'dart:async';

import 'package:flutter/material.dart';

import 'package:permission_handler/permission_handler.dart';

class PermissionView extends StatefulWidget {
  const PermissionView({super.key});

  @override
  State<PermissionView> createState() => _PermissionViewState();
}

class _PermissionViewState extends State<PermissionView> {
  bool _isGranted = false;
  Map<Permission, PermissionStatus> status = {};

  Timer? _timer;

  @override
  void initState() {
    super.initState();
    _requestPermissions();
    _timer = Timer.periodic(const Duration(seconds: 1), (timer) {
      if (status[Permission.location] == PermissionStatus.granted) {
        setState(() {
          _isGranted = true;
        });
        timer.cancel();
      }
    });
  }

  @override
  void dispose() {
    _timer?.cancel();
    super.dispose();
  }

  void _requestPermissions() async {
    status = await [
      Permission.location,
      Permission.locationAlways,
      Permission.locationWhenInUse,
    ].request();
  }

  @override
  Widget build(BuildContext context) {
    return Builder(
      builder: (context) {
        if (_isGranted) {
          WidgetsBinding.instance.addPostFrameCallback((_) {
            if (status[Permission.location] == PermissionStatus.granted) {
              Navigator.pushNamedAndRemoveUntil(
                  context, '/pre-connection', (route) => false);
            }
          });
        }

        return Scaffold(
          appBar: AppBar(
            title: const Text('Request Permissions'),
          ),
          body: const Center(
            child: Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: <Widget>[
                CircularProgressIndicator(),
                SizedBox(height: 16),
                Text('Requesting Permissions'),
              ],
            ),
          ),
        );
      },
    );
  }
}
