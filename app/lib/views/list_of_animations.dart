
import 'package:conn_app/enums/connectivity_status.dart';
import 'package:conn_app/views/pre_connection_view.dart';
import 'package:flutter/material.dart';
import 'package:file_picker/file_picker.dart';  // File picker package needed to upload files
import 'dart:typed_data';
import 'package:conn_app/controllers/mqtt_controller.dart';


class ListOfAnimations extends StatefulWidget {
  final MQTTController controller;
  ListOfAnimations({required this.controller});
  //ListOfAnimations({super.key});

  @override
  _ListOfAnimationsState createState() => _ListOfAnimationsState(controller: controller);
  //_ListOfAnimationsState(controller: controller);
}

class _ListOfAnimationsState extends State<ListOfAnimations> {

  final MQTTController controller;
  _ListOfAnimationsState({required this.controller});

  List<String> items = ['Item 1', 'Item 2', 'Item 3']; // List of items

  @override
  void initState() {
    super.initState();
    _runOnLoad();
  }

  void _runOnLoad() {
    print('List loaded or returned to');

    setState(() {
      //Get loaded animations from esp

      items = ['Item 1', 'Item 2', 'Item 3'];
    });

    controller.espStatus.listen((event) {
      if (event != ConnectivityStatus.connected) {
        print('ESP is disconnected');

        Navigator.push(
          context,
          MaterialPageRoute(builder: (context) => PreConnectionView()),
        );
      }
    });
  }

  void _deleteItem(int index) {
    setState(() {
      items.removeAt(index); // Remove item from the list
    });
  }

  void _playItem(String item) {
    // Placeholder function for the "Play" button action
    print('Playing $item');
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text('List of animations'),
      ),
      body: Column(
        children: [
          Padding(
            padding: const EdgeInsets.symmetric(horizontal: 16.0, vertical: 8.0),  // Add some padding to the top row
            child: Row(
              mainAxisAlignment: MainAxisAlignment.spaceBetween,  // Spread buttons across the row
              children: [
                // Settings Button (top-left)
                IconButton(
                  icon: Icon(Icons.settings),
                  onPressed: () {
                    Navigator.push(
                      context,
                      MaterialPageRoute(builder: (context) => SettingsView(controller: this.controller)), // Navigate to SettingsView
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
                      MaterialPageRoute(builder: (context) => SecondView(controller: this.controller)), // Navigate to SecondView
                    ).then((_) {
                      _runOnLoad(); // Call the function when returning
                    });
                  },
                  child: Text('Upload a new animation'),
                ),
              ],
            ),
          ),
          // List of items with Play and Delete buttons
          Expanded(
            child: ListView.separated(
              itemCount: items.length,
              separatorBuilder: (context, index) => Divider(),  // Divider between items
              itemBuilder: (context, index) {
              return Padding(
                padding: const EdgeInsets.symmetric(vertical: 4.0),
                child: Card(  // Moved Card to the correct position
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
                        _deleteItem(index);
                      },
                    ),
                  ),
                ),
              );
              },
            ),
          ),
        ],
      ),
    );
  }


}

class SecondView extends StatefulWidget {
  final MQTTController controller;
  SecondView({required this.controller});

  @override
  _SecondViewState createState() => _SecondViewState(controller: controller);
}

class _SecondViewState extends State<SecondView> {
  final MQTTController controller;
  _SecondViewState({required this.controller});

  String? _fileName; // To store the file name
  Uint8List? _fileBytes; // To store file bytes
  TextEditingController _textController = TextEditingController();  // Controller to modify file name

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


  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text('Uploading a file'),
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
              child: Text('Pick a .dat file'),
            ),
            if (_fileName != null) ...[
              SizedBox(height: 20),
              Text(_fileName ?? ''),
              TextField(
                  controller: _textController,
                  decoration: InputDecoration(
                    border: OutlineInputBorder(),
                    labelText: 'Edit animation name',
                  ),
                ),
            ],
            SizedBox(height: 20),
            ElevatedButton(
              onPressed: (_fileName != null && _fileBytes != null && _fileBytes!.isNotEmpty)
                  ? () {
                      _sendFile();
                      Navigator.pop(context);  // Go back to FirstView
                    }
                  : null,  // Disable button if no file is selected or file length is 0
              child: Text('Send the animation'),
              style: ElevatedButton.styleFrom(
                backgroundColor: _fileName != null ? null : Colors.grey,  // Grey color when inactive
              ),
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
  SettingsView({required this.controller});

  // Simulated connection status: 0 for not connected, 1 for connected
  final int connectionStatus = 1; // Change this to 0 or 1 to simulate different states

  
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text('Settings'),
      ),
      body: Padding(
        padding: const EdgeInsets.only(top: 20.0), // Add space from the top
        child: Column(
          children: [
            Row(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                Text('Connection with ESP: ${connectionStatus == 1 ? 'Connected' : 'Not Connected'}'),
                SizedBox(width: 20), // Space between rows
                ElevatedButton(
                  onPressed: () {
                    // Action to perform on disconnect
                    print('Disconnected from ESP');
                  },
                  child: Text('Disconnect'),
                )
              ],
            ),
            SizedBox(height: 20),
            Row(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                Text('Sranie w banie: ${connectionStatus == 1 ? 'XD!' : 'XD?'}'),
                SizedBox(width: 20), // Space between rows
                ElevatedButton(
                  onPressed: () {
                    // Action to perform on disconnect
                    print('Sraken');
                  },
                  child: Text('Sraken'),
                )
              ],
            ),
          ],
        ),
      ),
    );
  }
}
