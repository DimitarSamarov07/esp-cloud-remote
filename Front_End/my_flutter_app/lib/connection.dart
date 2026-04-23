import 'package:flutter/material.dart';
import 'package:flutter_svg/flutter_svg.dart';
import 'package:get/get.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'dart:developer';
import 'ble_controller.dart';

class ConnectionPage extends StatefulWidget {
  const ConnectionPage({super.key});

  @override
  State<ConnectionPage> createState() => _ConnectionPageState();
}

class _ConnectionPageState extends State<ConnectionPage> with SingleTickerProviderStateMixin {
  final BleController controller = Get.put(BleController());
  late AnimationController _animationController;

  @override
  void initState() {
    super.initState();
    _animationController = AnimationController(
      vsync: this,
      duration: const Duration(seconds: 2),
    );

    // Watch scanning state to start/stop animation
    ever(controller.isScanning, (bool scanning) {
      if (scanning) {
        _animationController.repeat();
      } else {
        _animationController.stop();
        _animationController.reset();
      }
    });

    if (controller.isScanning.value) {
      _animationController.repeat();
    }

    // Start scanning when the page opens
    controller.scanDevices();
  }

  @override
  void dispose() {
    _animationController.dispose();
    super.dispose();
  }

  Widget _buildScanningCircle(int index) {
    return AnimatedBuilder(
      animation: _animationController,
      builder: (context, child) {
        if (!controller.isScanning.value) {
          double size = 140.0 - (index * 30.0);
          return Container(
            width: size,
            height: size,
            decoration: BoxDecoration(
              shape: BoxShape.circle,
              border: Border.all(color: Colors.grey.shade200, width: 2),
            ),
          );
        }

        double value = (_animationController.value + (index * 0.33)) % 1.0;
        double opacity = (1.0 - value).clamp(0.0, 1.0);
        double scale = 0.5 + (value * 1.2);

        return Opacity(
          opacity: opacity,
          child: Transform.scale(
            scale: scale,
            child: Container(
              width: 140,
              height: 140,
              decoration: BoxDecoration(
                shape: BoxShape.circle,
                border: Border.all(color: Colors.blue.withValues(alpha: 0.5), width: 2),
              ),
            ),
          ),
        );
      },
    );
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: Colors.white,
      appBar: AppBar(
        leadingWidth: 140,
        leading: Padding(
          padding: const EdgeInsets.all(8.0),
          child: Image.asset(
            'assets/espcr.png',
            errorBuilder: (context, error, stackTrace) => const Icon(Icons.broken_image),
          ),
        ),
        actions: <Widget>[
          Padding(
            padding: const EdgeInsets.all(8.0),
            child: OutlinedButton(
              onPressed: () => Navigator.pop(context),
              style: OutlinedButton.styleFrom(
                textStyle: const TextStyle(color: Colors.blue),
                side: const BorderSide(color: Colors.blue, width: 1),
                shape: RoundedRectangleBorder(
                  borderRadius: BorderRadius.circular(40),
                ),
              ),
              child: const Text(
                'Home',
                style: TextStyle(color: Colors.blue),
              ),
            ),
          )
        ],
        backgroundColor: Colors.white,
        elevation: 0,
        foregroundColor: Colors.black,
      ),
      body: SafeArea(
        child: SingleChildScrollView(
          child: Column(
            children: [
              const SizedBox(height: 20),

              // Bluetooth Status Warning
              Obx(() {
                if (controller.adapterState.value != BluetoothAdapterState.on) {
                  return AnimatedContainer(
                    duration: const Duration(milliseconds: 300),
                    margin: const EdgeInsets.symmetric(horizontal: 30, vertical: 10),
                    padding: const EdgeInsets.all(16),
                    decoration: BoxDecoration(
                      color: Colors.amber.shade50,
                      borderRadius: BorderRadius.circular(12),
                      border: Border.all(color: Colors.amber.shade200, width: 1.5),
                      boxShadow: [
                        BoxShadow(
                          color: Colors.black.withAlpha(10),
                          blurRadius: 8,
                          offset: const Offset(0, 4),
                        ),
                      ],
                    ),
                    child: Column(
                      mainAxisSize: MainAxisSize.min,
                      children: [
                        Row(
                          children: [
                            Icon(Icons.bluetooth_disabled, color: Colors.redAccent, size: 28),
                            const SizedBox(width: 16),
                            const Expanded(
                              child: Text(
                                'Bluetooth Required',
                                style: TextStyle(
                                  color: Colors.black87,
                                  fontWeight: FontWeight.bold,
                                  fontSize: 16,
                                ),
                              ),
                            ),
                          ],
                        ),
                        const SizedBox(height: 12),
                        const Text(
                          'To scan for nearby ESP devices, please enable Bluetooth on your device.',
                          style: TextStyle(color: Colors.black54, fontSize: 14),
                        ),
                        const SizedBox(height: 16),
                        Row(
                          mainAxisAlignment: MainAxisAlignment.end,
                          children: [
                            ElevatedButton(
                              onPressed: () async {
                                try {
                                  await FlutterBluePlus.turnOn();
                                } catch (e) {
                                  log("Error turning on Bluetooth: $e");
                                }
                              },
                              style: ElevatedButton.styleFrom(
                                backgroundColor: Colors.amber.shade800,
                                foregroundColor: Colors.white,
                                elevation: 0,
                                shape: RoundedRectangleBorder(
                                  borderRadius: BorderRadius.circular(8),
                                ),
                              ),
                              child: const Text('Turn On'),
                            ),
                          ],
                        ),
                      ],
                    ),
                  );
                }
                return const SizedBox.shrink();
              }),

              const SizedBox(height: 40),

              // Title Section
              const Text(
                'Available Bluetooth ESPs',
                style: TextStyle(
                  fontSize: 32,
                  fontWeight: FontWeight.bold,
                  color: Colors.black,
                ),
                textAlign: TextAlign.center,
              ),
              const SizedBox(height: 8),
              const Text(
                'Select an ESP below to connect',
                style: TextStyle(
                  fontSize: 16,
                  color: Colors.grey,
                ),
                textAlign: TextAlign.center,
              ),

              const SizedBox(height: 40),

              // Main Connection Box
              Container(
                margin: const EdgeInsets.symmetric(horizontal: 30),
                width: double.infinity,
                height: 300,
                decoration: BoxDecoration(
                  color: Colors.white,
                  borderRadius: BorderRadius.circular(12),
                  border: Border.all(color: Colors.grey.shade300),
                ),
                child: Obx(() {
                  if (controller.scanResults.isEmpty) {
                    return Center(
                      child: Column(
                        mainAxisAlignment: MainAxisAlignment.center,
                        children: [
                          Stack(
                            alignment: Alignment.center,
                            children: [
                              // Animated scanning circles
                              _buildScanningCircle(0),
                              _buildScanningCircle(1),
                              _buildScanningCircle(2),
                              SvgPicture.asset(
                                'assets/esp.svg',
                                width: 40,
                                colorFilter: const ColorFilter.mode(Colors.black, BlendMode.srcIn),
                              ),
                            ],
                          ),
                          const SizedBox(height: 20),
                          Text(
                            controller.isScanning.value ? "Scanning for devices..." : "No ESPs found",
                            style: const TextStyle(color: Colors.grey),
                          ),
                          if (!controller.isScanning.value)
                            TextButton(
                              onPressed: () => controller.scanDevices(),
                              child: const Text("Scan Again"),
                            ),
                        ],
                      ),
                    );
                  }

                  return ListView.separated(
                    padding: const EdgeInsets.all(10),
                    itemCount: controller.scanResults.length,
                    separatorBuilder: (context, index) => const Divider(),
                    itemBuilder: (context, index) {
                      final result = controller.scanResults[index];
                      return ListTile(
                        leading: const Icon(Icons.bluetooth, color: Colors.blue),
                        title: Text(
                          result.advertisementData.advName,
                          style: const TextStyle(fontWeight: FontWeight.bold),
                        ),
                        subtitle: Text(result.device.remoteId.toString()),
                        trailing: const Icon(Icons.chevron_right),
                        onTap: () => controller.connectToDevice(result.device),
                      );
                    },
                  );
                }),
              ),

              const SizedBox(height: 100),
            ],
          ),
        ),
      ),
    );
  }
}
