import 'package:flutter/material.dart';
import 'package:flutter/rendering.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:get/get.dart';
import 'package:my_flutter_app/admin_panel.dart';
import 'ble_controller.dart';
import 'package:permission_handler/permission_handler.dart';


class ConnectionPage extends StatefulWidget {
  const ConnectionPage({super.key});

  @override
  State<ConnectionPage> createState() => _ConnectionPageState();
}

class _ConnectionPageState extends State<ConnectionPage> {
  List<ScanResult> results = [];
  bool isScanning = false;

  @override
  void initState() {
    super.initState();
    _requestPermissionsAndScan();
  }

  Future<void> _requestPermissionsAndScan() async {
    Map<Permission, PermissionStatus> statuses = await [
      Permission.bluetoothScan,
      Permission.bluetoothConnect,
      Permission.locationWhenInUse,
    ].request();

    statuses.forEach((permission, status) {
      print('$permission: $status');
    });

    bool allGranted = statuses.values.every((s) => s.isGranted);
    if (!allGranted) {
      print('[X] Permissions not fully granted — scan will find nothing');
      return;
    }

    await FlutterBluePlus.adapterState
        .where((s) => s == BluetoothAdapterState.on)
        .first;

    setState(() => isScanning = true);

    var sub = FlutterBluePlus.onScanResults.listen((results) {
      setState(() => results = results);
    });

    FlutterBluePlus.cancelWhenScanComplete(sub);

    await FlutterBluePlus.startScan(timeout: Duration(seconds: 10));

    await FlutterBluePlus.isScanning.where((v) => v == false).first;
    setState(() => isScanning = false);
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: Text(isScanning ? 'Scanning...' : 'BLE Devices')),
      body: ListView.builder(
        itemCount: results.length,
        itemBuilder: (context, i) {
          final r = results[i];
          final name = r.advertisementData.advName.isNotEmpty
              ? r.advertisementData.advName
              : 'Unknown Device';
          return ListTile(
            title: Text(name),
            subtitle: Text(r.device.remoteId.toString()),
            trailing: Text('${r.rssi} dBm'),
          );
        },
      ),
    );
  }
}