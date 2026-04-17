import 'dart:convert';

import 'package:flutter/material.dart';
import 'package:flutter_svg/flutter_svg.dart';
import 'package:http/http.dart' as http;
import 'package:my_flutter_app/connection.dart';
import 'package:my_flutter_app/wifi_dialog.dart';
import 'dart:developer';

const String serverURL = 'http://90.154.171.96:8690';

class DeviceData {
  String deviceID;
  String name;
  int temp;
  String mode;
  bool power;
  int fanSpeed;
  bool swing;

  DeviceData({
    required this.deviceID,
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
    DeviceData(name: 'ESP32 Kitchen', deviceID: 'mqtt_pc_758fb8'),
    DeviceData(name: 'ESP32 Living Room', deviceID: 'B_mqtt_pc_758fb8'),
    DeviceData(name: 'ESP32 Bedroom', deviceID: 'C_mqtt_pc_758fb8'),
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
        _devices[index].name = result['name'];
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
                onPressed: (){
                  Navigator.pushReplacement(
                      context,
                      MaterialPageRoute(builder: (context) => ConnectionPage())
                  );
                },
                style: OutlinedButton.styleFrom(
                  textStyle: const TextStyle(color: Colors.blue),
                  side: const BorderSide(color: Colors.blue, width: 1),
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
        elevation: 0,
        foregroundColor: Colors.black,
      ),
      body: _buildDeviceList(),
    );
  }

  Widget _welcomeText() {
    return const Padding(
      padding: EdgeInsets.symmetric(vertical: 20.0),
      child: Center(
        child: Text.rich(
          TextSpan(
            text: 'Welcome back, ',
            style: TextStyle(
              fontSize: 26,
              fontWeight: FontWeight.bold,
              fontStyle: FontStyle.italic,
              color: Colors.black,
            ),
            children: [
              TextSpan(
                text: 'Pesho!',
                style: TextStyle(color: Colors.blue),
              ),
            ],
          ),
        ),
      ),
    );
  }

  Widget _statusBoxes() {
    int activeAcs = _devices.where((d) => d.power).length;
    int totalEsps = _devices.length;

    return Padding(
      padding: const EdgeInsets.symmetric(horizontal: 16.0),
      child: Row(
        children: [
          Expanded(
            child: _buildStatusCard(
              title: 'Active ACs',
              value: activeAcs.toString(),
              icon: Icons.toys_outlined,
            ),
          ),
          const SizedBox(width: 16),
          Expanded(
            child: _buildStatusCard(
              title: 'Total ESPs',
              value: totalEsps.toString(),
              icon: Icons.memory_outlined,
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildStatusCard({required String title, required String value, required IconData icon}) {
    return Container(
      padding: const EdgeInsets.all(16),
      decoration: BoxDecoration(
        color: const Color(0xFFF2F2F2),
        borderRadius: BorderRadius.circular(16),
        border: Border.all(color: Colors.grey.shade300, width: 1.5),
      ),
      child: Column(
        children: [
          Text(
            title,
            style: TextStyle(
              color: Colors.grey.shade600,
              fontWeight: FontWeight.w600,
              fontSize: 15,
            ),
          ),
          const SizedBox(height: 12),
          Row(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              Icon(icon, size: 38, color: Colors.grey.shade600),
              const SizedBox(width: 8),
              Text(
                value,
                style: const TextStyle(
                  fontSize: 32,
                  fontWeight: FontWeight.bold,
                ),
              ),
            ],
          ),
        ],
      ),
    );
  }
  Widget _buildDeviceList() {
    return ListView.separated(
      padding: const EdgeInsets.only(bottom: 16),
      itemCount: _devices.length + 4,
      separatorBuilder: (context, index) {
        if (index == 0) return const SizedBox.shrink();
        return Divider(
          color: Colors.grey[300],
          height: 1,
          thickness: 1.6,
        );
      },
      itemBuilder: (BuildContext context, int index) {
        if (index == 0) return _welcomeText();
        if (index == 1) return _statusBoxes();

        if (index == 2) {
          return Padding(
            padding: const EdgeInsets.fromLTRB(16, 16, 16, 8),
            child: Row(
              mainAxisAlignment: MainAxisAlignment.spaceBetween,
              children: [
                const Text(
                  'Admin Panel',
                  style: TextStyle(fontSize: 22, fontWeight: FontWeight.bold),
                ),
                OutlinedButton(
                  onPressed: () {},
                  style: OutlinedButton.styleFrom(
                    shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(20)),
                    side: BorderSide(color: Colors.grey.shade400),
                    padding: const EdgeInsets.symmetric(horizontal: 20),
                  ),
                  child: Text('All ESPs', style: TextStyle(color: Colors.grey.shade600, fontWeight: FontWeight.w500)),
                ),
              ],
            ),
          );
        }

        if (index == 3) {
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

        final deviceIndex = index - 4;
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
  late TextEditingController _nameController;
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
        case 1:
          goofyFan = 'off';
          break;
        case 2:
          goofyFan = 'silent';
          break;
        case 4:
          goofyFan = 'strong';
          break;
        case 0:
        default:
          goofyFan = 'auto';
      }

      log('POSTING: \n\tname: ${_nameController.text}, \n\tid: $deviceID, \n\ttemp: $localTemp, \n\tfanSpeed: $goofyFan, \n\tswing: $localSwing, \n\tpower: $goofyPower, \n\tmode: $goofyMode');
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

      log('POSTED: \n\tid: $deviceID, \n\ttemp: $localTemp, \n\tfanSpeed: $goofyFan, \n\tswing: $localSwing, \n\tpower: $goofyPower, \n\tmode: $goofyMode');

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
    _nameController = TextEditingController(text: widget.device.name);
    localTemp = widget.device.temp;
    localMode = widget.device.mode;
    localPower = widget.device.power;
    localSwing = widget.device.swing;
    localFanSpeed = widget.device.fanSpeed;
  }

  @override
  void dispose() {
    _nameController.dispose();
    super.dispose();
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
                padding: EdgeInsets.symmetric(horizontal: scale(16), vertical: scale(4)),
                decoration: BoxDecoration(color: Colors.grey[100], borderRadius: BorderRadius.circular(15)),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Text('Name*', style: TextStyle(color: Colors.grey[600], fontSize: scale(12))),
                    TextField(
                      controller: _nameController,
                      style: TextStyle(fontSize: scale(16), fontWeight: FontWeight.w500),
                      decoration: const InputDecoration(
                        isDense: true,
                        border: InputBorder.none,
                        contentPadding: EdgeInsets.symmetric(vertical: 4),
                      ),
                    ),
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
                        'name': _nameController.text,
                        'temp': localTemp,
                        'mode': localMode,
                        'power': localPower,
                        'fanSpeed': localFanSpeed,
                        'swing': localSwing,
                      });
                      setAcData(widget.device.deviceID); // not the right place. i know
                    },
                    child: Text('SEND', style: TextStyle(color: Colors.cyan, fontWeight: FontWeight.bold, fontSize: scale(14))),
                  ),
                ],
              ),
            ],
          ),
        ),
      )
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
