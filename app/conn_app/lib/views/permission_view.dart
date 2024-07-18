import 'package:flutter/material.dart';

import 'package:permission_handler/permission_handler.dart';

class PermissionView extends StatefulWidget {
  const PermissionView({super.key});

  @override
  State<PermissionView> createState() => _PermissionViewState();
}

class _PermissionViewState extends State<PermissionView> {

  @override
  void initState() {
    super.initState();
    _requestPermissions();
  }

  void _requestPermissions() async {
    final statuses = await [
      Permission.location,
      Permission.locationAlways,
      Permission.locationWhenInUse,
    ].request();
  }

  @override
  Widget build(BuildContext context) {
    return FutureBuilder(
      future: Permission.location.status,
      builder: (context, snapshot) {
        if (snapshot.connectionState == ConnectionState.done) {
          final status = snapshot.data as PermissionStatus;

          WidgetsBinding.instance.addPostFrameCallback((_){
            if (status.isGranted) {
              Navigator.pushNamedAndRemoveUntil(context, '/pre-connection', (route) => false);
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