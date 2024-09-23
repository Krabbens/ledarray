import 'package:flutter/material.dart';

import 'views/register_network.dart';
import 'views/permission_view.dart';

import 'views/pre_connection_view.dart';

class App extends StatelessWidget {
  const App({super.key});

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
        //'/animation-list': (context) => ListOfAnimations(controller: MQTTController('esp32/alive_status')),
        //'/settings': (context) => SettingsView(controller: MQTTController('esp32/alive_status')),
        //'/upload': (context) => SecondView(controller: MQTTController('esp32/alive_status')),

      },
      home: const PermissionView(),
      //home: ListOfAnimations(controller: MQTTController('esp32/alive_status')),
    );
  }
}


