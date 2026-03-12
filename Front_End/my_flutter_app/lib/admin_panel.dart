import 'package:flutter/material.dart';
import 'package:flutter_svg/flutter_svg.dart';

class AdminPage extends StatefulWidget {
  const AdminPage({super.key});

  @override
  State<AdminPage> createState() => _AdminPageState();
}

class _AdminPageState extends State<AdminPage> {
  // Global state for the settings
  List<bool> isSelected = [false, true]; // [Hot, Cold]
  int _temp = 25;
  bool light1 = true;
  int fanSpeed = 2; // 0-4

  void _showCustomDialog() {
    // Create local copies of the state to allow "Cancel" functionality
    int localTemp = _temp;
    List<bool> localIsSelected = List.from(isSelected);
    bool localLight1 = light1;
    int localFanSpeed = fanSpeed;

    showDialog(
      context: context,
      builder: (context) {
        return StatefulBuilder(
          builder: (context, setDialogState) {
            return Dialog(
              shape: RoundedRectangleBorder(
                borderRadius: BorderRadius.circular(20),
              ),
              child: Padding(
                padding: const EdgeInsets.all(24),
                child: Column(
                  mainAxisSize: MainAxisSize.min,
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    // Header with Close Button
                    Row(
                      mainAxisAlignment: MainAxisAlignment.spaceBetween,
                      children: [
                        const Text(
                          'Edit Settings',
                          style: TextStyle(
                            fontSize: 22,
                            fontWeight: FontWeight.bold,
                          ),
                        ),
                        IconButton(
                          onPressed: () => Navigator.pop(context),
                          icon: const Icon(Icons.close),
                          padding: EdgeInsets.zero,
                          constraints: const BoxConstraints(),
                        ),
                      ],
                    ),
                    const SizedBox(height: 20),

                    // Name Display
                    Container(
                      width: double.infinity,
                      padding: const EdgeInsets.symmetric(
                          horizontal: 16, vertical: 10),
                      decoration: BoxDecoration(
                        color: Colors.grey[100],
                        borderRadius: BorderRadius.circular(15),
                      ),
                      child: Column(
                        crossAxisAlignment: CrossAxisAlignment.start,
                        children: [
                          Text(
                            'Name*',
                            style: TextStyle(
                                color: Colors.grey[600], fontSize: 12),
                          ),
                          TextFormField(
                            decoration: const InputDecoration(
                              labelText: 'ESP Name',
                              prefixIcon: Icon(Icons.person_outline),
                              border: OutlineInputBorder(),
                            ),
                          ),
                        ],
                      ),
                    ),
                    const SizedBox(height: 20),

                    // Temperature Control
                    Row(
                      children: [
                        const Text('Temperature: ',
                            style: TextStyle(fontSize: 18)),
                        IconButton(
                          onPressed: () => setDialogState(() => localTemp--),
                          icon: const Icon(Icons.remove, size: 18),
                          padding: EdgeInsets.zero,
                          constraints: const BoxConstraints(),
                        ),
                        const SizedBox(width: 4),
                        Text(
                          '$localTemp',
                          style: const TextStyle(
                            fontSize: 18,
                            color: Colors.cyan,
                            decoration: TextDecoration.underline,
                            fontWeight: FontWeight.bold,
                          ),
                        ),
                        const Text('°C',
                            style: TextStyle(
                                fontSize: 18, fontWeight: FontWeight.bold)),
                        const SizedBox(width: 4),
                        IconButton(
                          onPressed: () => setDialogState(() => localTemp++),
                          icon: const Icon(Icons.add, size: 18),
                          padding: EdgeInsets.zero,
                          constraints: const BoxConstraints(),
                        ),
                      ],
                    ),
                    const SizedBox(height: 12),

                    // Mode Selection
                    Row(
                      children: [
                        const Text('Mode: ', style: TextStyle(fontSize: 18)),
                        const SizedBox(width: 8),
                        _buildModeButton('Cold', localIsSelected[1], () {
                          setDialogState(() {
                            localIsSelected = [false, true];
                          });
                        }),
                        const SizedBox(width: 8),
                        _buildModeButton('Hot', localIsSelected[0], () {
                          setDialogState(() {
                            localIsSelected = [true, false];
                          });
                        }),
                      ],
                    ),
                    const SizedBox(height: 12),

                    Row(
                      children: [
                        const Text('Fan: ', style: TextStyle(fontSize: 18)),
                        ...List.generate(4, (index) {
                          bool isActive = index < localFanSpeed;
                          return IconButton(
                            onPressed: () =>
                                setDialogState(() => localFanSpeed = index + 1),
                            icon: Icon(
                              Icons.wind_power,
                              color: isActive ? Colors.cyan : Colors.grey[300],
                            ),
                            padding: const EdgeInsets.symmetric(horizontal: 4),
                            constraints: const BoxConstraints(),
                          );
                        }),
                      ],
                    ),
                    const SizedBox(height: 12),

                    // Power Toggle
                    Row(
                      children: [
                        const Text('Power (ON/OFF): ',
                            style: TextStyle(fontSize: 18)),
                        Switch(
                          value: localLight1,
                          onChanged: (bool value) {
                            setDialogState(() {
                              localLight1 = value;
                            });
                          },
                          activeThumbColor: Colors.cyan,
                        ),
                      ],
                    ),
                    const SizedBox(height: 24),

                    // Footer Buttons
                    Row(
                      mainAxisAlignment: MainAxisAlignment.end,
                      children: [
                        TextButton(
                          onPressed: () => Navigator.pop(context),
                          child: const Text(
                            'CANCEL',
                            style: TextStyle(
                                color: Colors.cyan,
                                fontWeight: FontWeight.bold),
                          ),
                        ),
                        const SizedBox(width: 16),
                        TextButton(
                          onPressed: () {
                            // Update the main page state only on Save
                            setState(() {
                              _temp = localTemp;
                              isSelected = localIsSelected;
                              light1 = localLight1;
                              fanSpeed = localFanSpeed;
                            });
                            Navigator.pop(context);
                          },
                          child: const Text(
                            'SAVE',
                            style: TextStyle(
                                color: Colors.cyan,
                                fontWeight: FontWeight.bold),
                          ),
                        ),
                      ],
                    ),
                  ],
                ),
              ),
            );
          },
        );
      },
    );
  }

  Widget _buildModeButton(String label, bool active, VoidCallback onTap) {
    return GestureDetector(
      onTap: onTap,
      child: Container(
        padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 6),
        decoration: BoxDecoration(
          borderRadius: BorderRadius.circular(20),
          border: Border.all(color: active ? Colors.cyan : Colors.grey),
          color: active ? Colors.cyan.withValues(alpha: 0.1) : Colors.transparent,
        ),
        child: Text(
          label,
          style: TextStyle(
            color: active ? Colors.cyan : Colors.black,
            fontWeight: active ? FontWeight.bold : FontWeight.normal,
          ),
        ),
      ),
    );
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
          ),
        ),
        // title: const Text('Admin Panel'),
        actions: <Widget>[
          IconButton(
            icon: const Icon(
              Icons.menu,
              size: 30,
            ),
            tooltip: 'Menu',
            onPressed: () {},
          ),
        ],
        centerTitle: true,
        backgroundColor: Colors.white,
        elevation: 0.5,
        foregroundColor: Colors.black,
      ),
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            const SizedBox(height: 20),
            Container(
              padding: const EdgeInsets.all(24),
              decoration: BoxDecoration(
                color: Colors.white,
                borderRadius: BorderRadius.circular(20),
                boxShadow: [
                  BoxShadow(
                    color: Colors.black.withValues(alpha:0.05),
                    blurRadius: 15,
                    offset: const Offset(0, 5),
                  )
                ],
              ),
              child: Column(
                mainAxisSize: MainAxisSize.min,
                children: [
                  const SizedBox(height: 24),
                  ElevatedButton.icon(
                    onPressed: _showCustomDialog,
                    icon: const Icon(Icons.settings),
                    label: const Text('Edit Settings'),
                    style: ElevatedButton.styleFrom(
                      backgroundColor: Colors.cyan,
                      foregroundColor: Colors.white,
                      padding: const EdgeInsets.symmetric(
                          horizontal: 24, vertical: 12),
                      shape: RoundedRectangleBorder(
                        borderRadius: BorderRadius.circular(10),
                      ),
                    ),
                  ),
                ],
              ),
            ),
            const SizedBox(height: 20),
            /* Expanded(
              child: ListTile(
                  title: const Text('ESP Name'),
                  tileColor: Colors.grey,
                  onTap: (){},
                  leading: Icon(Icons.two_k_outlined),
                  trailing: Text(
                    'AC', // Fixed text temporarily
                    style: TextStyle(
                      fontWeight: FontWeight(800),
                      fontSize: 14,
                      color: Colors.grey,
                    ),
                  )
              ),
            ), */
            Expanded(child: _buildDeviceList())
          ],
        ),
      ),
    );
  }

  Widget _buildDeviceList() {
    return ListView.separated(
      itemCount: 3 +2, // bs
      itemBuilder: (BuildContext context, int index) {

        if (index == 0) {
          return ListTile(
            title: const Text(
              'Admin Panel',
              style: TextStyle(
                fontSize: 22,
                fontWeight: FontWeight(600)
              ),
            ),
            trailing: OutlinedButton(
              onPressed: (){},
              style: OutlinedButton.styleFrom(
                textStyle: TextStyle(color: Colors.grey),
                side: BorderSide(color: Colors.grey, width: 1),
                shape: RoundedRectangleBorder(
                  borderRadius: BorderRadius.circular(16)
                ),
              ),
              child: const Text('All ESPs')
            )
          );
        }

        if (index == 1) {
          return IgnorePointer(
            ignoring: true,
            child: ListTile(
              title: const Text(
                'ESP Name',
                style: TextStyle(
                  fontWeight: FontWeight(600),
                  fontSize: 14,
                  color: Colors.grey,
                ),
              ),
              tileColor: Colors.white,
              onTap: (){},
              leading: SizedBox(width: 24),
              trailing: Row(
                mainAxisSize: MainAxisSize.min,
                children: [
                  Text(
                      'Status', // Fixed text temporarily
                      style: TextStyle(
                        fontWeight: FontWeight(600),
                        fontSize: 14,
                        color: Colors.grey,
                      )),
                  SizedBox(width: 26),
                  Text(
                      'AC', // Fixed text temporarily
                      style: TextStyle(
                        fontWeight: FontWeight(600),
                        fontSize: 14,
                        color: Colors.grey,
                      ))
                ],
              ),
            ),
          );
        }

        return ListTile(
          title: const Text('ESP32'),
          tileColor: Colors.white,
          onTap: (){},
          leading: svgESP(),
          trailing: Row(
            mainAxisSize: MainAxisSize.min,
            children: [
              ESPstatusText(true),
              SizedBox(width: 26),
              ACstatusText(false)
            ],
          ),
        );
      },
      separatorBuilder: (BuildContext context, int index) => Divider(
        color: Colors.grey[300],
        height: 1,
        thickness: 1.6,
      ),
    );
  }

  Widget svgESP() {
    return SvgPicture.asset(
      "assets/esp.svg",
      width: 24,
      height: 24,
    );
  }

  Widget ESPstatusText(bool statusOn) {
    if (statusOn) {
      return Text(
          'Active', // Fixed text temporarily
          style: TextStyle(
            fontWeight: FontWeight(600),
            fontSize: 14,
            color: Colors.green,
          ));
    } else {
      return Text(
          'Inactive', // Fixed text temporarily
          style: TextStyle(
            fontWeight: FontWeight(600),
            fontSize: 14,
            color: Colors.red,
          ));
    }
  }

  Widget ACstatusText(bool statusOn) {
    if (statusOn) {
      return Text(
          'ON', // Fixed text temporarily
          style: TextStyle(
            fontWeight: FontWeight(600),
            fontSize: 14,
            color: Colors.green,
          ));
    } else {
      return Text(
          'OFF', // Fixed text temporarily
          style: TextStyle(
            fontWeight: FontWeight(600),
            fontSize: 14,
            color: Colors.red,
          ));
    }
  } // i know these are bs, let me cook
}
