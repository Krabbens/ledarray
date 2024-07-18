import 'package:flutter/material.dart';

import 'views/scan_wifi.dart';
import 'views/permission_view.dart';

import 'routers/pre_connection_router.dart';

class App extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Wifi Scanner',
      theme: ThemeData(
        primarySwatch: Colors.blue,
      ),
      routes: {
        '/pre-connection': (context) => const PreConnectionRouter(),
        '/scan-wifi': (context) => const ScanWifi(),
        '/permission': (context) => const PermissionView(),
      },
      home: const PreConnectionRouter(),
    );
  }
}


