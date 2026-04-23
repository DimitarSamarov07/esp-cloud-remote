import 'dart:convert';
import 'dart:developer';
import 'package:flutter/material.dart';
import 'package:http/http.dart' as http;
import 'models.dart';

const String serverURL = 'http://90.154.171.96:8690';

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

  void _setAcData(String deviceID) async {
    try {
      int powerValue = localPower ? 1 : 0;
      String modeValue = localMode.toLowerCase();

      String fanValue;
      switch (localFanSpeed) {
        case 1: fanValue = 'off'; break;
        case 2: fanValue = 'silent'; break;
        case 4: fanValue = 'strong'; break;
        default: fanValue = 'auto';
      }

      await http.post(
        Uri.parse('$serverURL/setAcData'),
        headers: {'Content-Type': 'application/json'},
        body: jsonEncode({
          'deviceID': deviceID,
          'temp': localTemp,
          'fanSpeed': fanValue,
          'swing': localSwing,
          'mode': modeValue,
          'power': powerValue
        }),
      );
    } catch (e) {
      log('Error setting AC data: $e');
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
              _buildHeader(scale),
              SizedBox(height: scale(20)),
              _buildNameField(scale),
              SizedBox(height: scale(20)),
              _buildTempSelector(scale),
              SizedBox(height: scale(12)),
              _buildListSelector('Mode', localMode, _modes, (val) => setState(() => localMode = val), scale),
              SizedBox(height: scale(12)),
              _buildFanSelector(scale),
              SizedBox(height: scale(12)),
              _buildToggle('Swing', localSwing, (val) => setState(() => localSwing = val), screenWidth),
              SizedBox(height: scale(12)),
              _buildToggle('Power', localPower, (val) => setState(() => localPower = val), screenWidth),
              SizedBox(height: scale(24)),
              _buildActions(scale),
            ],
          ),
        ),
      ),
    );
  }

  Widget _buildHeader(double Function(double) scale) {
    return Row(
      mainAxisAlignment: MainAxisAlignment.spaceBetween,
      children: [
        Text('Edit Settings', style: TextStyle(fontSize: scale(22), fontWeight: FontWeight.bold)),
        IconButton(
          onPressed: () => Navigator.pop(context),
          icon: const Icon(Icons.close),
          padding: EdgeInsets.zero,
          constraints: const BoxConstraints(),
        ),
      ],
    );
  }

  Widget _buildNameField(double Function(double) scale) {
    return Container(
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
            decoration: const InputDecoration(isDense: true, border: InputBorder.none),
          ),
        ],
      ),
    );
  }

  Widget _buildTempSelector(double Function(double) scale) {
    return Row(
      children: [
        Text('Temperature: ', style: TextStyle(fontSize: scale(18))),
        IconButton(onPressed: () => setState(() => localTemp--), icon: Icon(Icons.remove, size: scale(18))),
        Text('$localTemp', style: const TextStyle(fontSize: 18, color: Colors.cyan, decoration: TextDecoration.underline, fontWeight: FontWeight.bold)),
        const Text('°C', style: TextStyle(fontSize: 18, fontWeight: FontWeight.bold)),
        IconButton(onPressed: () => setState(() => localTemp++), icon: Icon(Icons.add, size: scale(18))),
      ],
    );
  }

  Widget _buildFanSelector(double Function(double) scale) {
    return Row(
      children: [
        Text('Fan: ', style: TextStyle(fontSize: scale(18))),
        ...List.generate(3, (index) {
          bool isActive = index < localFanSpeed && localFanSpeed != 0;
          return IconButton(
            onPressed: () => setState(() => localFanSpeed = index + 1),
            icon: Icon(Icons.wind_power, color: isActive ? Colors.cyan : Colors.grey[300], size: scale(20)),
          );
        }),
        const Spacer(),
        const Text('Auto'),
        Checkbox(
          value: localFanSpeed == 0,
          onChanged: (val) => setState(() => localFanSpeed = (val == true) ? 0 : 2),
          activeColor: Colors.cyan,
        )
      ],
    );
  }

  Widget _buildToggle(String label, bool value, ValueChanged<bool> onChanged, double screenWidth) {
    return Row(
      children: [
        Text('$label (OFF/ON): ', style: const TextStyle(fontSize: 18)),
        Transform.scale(
          scale: (screenWidth / 375.0).clamp(0.8, 1.2),
          child: Switch(value: value, onChanged: onChanged, activeThumbColor: Colors.cyan),
        ),
      ],
    );
  }

  Widget _buildActions(double Function(double) scale) {
    return Row(
      mainAxisAlignment: MainAxisAlignment.end,
      children: [
        TextButton(onPressed: () => Navigator.pop(context), child: const Text('CANCEL', style: TextStyle(color: Colors.cyan))),
        const SizedBox(width: 16),
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
            _setAcData(widget.device.deviceID);
          },
          child: const Text('SEND', style: TextStyle(color: Colors.cyan)),
        ),
      ],
    );
  }

  Widget _buildListSelector(String label, String current, List<String> items, ValueChanged<String> onChanged, double Function(double) scale) {
    return Row(
      children: [
        Text('$label: ', style: TextStyle(fontSize: scale(18))),
        IconButton(
          onPressed: () {
            int i = items.indexOf(current);
            onChanged(items[(i - 1 + items.length) % items.length]);
          },
          icon: const Icon(Icons.chevron_left),
        ),
        Text(current, style: const TextStyle(fontSize: 18, color: Colors.cyan, decoration: TextDecoration.underline, fontWeight: FontWeight.bold)),
        IconButton(
          onPressed: () {
            int i = items.indexOf(current);
            onChanged(items[(i + 1) % items.length]);
          },
          icon: const Icon(Icons.chevron_right),
        ),
      ],
    );
  }
}
