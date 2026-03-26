import 'dart:convert';

import 'package:flutter/material.dart';
import 'package:flutter_svg/flutter_svg.dart';
import 'package:http/http.dart' as http;
import 'connection.dart';
import 'dart:developer';

const String serverURL = 'http://90.154.171.96:8690';

class DeviceData {
  String name;
  int temp;
  String mode;
  bool power;
  int fanSpeed;
  bool swing;

  DeviceData({
    required this.name,
    this.temp = 25,
    this.mode = 'Cool',
    this.power = true,
    this.fanSpeed = 2,
    this.swing = false
  });
}

class AdminPage extends StatefulWidget {
  const AdminPage({super.key});

  @override
  State<AdminPage> createState() => _AdminPageState();
}

class _AdminPageState extends State<AdminPage> {
  // List of devices with their individual states
  final List<DeviceData> _devices = [
    DeviceData(name: 'ESP32 Kitchen'),
    DeviceData(name: 'ESP32 Living Room', temp: 22, power: false, mode: 'Heat'),
    DeviceData(name: 'ESP32 Bedroom', temp: 24),
  ];

  void _showSettingsDialog(int index) async {
    final result = await showDialog<Map<String, dynamic>>(
      context: context,
      builder: (context) => EspSettingsDialog(
        device: _devices[index],
      ),
    );

    if (result != null && mounted) {
      setState(() {
        _devices[index].temp = result['temp'];
        _devices[index].mode = result['mode'];
        _devices[index].power = result['power'];
        _devices[index].fanSpeed = result['fanSpeed'];
        _devices[index].swing = result['swing'];
      });
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: Colors.grey[100],
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
                onPressed: (){
                  Navigator.pushReplacement(
                      context,
                      MaterialPageRoute(builder: (context) => ConnectionPage())
                  );
                },
                style: OutlinedButton.styleFrom(
                  textStyle: TextStyle(color: Colors.blue),
                  side: BorderSide(color: Colors.blue, width: 1),
                  shape: RoundedRectangleBorder(
                      borderRadius: BorderRadius.circular(40)
                  ),
                ),
                child: const Text(
                  'Add ESP',
                  style: TextStyle(
                      color: Colors.blue
                  ),
                )
            ),
          )
        ],
        backgroundColor: Colors.white,
        elevation: 0.5,
        foregroundColor: Colors.black,
      ),
      body: _buildDeviceList(),
    );
  }

  Widget _buildDeviceList() {
    return ListView.separated(
      padding: const EdgeInsets.symmetric(vertical: 16),
      itemCount: _devices.length + 2,
      separatorBuilder: (context, index) => Divider(
        color: Colors.grey[300],
        height: 1,
        thickness: 1.6,
      ),
      itemBuilder: (BuildContext context, int index) {
        if (index == 0) {
          return const ListTile(
            title: Text(
              'Admin Panel',
              style: TextStyle(fontSize: 22, fontWeight: FontWeight.w600),
            ),
          );
        }

        if (index == 1) {
          return const IgnorePointer(
            child: ListTile(
              title: Text(
                'ESP Name',
                style: TextStyle(fontWeight: FontWeight.w600, fontSize: 14, color: Colors.grey),
              ),
              leading: SizedBox(width: 24),
              trailing: Row(
                mainAxisSize: MainAxisSize.min,
                children: [
                  Text('Status', style: TextStyle(fontWeight: FontWeight.w600, fontSize: 14, color: Colors.grey)),
                  SizedBox(width: 26),
                  Text('AC', style: TextStyle(fontWeight: FontWeight.w600, fontSize: 14, color: Colors.grey)),
                ],
              ),
            ),
          );
        }

        final deviceIndex = index - 2;
        final device = _devices[deviceIndex];

        return ListTile(
          title: Text(device.name),
          tileColor: Colors.white,
          onTap: () => _showSettingsDialog(deviceIndex),
          leading: svgESP(),
          trailing: Row(
            mainAxisSize: MainAxisSize.min,
            children: [
              _espStatusText(true),
              const SizedBox(width: 26),
              _acStatusText(device.power),
            ],
          ),
        );
      },
    );
  }

  Widget svgESP() {
    return SvgPicture.asset(
      "assets/esp.svg",
      width: 24,
      height: 24,
      placeholderBuilder: (context) => const Icon(Icons.developer_board, color: Colors.cyan),
    );
  }

  Widget _espStatusText(bool statusOn) {
    return Text(
      statusOn ? 'Active' : 'Inactive',
      style: TextStyle(
        fontWeight: FontWeight.w600,
        fontSize: 14,
        color: statusOn ? Colors.green : Colors.red,
      ),
    );
  }

  Widget _acStatusText(bool statusOn) {
    return Text(
      statusOn ? 'ON' : 'OFF',
      style: TextStyle(
        fontWeight: FontWeight.w600,
        fontSize: 14,
        color: statusOn ? Colors.green : Colors.red,
      ),
    );
  }
}

