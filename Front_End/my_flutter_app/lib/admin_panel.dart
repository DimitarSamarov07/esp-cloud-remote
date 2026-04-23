import 'package:flutter/material.dart';
import 'package:flutter_svg/flutter_svg.dart';
import 'package:my_flutter_app/connection.dart';
import 'models.dart';
import 'settings_dialog.dart';

class AdminPage extends StatefulWidget {
  const AdminPage({super.key});

  @override
  State<AdminPage> createState() => _AdminPageState();
}

class _AdminPageState extends State<AdminPage> {
  final List<DeviceData> _devices = [
    DeviceData(name: 'ESP32 Kitchen', deviceID: 'mqtt_pc_758fb8'),
    DeviceData(name: 'ESP32 Living Room', deviceID: 'B_mqtt_pc_758fb8'),
    DeviceData(name: 'ESP32 Bedroom', deviceID: 'C_mqtt_pc_758fb8'),
  ];

  Future<void> _showSettings(int index) async {
    final result = await showDialog<Map<String, dynamic>>(
      context: context,
      builder: (context) => EspSettingsDialog(device: _devices[index]),
    );

    if (result != null && mounted) {
      setState(() {
        _devices[index]
          ..name = result['name']
          ..temp = result['temp']
          ..mode = result['mode']
          ..power = result['power']
          ..fanSpeed = result['fanSpeed']
          ..swing = result['swing'];
      });
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: Colors.white,
      appBar: _buildAppBar(),
      body: _buildDeviceList(),
    );
  }

  PreferredSizeWidget _buildAppBar() {
    return AppBar(
      leadingWidth: 140,
      leading: Padding(
        padding: const EdgeInsets.all(8.0),
        child: Image.asset(
          'assets/espcr.png',
          errorBuilder: (_, __, ___) => const Icon(Icons.broken_image),
        ),
      ),
      actions: [
        Padding(
          padding: const EdgeInsets.all(8.0),
          child: OutlinedButton(
            onPressed: () => Navigator.push(context, MaterialPageRoute(builder: (_) => const ConnectionPage())),
            style: OutlinedButton.styleFrom(
              side: const BorderSide(color: Colors.blue),
              shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(40)),
            ),
            child: const Text('Add ESP', style: TextStyle(color: Colors.blue)),
          ),
        )
      ],
      backgroundColor: Colors.white,
      elevation: 0,
    );
  }

  Widget _buildDeviceList() {
    return ListView.separated(
      padding: const EdgeInsets.only(bottom: 16),
      itemCount: _devices.length + 4,
      separatorBuilder: (_, index) => index == 0 ? const SizedBox.shrink() : Divider(color: Colors.grey[300], thickness: 1.6),
      itemBuilder: (context, index) {
        if (index == 0) return _buildWelcome();
        if (index == 1) return _buildStatusOverview();
        if (index == 2) return _buildPanelHeader();
        if (index == 3) return _buildListHeader();

        final deviceIndex = index - 4;
        final device = _devices[deviceIndex];

        return ListTile(
          onTap: () => _showSettings(deviceIndex),
          leading: SvgPicture.asset("assets/esp.svg", width: 24, placeholderBuilder: (_) => const Icon(Icons.developer_board, color: Colors.cyan)),
          title: Text(device.name),
          trailing: Row(
            mainAxisSize: MainAxisSize.min,
            children: [
              _statusText('Active', Colors.green),
              const SizedBox(width: 26),
              _statusText(device.power ? 'ON' : 'OFF', device.power ? Colors.green : Colors.red),
            ],
          ),
        );
      },
    );
  }

  Widget _buildWelcome() {
    return const Padding(
      padding: EdgeInsets.symmetric(vertical: 20),
      child: Center(
        child: Text.rich(TextSpan(
          text: 'Welcome back, ',
          style: TextStyle(fontSize: 26, fontWeight: FontWeight.bold, fontStyle: FontStyle.italic),
          children: [TextSpan(text: 'Pesho!', style: TextStyle(color: Colors.blue))],
        )),
      ),
    );
  }

  Widget _buildStatusOverview() {
    final active = _devices.where((d) => d.power).length;
    return Padding(
      padding: const EdgeInsets.symmetric(horizontal: 16),
      child: Row(
        children: [
          Expanded(child: _StatusCard(title: 'Active ACs', value: '$active', icon: Icons.toys_outlined)),
          const SizedBox(width: 16),
          Expanded(child: _StatusCard(title: 'Total ESPs', value: '${_devices.length}', icon: Icons.memory_outlined)),
        ],
      ),
    );
  }

  Widget _buildPanelHeader() {
    return Padding(
      padding: const EdgeInsets.fromLTRB(16, 16, 16, 8),
      child: Row(
        mainAxisAlignment: MainAxisAlignment.spaceBetween,
        children: [
          const Text('Admin Panel', style: TextStyle(fontSize: 22, fontWeight: FontWeight.bold)),
          OutlinedButton(
            onPressed: () {},
            style: OutlinedButton.styleFrom(shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(20))),
            child: const Text('All ESPs', style: TextStyle(color: Colors.grey)),
          ),
        ],
      ),
    );
  }

  Widget _buildListHeader() {
    return const IgnorePointer(
      child: ListTile(
        leading: SizedBox(width: 24),
        title: Text('ESP Name', style: TextStyle(fontSize: 14, color: Colors.grey, fontWeight: FontWeight.w600)),
        trailing: Row(
          mainAxisSize: MainAxisSize.min,
          children: [
            Text('Status', style: TextStyle(fontSize: 14, color: Colors.grey, fontWeight: FontWeight.w600)),
            SizedBox(width: 26),
            Text('AC', style: TextStyle(fontSize: 14, color: Colors.grey, fontWeight: FontWeight.w600)),
          ],
        ),
      ),
    );
  }

  Widget _statusText(String text, Color color) {
    return Text(text, style: TextStyle(fontWeight: FontWeight.w600, fontSize: 14, color: color));
  }
}

class _StatusCard extends StatelessWidget {
  final String title, value;
  final IconData icon;

  const _StatusCard({required this.title, required this.value, required this.icon});

  @override
  Widget build(BuildContext context) {
    return Container(
      padding: const EdgeInsets.all(16),
      decoration: BoxDecoration(
        color: const Color(0xFFF2F2F2),
        borderRadius: BorderRadius.circular(16),
        border: Border.all(color: Colors.grey.shade300, width: 1.5),
      ),
      child: Column(
        children: [
          Text(title, style: TextStyle(color: Colors.grey.shade600, fontWeight: FontWeight.w600, fontSize: 15)),
          const SizedBox(height: 12),
          Row(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              Icon(icon, size: 38, color: Colors.grey.shade600),
              const SizedBox(width: 8),
              Text(value, style: const TextStyle(fontSize: 32, fontWeight: FontWeight.bold)),
            ],
          ),
        ],
      ),
    );
  }
}
