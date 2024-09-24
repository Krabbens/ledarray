
import 'package:conn_app/enums/connectivity_status.dart';
import 'package:conn_app/enums/frame.dart';
import 'package:flutter/material.dart';
import 'package:file_picker/file_picker.dart';  // File picker package needed to upload files
import 'dart:typed_data';
import 'package:conn_app/controllers/mqtt_controller.dart';


class ListOfAnimations extends StatefulWidget {
  final MQTTController controller;
  const ListOfAnimations({super.key, required this.controller});
  //ListOfAnimations({super.key});

  @override
  _ListOfAnimationsState createState() => _ListOfAnimationsState(controller: controller);
  //_ListOfAnimationsState(controller: controller);
}

class _ListOfAnimationsState extends State<ListOfAnimations> {
  final MQTTController controller;
  _ListOfAnimationsState({required this.controller});

  List<String> items = []; // List of items

  int takenMemory = 1;
  int totalMemory = 1;

  @override
  void initState() {
    super.initState();
    _runOnLoad();
  }

  void _runOnLoad() {
    print('List loaded or returned to');

    setState(() {
      //Get loaded animations from esp
      controller.sendFrame(FrameType.animationGet, "upper_esp");
    });

    controller.espStatus.listen((event) {
      if (event != ConnectivityStatus.connected) {
        print('ESP is disconnected');

        Navigator.pushNamedAndRemoveUntil(
            context, '/pre-connection', (route) => false);
      }
    });

    controller.fileNamesStream.listen((fileNames) {
      setState(() {
        items = fileNames;
      });
    });

    controller.sizeInfoStream.listen((sizeInfo) {
      setState(() {
        takenMemory = sizeInfo.usedBytes;
        totalMemory = sizeInfo.totalBytes;
      });
    });
  }

  void _deleteItem(String item) {
    //setState(() {
    //items.removeAt(index); // Remove item from the list
    //});
    print('Deleting $item');

    controller.sendString(FrameType.animationRemove, "upper_esp", item);
  }

  void _playItem(String item) {
    // Placeholder function for the "Play" button action
    print('Playing $item');

    controller.sendString(FrameType.animationPlay, "upper_esp", item);
  }

  @override
  Widget build(BuildContext context) {
    // Calculate the progress based on takenMemory and totalMemory
    double memoryUsage = takenMemory / totalMemory;

    return Scaffold(
      appBar: AppBar(
        title: const Text('List of animations'),
        automaticallyImplyLeading: false,
      ),
      body: Column(
        children: [
          // Memory Usage Bar
          Padding(
            padding: const EdgeInsets.symmetric(horizontal: 16.0, vertical: 8.0),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Text('Memory Usage: $takenMemory B / $totalMemory B'),
                SizedBox(height: 10),
                LinearProgressIndicator(
                  value: memoryUsage,
                  minHeight: 20,
                  backgroundColor: Colors.grey[300],
                  valueColor: AlwaysStoppedAnimation<Color>(Colors.blue),
                ),
              ],
            ),
          ),
          Padding(
            padding: const EdgeInsets.symmetric(horizontal: 16.0, vertical: 8.0), // Add some padding to the top row
            child: Row(
              mainAxisAlignment: MainAxisAlignment.spaceBetween, // Spread buttons across the row
              children: [
                // Settings Button (top-left)
                IconButton(
                  icon: const Icon(Icons.settings),
                  onPressed: () {
                    Navigator.push(
                      context,
                      MaterialPageRoute(builder: (context) => SettingsView(controller: controller)), // Navigate to SettingsView
                    ).then((_) {
                      _runOnLoad(); // Call the function when returning
                    });
                  },
                ),
                // Go to Upload View Button (top-right)
                ElevatedButton(
                  onPressed: () {
                    Navigator.push(
                      context,
                      MaterialPageRoute(builder: (context) => SecondView(controller: controller)), // Navigate to SecondView
                    ).then((_) {
                      _runOnLoad(); // Call the function when returning
                    });
                  },
                  child: const Text('Upload a new animation'),
                ),
              ],
            ),
          ),
          // List of items with Play and Delete buttons
          Expanded(
            child: items.isNotEmpty
                ? ListView.separated(
                    itemCount: items.length,
                    separatorBuilder: (context, index) => const Divider(),
                    itemBuilder: (context, index) {
                      return Padding(
                        padding: const EdgeInsets.symmetric(vertical: 4.0),
                        child: Card(
                          elevation: 4,
                          shape: RoundedRectangleBorder(
                            borderRadius: BorderRadius.circular(10),
                          ),
                          child: ListTile(
                            title: Text(items[index]),
                            leading: IconButton(
                              icon: const Icon(Icons.play_arrow),
                              onPressed: () {
                                _playItem(items[index]);
                              },
                            ),
                            trailing: IconButton(
                              icon: const Icon(Icons.delete),
                              onPressed: () {
                                _deleteItem(items[index]);
                              },
                            ),
                          ),
                        ),
                      );
                    },
                  )
                : const Center(
                    child: Text(
                      'No animations available.',
                      style: TextStyle(fontSize: 18),
                    ),
                  ),
          ),
        ],
      ),
    );
  }
}


