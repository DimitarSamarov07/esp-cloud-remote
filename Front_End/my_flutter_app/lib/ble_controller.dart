import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:get/get.dart';
import 'package:permission_handler/permission_handler.dart';
import 'dart:developer';

class BleController extends GetxController {

  var scanResults = <ScanResult>[].obs;
  var connectionState = BluetoothConnectionState.disconnected.obs;

  Future<void> requestPermissions() async {
    await [
      Permission.bluetoothScan,
      Permission.bluetoothConnect,
      Permission.locationWhenInUse
    ].request();
  }

  Future<void> scanDevices() async {
    await requestPermissions();

    // Clear previous results
    scanResults.clear();

    await FlutterBluePlus.adapterState
        .where((state) => state == BluetoothAdapterState.on)
        .first;

    var sub = FlutterBluePlus.onScanResults.listen((results) {
      // Filter to keep only devices with a name
      final filteredResults = results.where((r) => r.advertisementData.advName.isNotEmpty).toList();
      scanResults.assignAll(filteredResults);
      for (ScanResult r in filteredResults) {
        log('Found: ${r.device.remoteId} - "${r.advertisementData.advName}"');
      }
    });

    FlutterBluePlus.cancelWhenScanComplete(sub);

    await FlutterBluePlus.startScan(timeout: Duration(seconds: 10));

    await FlutterBluePlus.isScanning.where((v) => v == false).first;
  }

  Future<void> connectToDevice(BluetoothDevice device) async {
    try {
      // Stop scanning before connecting
      await FlutterBluePlus.stopScan();

      await device.connect();
      log("Connected to ${device.remoteId}");

      // Listen for connection state changes
      device.connectionState.listen((BluetoothConnectionState state) {
        connectionState.value = state;
        if (state == BluetoothConnectionState.disconnected) {
          log("Device disconnected: ${device.remoteId}");
        }
      });

    } catch (e) {
      log("Error connecting to device: $e");
    }
  }

  Future<void> disconnectDevice(BluetoothDevice device) async {
    await device.disconnect();
    connectionState.value = BluetoothConnectionState.disconnected;
    log("Disconnected from ${device.remoteId}");
  }

}