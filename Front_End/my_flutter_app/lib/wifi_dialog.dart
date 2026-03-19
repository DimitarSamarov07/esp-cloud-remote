import 'package:flutter/material.dart';

class WifiConnectionDialog extends StatefulWidget {
  final List<String> networks;
  final String deviceName;

  const WifiConnectionDialog({
    super.key,
    required this.networks,
    required this.deviceName,
  });

  @override
  State<WifiConnectionDialog> createState() => _WifiConnectionDialogState();
}

class _WifiConnectionDialogState extends State<WifiConnectionDialog> {
  String? selectedNetwork;

  @override
  Widget build(BuildContext context) {
    final double screenWidth = MediaQuery.of(context).size.width;
    // Normalized scale factor for responsive UI
    double scale(double size) => size * (screenWidth / 375.0).clamp(0.7, 1.15);

    return Dialog(
      shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(20)),
      child: SingleChildScrollView(
        physics: const BouncingScrollPhysics(),
        child: Container(
          padding: EdgeInsets.all(scale(24)),
          constraints: BoxConstraints(maxWidth: scale(320)),
          child: Column(
            mainAxisSize: MainAxisSize.min,
            crossAxisAlignment: CrossAxisAlignment.center,
            children: [
              Text(
                'WiFi Connection',
                style: TextStyle(
                  fontSize: scale(20),
                  fontWeight: FontWeight.bold,
                ),
              ),
              SizedBox(height: scale(4)),
              Text(
                'Choose a WiFi to connect ${widget.deviceName}',
                textAlign: TextAlign.center,
                style: TextStyle(
                  fontSize: scale(13),
                  color: Colors.black54,
                ),
              ),
              SizedBox(height: scale(15)),
              
              // Network List Container - Nested Scrollable
              Container(
                constraints: BoxConstraints(maxHeight: scale(200)), // Limit height of the box
                decoration: BoxDecoration(
                  border: Border.all(color: Colors.grey.shade300),
                  borderRadius: BorderRadius.circular(12),
                ),
                child: ClipRRect(
                  borderRadius: BorderRadius.circular(12),
                  child: SingleChildScrollView(
                    physics: const BouncingScrollPhysics(),
                    child: Column(
                      children: widget.networks.map((network) {
                        bool isSelected = selectedNetwork == network;
                        return InkWell(
                          onTap: () {
                            setState(() {
                              selectedNetwork = network;
                            });
                          },
                          child: Container(
                            padding: EdgeInsets.symmetric(
                              horizontal: scale(12),
                              vertical: scale(12),
                            ),
                            decoration: BoxDecoration(
                              color: isSelected ? Colors.grey.shade100 : Colors.transparent,
                              border: widget.networks.last == network
                                  ? null
                                  : Border(bottom: BorderSide(color: Colors.grey.shade200)),
                            ),
                            child: Row(
                              children: [
                                Icon(Icons.wifi, size: scale(20), color: Colors.black87),
                                SizedBox(width: scale(12)),
                                Expanded(
                                  child: Text(
                                    network,
                                    style: TextStyle(
                                      fontSize: scale(14),
                                      fontWeight: isSelected ? FontWeight.bold : FontWeight.normal,
                                      color: Colors.black87,
                                    ),
                                  ),
                                ),
                              ],
                            ),
                          ),
                        );
                      }).toList(),
                    ),
                  ),
                ),
              ),
              
              SizedBox(height: scale(16)),
              
              // Refresh Icon (Centered)
              IconButton(
                onPressed: () {
                  // Logic to refresh networks
                },
                icon: Icon(Icons.refresh, size: scale(24), color: Colors.grey.shade400),
              ),
              
              SizedBox(height: scale(12)),
              
              // Action Buttons
              Wrap(
                children: [
                  TextButton(
                    onPressed: () => Navigator.pop(context),
                    child: Text(
                      'CANCEL',
                      style: TextStyle(
                        color: Colors.cyan,
                        fontWeight: FontWeight.bold,
                        fontSize: scale(14),
                      ),
                    ),
                  ),
                  SizedBox(width: scale(16)),
                  TextButton(
                    onPressed: selectedNetwork == null
                      ? null
                      : () => Navigator.pop(context, selectedNetwork),
                    child: Text(
                      'CONNECT',
                      style: TextStyle(
                        color: selectedNetwork == null ? Colors.grey : Colors.cyan,
                        fontWeight: FontWeight.bold,
                        fontSize: scale(14),
                      ),
                    ),
                  ),
                ],
              ),
            ],
          ),
        ),
      ),
    );
  }
}
