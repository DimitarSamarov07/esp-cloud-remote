import 'dart:math';

// import 'package:flutter_blue/flutter_blue.dart'
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:get/get.dart';
import 'package:permission_handler/permission_handler.dart';
import 'dart:developer';

class BleController extends GetxController {
  // flutter_blue is deprecated and unmaintained. Use flutter_blue_plus later

  Future<void> requestPermissions() async {
    await [
      Permission.bluetoothScan,
      Permission.bluetoothConnect,
      Permission.locationWhenInUse
    ].request();
  }

  Future<void> scanDevices() async {
    await requestPermissions();
    await FlutterBluePlus.adapterState
        .where((state) => state == BluetoothAdapterState.on)
        .first;

    var sub = FlutterBluePlus.onScanResults.listen((results) {
      for (ScanResult r in results) {
        print('Found: ${r.device.remoteId} - "${r.advertisementData.advName}"');
      }
    });


    FlutterBluePlus.cancelWhenScanComplete(sub);

    await FlutterBluePlus.startScan(timeout: Duration(seconds: 10));

    await FlutterBluePlus.isScanning.where((v) => v == false).first;
  }

}