import 'package:flutter/material.dart';

import 'views/register_network.dart';
import 'views/permission_view.dart';

import 'views/pre_connection_view.dart';

class App extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Wifi Scanner',
      theme: ThemeData(
        primarySwatch: Colors.blue,
      ),
      routes: {
        '/pre-connection': (context) => const PreConnectionView(),
        '/scan-wifi': (context) => const RegisterNetwork(),
        '/permission': (context) => const PermissionView(),
      },
      home: const PermissionView(),
    );
  }
}


