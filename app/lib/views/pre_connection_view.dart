import 'package:conn_app/views/list_of_animations.dart';
import 'package:flutter/material.dart';
import 'package:conn_app/views/loading_box.dart';
import 'package:conn_app/controllers/websocket_controller.dart';
import 'package:conn_app/controllers/internet_controller.dart';
import 'package:conn_app/enums/connectivity_status.dart';
import 'package:conn_app/enums/internet_status.dart';
import 'package:provider/provider.dart';

class PreConnectionView extends StatefulWidget {
  const PreConnectionView({super.key});

  @override
  State<PreConnectionView> createState() => _PreConnectionViewState();
}

class _PreConnectionViewState extends State<PreConnectionView> {
  late WebSocketController _controller;
  late InternetController _internetController;

  ConnectivityStatus _mqttStatus = ConnectivityStatus.unknown;
  ConnectivityStatus _espStatus = ConnectivityStatus.unknown;
  InternetStatus _internetStatus = InternetStatus.unknown;

  @override
  void initState() {
    super.initState();
    _internetController = InternetController();

    _internetController.connectionStatus.listen((event) {
      setState(() {
        _internetStatus = event;
      });
    });
  }

  @override
  void dispose() {
    _controller.dispose();
    _internetController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    _controller = context.watch<WebSocketController?>() ?? WebSocketController('ws://upper_esp.local/ws');
    _controller.connectionStatus.listen((event) {
      setState(() { 
        _mqttStatus = event;
      });
    });
    _controller.espStatus.listen((event) {
      setState(() {
        _espStatus = event;
      });
    });

    return Scaffold(
      appBar: AppBar(
        title: const Text(
          'Led Array',
          style: TextStyle(color: Colors.white),
        ),
        backgroundColor: Colors.deepPurpleAccent,
      ),
      body: Builder(
        builder: (context) {
          if (_internetStatus == InternetStatus.unknown) {
            return const LoadingBox(text: 'Checking internet connection');
          }

          if (_internetStatus == InternetStatus.noInternet) {
            return const Center(
              child: Text('No internet connection'),
            );
          }

          if (_internetStatus == InternetStatus.esp) {
            return SizedBox(
                width: MediaQuery.of(context).size.width,
                child: Column(
                  mainAxisAlignment: MainAxisAlignment.center,
                  crossAxisAlignment: CrossAxisAlignment.center,
                  children: [
                    const SizedBox(height: 64),
                    const Text('Connected to ESP32 WiFi'),
                    const Text('Do you want to configure the board?'),
                    const SizedBox(height: 16),
                    ElevatedButton(
                      onPressed: () {
                        WidgetsBinding.instance.addPostFrameCallback((_) {
                          Navigator.of(context).pushNamedAndRemoveUntil(
                              '/scan-wifi', (route) => false);
                        });
                      },
                      child: const Text('Configure'),
                    ),
                  ],
                ));
          }

          if (_mqttStatus == ConnectivityStatus.unknown) {
            return const LoadingBox(text: 'Launching WebSocket');
          }

          if (_espStatus == ConnectivityStatus.unknown) {
            return const LoadingBox(text: 'Checking ESP32 status');
          }

          if (_espStatus == ConnectivityStatus.disconnected) {
            return SizedBox(
                width: MediaQuery.of(context).size.width,
                child: Column(
                  mainAxisAlignment: MainAxisAlignment.center,
                  crossAxisAlignment: CrossAxisAlignment.center,
                  children: [
                    const SizedBox(height: 64),
                    const CircularProgressIndicator(),
                    const SizedBox(height: 16),
                    const Text('Checking ESP32 status'),
                    const SizedBox(height: 48),
                    const Text('Do you want to configure the board?'),
                    const SizedBox(height: 16),
                    ElevatedButton(
                      onPressed: () {
                        WidgetsBinding.instance.addPostFrameCallback((_) {
                          Navigator.of(context).pushNamedAndRemoveUntil(
                              '/permission', (route) => false);
                        });
                      },
                      child: const Text('Configure'),
                    ),
                  ],
                ));
          }

          if (_espStatus == ConnectivityStatus.connected) {
            return Center(
              child: Column(
                mainAxisAlignment: MainAxisAlignment.center,
                crossAxisAlignment: CrossAxisAlignment.center,
                children: [
                  const Text('Connected to ESP32'),
                  const SizedBox(height: 20),
                  ElevatedButton(
                    onPressed: () {
                      Navigator.push(
                        context,
                        MaterialPageRoute(builder: (context) => ListOfAnimations(controller: _controller)),
                      );
                    },
                    child: const Text('Continue'),
                  ),
                ],
              ),
            );
          }

          return const Center(
            child: Text('Unknown status'),
          );
        },
      ),
    );
  }
}