// Separate StatefulWidget for the Settings Dialog
class EspSettingsDialog extends StatefulWidget {
  final DeviceData device;

  const EspSettingsDialog({super.key, required this.device});

  @override
  State<EspSettingsDialog> createState() => _EspSettingsDialogState();
}

class _EspSettingsDialogState extends State<EspSettingsDialog> {
  late int localTemp;
  late String localMode;
  late bool localPower;
  late int localFanSpeed;
  late bool localSwing;

  final List<String> _modes = ['Cool', 'Heat', 'Dry', 'Auto'];

  void setAcData(String deviceID) async {
    try {
      log('Sending POST to $serverURL/setAcData...');

      int goofyPower = localPower ? 1 : 0;
      String goofyMode = localMode.toLowerCase();

      String goofyFan;
      switch (localFanSpeed) {
        case 0:
          goofyFan = 'auto';
          break;
        case 1:
          goofyFan = 'off';
          break;
        case 2:
          goofyFan = 'silent';
          break;
        case 4:
          goofyFan = 'strong';
          break;
        default:
          goofyFan = 'auto';
      }

      log(goofyPower.runtimeType.toString());
      final response = await http.post(
        Uri.parse('$serverURL/setAcData'),
        headers: {'Content-Type': 'application/json'},
        body: jsonEncode({
          'deviceID': deviceID,
          'temp': localTemp,
          'fanSpeed': goofyFan,
          'swing': localSwing,
          'mode': goofyMode,
          'power': goofyPower
        }),
      );

      log('POSTED: id: $deviceID, temp: $localTemp, fanSpeed: $goofyFan, swing: $localSwing, power: $goofyPower, mode: $goofyMode');

      if (response.statusCode == 200) {
        log("sent");
      } else {
        throw Exception('POST failed with code: ${response.statusCode}');
      }
    } catch (e) {
      throw Exception(e);
    }
  }

  @override
  void initState() {
    super.initState();
    localTemp = widget.device.temp;
    localMode = widget.device.mode;
    localPower = widget.device.power;
    localSwing = widget.device.swing;
    localFanSpeed = widget.device.fanSpeed;
  }