class SecondView extends StatefulWidget {
  final MQTTController controller;
  const SecondView({super.key, required this.controller});

  @override
  _SecondViewState createState() => _SecondViewState(controller: controller);
}

class _SecondViewState extends State<SecondView> {
  final MQTTController controller;
  _SecondViewState({required this.controller});

  String? _fileName; // To store the file name
  Uint8List? _fileBytes; // To store file bytes
  final TextEditingController _textController = TextEditingController();  // Controller to modify file name

  void _pickFile() async {
    FilePickerResult? result = await FilePicker.platform.pickFiles(
      withData: true,
      type: FileType.custom,
      allowedExtensions: ['dat', 'zip'],
    );

    if (result != null) {
      setState(() {
        _fileName = result.files.single.name;      // Set file name
        _fileBytes = result.files.single.bytes;    // Get the file bytes
        _textController.text = _fileName!;         // Set text field with file name
      });
    }
  }

  void _sendFile() {
    // Placeholder function for sending the file
    print('File name: ${_fileBytes!.length} bytes');

    controller.sendAnimation(_fileBytes!, _textController.text, "upper_esp");
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Uploading a file'),
      ),
      body: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Align(  // Align at the top
          alignment: Alignment.topCenter,
          child: Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            ElevatedButton(
              onPressed: _pickFile,
              child: const Text('Pick a file'),
            ),
            if (_fileName != null) ...[
              const SizedBox(height: 20),
              Text(_fileName ?? ''),
              TextField(
                  controller: _textController,
                  decoration: const InputDecoration(
                    border: OutlineInputBorder(),
                    labelText: 'Edit animation name',
                  ),
                ),
            ],
            const SizedBox(height: 20),
            ElevatedButton(
              onPressed: (_fileName != null && _fileBytes != null && _fileBytes!.isNotEmpty)
                  ? () {
                      _sendFile();
                      Navigator.pop(context);  // Go back to FirstView
                    }
                  : null,
              style: ElevatedButton.styleFrom(
                backgroundColor: _fileName != null ? null : Colors.grey,  // Grey color when inactive
              ),  // Disable button if no file is selected or file length is 0
              child: Text('Send the animation'),
            ),
          ],
          )
        )
      ),
    );
  }
}


class SettingsView extends StatelessWidget {
  final MQTTController controller;
  const SettingsView({super.key, required this.controller});

  // Simulated connection status: 0 for not connected, 1 for connected
  final int connectionStatus = 1; // Change this to 0 or 1 to simulate different states

  
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Settings'),
      ),
      body: Padding(
        padding: const EdgeInsets.only(top: 20.0), // Add space from the top
        child: Column(
          children: [
            Row(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                Text('Connection with ESP: ${connectionStatus == 1 ? 'Connected' : 'Not Connected'}'),
                const SizedBox(width: 20), // Space between rows
                ElevatedButton(
                  onPressed: () {
                    // Action to perform on disconnect
                    print('Disconnected from ESP');
                  },
                  child: const Text('Disconnect'),
                )
              ],
            ),
            const SizedBox(height: 20),
            Row(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                Text('Sranie w banie: ${connectionStatus == 1 ? 'XD!' : 'XD?'}'),
                const SizedBox(width: 20), // Space between rows
                ElevatedButton(
                  onPressed: () {
                    // Action to perform on disconnect
                    print('Sraken');
                  },
                  child: const Text('Sraken'),
                )
              ],
            ),
          ],
        ),
      ),
    );
  }
}
