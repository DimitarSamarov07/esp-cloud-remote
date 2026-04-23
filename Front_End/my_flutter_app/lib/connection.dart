import 'package:flutter/material.dart';
import 'package:get/get.dart';
import 'ble_controller.dart';


class ConnectionPage extends StatelessWidget {
  const ConnectionPage({super.key});

  @override
  Widget build(BuildContext context) {
    final BleController controller = Get.put(BleController());

    return Scaffold(
      appBar: AppBar(title: const Text('BLE Devices')),
      body: Center(
        child: Column(
          children: [
            Obx(() => Text("Status: ${controller.connectionState.value.name}")),
            const SizedBox(height: 10),
            ElevatedButton(
              onPressed: () => controller.scanDevices(),
              child: const Text("Scan Devices"),
            ),
            const SizedBox(height: 10),
            Expanded(
              child: Obx(() => ListView.builder(
                    itemCount: controller.scanResults.length,
                    itemBuilder: (context, i) {
                      final r = controller.scanResults[i];
                      final name = r.advertisementData.advName;
                      return ListTile(
                        title: Text(name),
                        subtitle: Text(r.device.remoteId.toString()),
                        trailing: Text('${r.rssi} dBm'),
                        onTap: () => controller.connectToDevice(r.device),
                      );
                    },
                  )),
            ),
          ],
        ),
      ),
    );
  }
}