  @override
  Widget build(BuildContext context) {
    // Get screen width to make font sizes responsive
    final double screenWidth = MediaQuery.of(context).size.width;

    double scale(double size) => size * (screenWidth / 400.0).clamp(0.7, 1.3);

    return Dialog(
      shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(20)),
      child: SingleChildScrollView(
        child: Padding(
          padding: EdgeInsets.all(scale(24)),
          child: Column(
            mainAxisSize: MainAxisSize.min,
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceBetween,
                children: [
                  Text('Edit Settings', style: TextStyle(fontSize: scale(22), fontWeight: FontWeight.bold)),
                  IconButton(
                    onPressed: () => Navigator.pop(context),
                    icon: const Icon(Icons.close),
                    padding: EdgeInsets.zero,
                    constraints: const BoxConstraints(),
                    iconSize: scale(24),
                  ),
                ],
              ),
              SizedBox(height: scale(20)),
              Container(
                width: double.infinity,
                padding: EdgeInsets.symmetric(horizontal: scale(16), vertical: scale(10)),
                decoration: BoxDecoration(color: Colors.grey[100], borderRadius: BorderRadius.circular(15)),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Text('Name*', style: TextStyle(color: Colors.grey[600], fontSize: scale(12))),
                    Text(widget.device.name, style: TextStyle(fontSize: scale(16), fontWeight: FontWeight.w500)),
                  ],
                ),
              ),
              SizedBox(height: scale(20)),
              Row(
                children: [
                  Text('Temperature: ', style: TextStyle(fontSize: scale(18))),
                  IconButton(
                    onPressed: () => setState(() => localTemp--),
                    icon: Icon(Icons.remove, size: scale(18)),
                    padding: EdgeInsets.zero,
                    constraints: const BoxConstraints(),
                  ),
                  SizedBox(width: scale(4)),
                  Text('$localTemp', style: TextStyle(fontSize: scale(18), color: Colors.cyan, decoration: TextDecoration.underline, fontWeight: FontWeight.bold)),
                  Text('°C', style: TextStyle(fontSize: scale(18), fontWeight: FontWeight.bold)),
                  SizedBox(width: scale(4)),
                  IconButton(
                    onPressed: () => setState(() => localTemp++),
                    icon: Icon(Icons.add, size: scale(18)),
                    padding: EdgeInsets.zero,
                    constraints: const BoxConstraints(),
                  ),
                ],
              ),
              SizedBox(height: scale(12)),
              _buildListSelector('Mode', localMode, _modes, (newValue) {
                setState(() => localMode = newValue);
              }, scale),
              SizedBox(height: scale(12)),
              Row(
                children: [
                  Text('Fan: ', style: TextStyle(fontSize: scale(18))),
                  ...List.generate(3, (index) {
                    bool isActive = index < localFanSpeed && localFanSpeed != 0;
                    return IconButton(
                      onPressed: () => setState(() => localFanSpeed = index + 1),
                      icon: Icon(Icons.wind_power, color: isActive ? Colors.cyan : Colors.grey[300], size: scale(20)),
                      padding: EdgeInsets.symmetric(horizontal: scale(2)),
                      constraints: const BoxConstraints(),
                    );
                  }),
                  const Spacer(),
                  Text('Auto', style: TextStyle(fontSize: scale(14))),
                  Checkbox(
                      value: localFanSpeed == 0,
                      onChanged: (bool? value) {
                        setState(() {
                          localFanSpeed = (value == true) ? 0 : 2; // Default to 2 if unchecked
                        });
                      },
                      activeColor: Colors.cyan)
                ],
              ),
              SizedBox(height: scale(12)),
              Row(
                children: [
                  Text('Swing (OFF/ON): ', style: TextStyle(fontSize: scale(18))),
                  Transform.scale(
                    scale: (screenWidth / 375.0).clamp(0.8, 1.2),
                    child: Switch(
                      value: localSwing,
                      onChanged: (bool value) => setState(() => localSwing = value),
                      activeThumbColor: Colors.cyan,
                    ),
                  ),
                ],
              ),
              SizedBox(height: scale(12)),
              Row(
                children: [
                  Text('Power (OFF/ON): ', style: TextStyle(fontSize: scale(18))),
                  Transform.scale(
                    scale: (screenWidth / 375.0).clamp(0.8, 1.2),
                    child: Switch(
                      value: localPower,
                      onChanged: (bool value) => setState(() => localPower = value),
                      activeThumbColor: Colors.cyan,
                    ),
                  ),
                ],
              ),
              SizedBox(height: scale(24)),
              Row(
                mainAxisAlignment: MainAxisAlignment.end,
                children: [
                  TextButton(
                    onPressed: () => Navigator.pop(context),
                    child: Text('CANCEL', style: TextStyle(color: Colors.cyan, fontWeight: FontWeight.bold, fontSize: scale(14))),
                  ),
                  SizedBox(width: scale(16)),
                  TextButton(
                    onPressed: () {
                      Navigator.pop(context, {
                        'temp': localTemp,
                        'mode': localMode,
                        'power': localPower,
                        'fanSpeed': localFanSpeed,
                        'swing': localSwing,
                      });
                      setAcData('mqtt_pc_758fb8'); // not the right place. i know
                    },
                    child: Text('SEND', style: TextStyle(color: Colors.cyan, fontWeight: FontWeight.bold, fontSize: scale(14))),
                  ),
                ],
              ),
            ],
          ),
        ),
      ),
    );
  }

  Widget _buildListSelector(String label, String currentValue, List<String> items, ValueChanged<String> onChanged, double Function(double) scale) {
    return Row(
      children: [
        Text('$label: ', style: TextStyle(fontSize: scale(18))),
        IconButton(
          onPressed: () {
            int index = items.indexOf(currentValue);
            onChanged(items[(index - 1 + items.length) % items.length]);
          },
          icon: Icon(Icons.chevron_left, size: scale(22)),
          padding: EdgeInsets.zero,
          constraints: const BoxConstraints(),
        ),
        SizedBox(width: scale(4)),
        Text(currentValue, style: TextStyle(fontSize: scale(18), color: Colors.cyan, decoration: TextDecoration.underline, fontWeight: FontWeight.bold)),
        SizedBox(width: scale(4)),
        IconButton(
          onPressed: () {
            int index = items.indexOf(currentValue);
            onChanged(items[(index + 1) % items.length]);
          },
          icon: Icon(Icons.chevron_right, size: scale(22)),
          padding: EdgeInsets.zero,
          constraints: const BoxConstraints(),
        ),
      ],
    );
  }
}
