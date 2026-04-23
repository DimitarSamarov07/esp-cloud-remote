import 'dart:async';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:get/get.dart';
import 'package:permission_handler/permission_handler.dart';
import 'dart:developer';

class BleController extends GetxController {
  var scanResults = <ScanResult>[].obs;
  var isScanning = false.obs;
  var connectionState = BluetoothConnectionState.disconnected.obs;
  var adapterState = BluetoothAdapterState.unknown.obs;

  StreamSubscription? _scanResultsSubscription;
  StreamSubscription? _isScanningSubscription;
  StreamSubscription? _connectionStateSubscription;
  StreamSubscription? _adapterStateSubscription;

  @override
  void onInit() {
    super.onInit();
    _adapterStateSubscription = FlutterBluePlus.adapterState.listen((state) {
      adapterState.value = state;
    });
  }

  @override
  void onClose() {
    _scanResultsSubscription?.cancel();
    _isScanningSubscription?.cancel();
    _connectionStateSubscription?.cancel();
    _adapterStateSubscription?.cancel();
    super.onClose();
  }

  Future<bool> _checkPermissions() async {
    Map<Permission, PermissionStatus> statuses = await [
      Permission.bluetoothScan,
      Permission.bluetoothConnect,
      Permission.locationWhenInUse,
    ].request();

    return statuses.values.every((status) => status.isGranted);
  }

  Future<void> scanDevices() async {
    bool permissionsGranted = await _checkPermissions();
    if (!permissionsGranted) {
      log("Permissions not granted");
      return;
    }

    // Ensure Bluetooth is ON
    if (await FlutterBluePlus.adapterState.first != BluetoothAdapterState.on) {
      log("Bluetooth adapter is not ON");
      return;
    }

    // Stop existing scan if any
    if (FlutterBluePlus.isScanningNow) {
      await FlutterBluePlus.stopScan();
    }

    scanResults.clear();

    // Setup scanning state listener
    _isScanningSubscription?.cancel();
    _isScanningSubscription = FlutterBluePlus.isScanning.listen((scanning) {
      isScanning.value = scanning;
    });

    // Setup results listener
    _scanResultsSubscription?.cancel();
    _scanResultsSubscription = FlutterBluePlus.onScanResults.listen(
      (results) {
        // Filter to keep only devices with a name
        final filteredResults = results.where((r) => r.advertisementData.advName.isNotEmpty).toList();
        scanResults.assignAll(filteredResults);
      },
      onError: (e) => log("Scan Results Error: $e"),
    );

    try {
      await FlutterBluePlus.startScan(
        timeout: const Duration(seconds: 15),
        androidUsesFineLocation: true,
      );
    } catch (e) {
      log("Error starting scan: $e");
    }
  }

  Future<void> connectToDevice(BluetoothDevice device) async {
    try {
      // 1. Stop scanning before connecting (Recommended for stability)
      if (FlutterBluePlus.isScanningNow) {
        await FlutterBluePlus.stopScan();
      }

      // 2. Listen for connection state changes BEFORE connecting
      _connectionStateSubscription?.cancel();
      _connectionStateSubscription = device.connectionState.listen((BluetoothConnectionState state) {
        connectionState.value = state;
        log("Connection state changed: $state for ${device.remoteId}");
      });

      // 3. Connect with timeout
      await device.connect(timeout: const Duration(seconds: 10), autoConnect: false, license: License.free);
      log("Connect command successful for ${device.remoteId}");

    } catch (e) {
      log("Error connecting to device ${device.remoteId}: $e");
      connectionState.value = BluetoothConnectionState.disconnected;
    }
  }

  Future<void> disconnectDevice(BluetoothDevice device) async {
    try {
      await device.disconnect();
      log("Disconnected from ${device.remoteId}");
    } catch (e) {
      log("Error during disconnect: $e");
    } finally {
      connectionState.value = BluetoothConnectionState.disconnected;
      _connectionStateSubscription?.cancel();
    }
  }
}
