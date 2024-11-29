import 'package:conn_app/enums/frame.dart';
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

import 'views/register_network.dart';
import 'views/permission_view.dart';
import 'views/pre_connection_view.dart';
import 'controllers/websocket_controller.dart';

class App extends StatelessWidget {
  const App({super.key});

  @override
  Widget build(BuildContext context) {
    return MultiProvider(
      providers: [
        Provider<WebSocketController?>(
          create: (_) {
            final controller = WebSocketController('ws://upper_esp.local/ws');
            return controller;
          },
        ),
        Provider<SizeInfo>(
          create: (_){
            final sizeInfo = SizeInfo(totalBytes: 1, usedBytes: 1);
            return sizeInfo;
          },
        ),
      ],
      child: MaterialApp(
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
      ),
    );
  }
}

