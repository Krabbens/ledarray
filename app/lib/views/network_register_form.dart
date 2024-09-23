import 'package:flutter/material.dart';
import 'package:conn_app/controllers/http_esp_controller.dart';
import 'package:conn_app/enums/connectivity_status.dart';
import 'package:conn_app/views/loading_box.dart';

class NetworkRegisterForm extends StatefulWidget {
  const NetworkRegisterForm({super.key});

  @override
  State<NetworkRegisterForm> createState() => _NetworkRegisterFormState();
}

class _NetworkRegisterFormState extends State<NetworkRegisterForm> {

  late HttpEspController _espController;

  String _ssid = '';
  String _password = '';
  ConnectivityStatus _espStatus = ConnectivityStatus.idle;

  @override
  void initState() {
    _espController = HttpEspController();

    _espController.espStatus.listen((event) {
      setState(() {
        _espStatus = event;
      });
    });

    super.initState();
  }

  @override
  void dispose() {
    _espController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return StreamBuilder(
      stream: _espController.espStatus,
      builder: (context, snapshot) {
        if (snapshot.hasData) {
          if (snapshot.data == ConnectivityStatus.unknown) {
            return const LoadingBox(text: 'Registering Network on ESP');
          } else if (snapshot.data == ConnectivityStatus.connected) {
            WidgetsBinding.instance.addPostFrameCallback((_) {
              Navigator.of(context)
                  .pushNamedAndRemoveUntil('/pre-connection', (route) => false);
            });
          }
        }

        return Padding(
          padding: const EdgeInsets.all(32.0),
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            crossAxisAlignment: CrossAxisAlignment.center,
            children: [
              const Text(
                'Register Network',
                style: TextStyle(
                  fontSize: 24,
                  fontWeight: FontWeight.bold,
                ),
              ),
              const SizedBox(height: 16),
              TextField(
                decoration: const InputDecoration(
                  labelText: 'SSID',
                ),
                onChanged: (value) {
                  setState(() {
                    _espStatus = ConnectivityStatus.idle;
                    _ssid = value;
                  });
                },
              ),
              TextField(
                decoration: const InputDecoration(
                  labelText: 'Password',
                ),
                onChanged: (value) {
                  setState(() {
                    _espStatus = ConnectivityStatus.idle;
                    _password = value;
                  });
                },
              ),
              const SizedBox(height: 32),
              ElevatedButton(
                onPressed: () {
                  _espController.registerNetwork(_ssid, _password);
                },
                child: const Text('Submit'),
              ),
              if (_espStatus == ConnectivityStatus.disconnected) ...[
                const Text('Not connected'),
              ]
            ],
          ),
        );
      },
    );
  }
}
