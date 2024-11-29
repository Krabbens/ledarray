import 'package:flutter/material.dart';
import 'package:conn_app/controllers/internet_controller.dart';

import 'package:conn_app/enums/internet_status.dart';
import 'package:conn_app/views/loading_box.dart';
import 'package:conn_app/views/network_register_form.dart';

class RegisterNetwork extends StatefulWidget {
  const RegisterNetwork({super.key});

  @override
  State<RegisterNetwork> createState() => _RegisterNetworkState();
}

class _RegisterNetworkState extends State<RegisterNetwork> {
  late InternetController _controller;

  InternetStatus _internetStatus = InternetStatus.unknown;
  

  @override
  void initState() {
    _controller = InternetController();

    _controller.connectionStatus.listen((event) {
      setState(() {
        _internetStatus = event;
      });
    });

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
        elevation: 40,
      ),
      body: StreamBuilder(
        stream: _controller.connectionStatus,
        builder: (context, snapshot) {
          if (snapshot.connectionState == ConnectionState.waiting) {
            return const LoadingBox(
              text: 'Retrieving WiFi name',
            );
          }

          if (snapshot.hasError || _internetStatus == InternetStatus.unknown) {
            return const LoadingBox(
              text: 'Error occurred',
            );
          }

          if (_internetStatus == InternetStatus.internet ||
              _internetStatus == InternetStatus.noInternet) {
            WidgetsBinding.instance.addPostFrameCallback((_) {
              Navigator.of(context)
                  .pushNamedAndRemoveUntil('/pre-connection', (route) => false);
            });
          }

          return const NetworkRegisterForm();
        },
      ),
    );
  }
}
