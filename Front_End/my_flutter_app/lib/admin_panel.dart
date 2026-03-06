import 'package:flutter/material.dart';

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
        title: const Text('Admin Panel'),
        centerTitle: true,
        backgroundColor: Colors.white,
        elevation: 0.5,
        foregroundColor: Colors.black,
      ),
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
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
          ],
        ),
      ),
    );
  }
}
