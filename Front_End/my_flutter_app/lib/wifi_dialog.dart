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
              
              Container(
                constraints: BoxConstraints(maxHeight: scale(200)),
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
              
              IconButton(
                onPressed: () {},
                icon: Icon(Icons.refresh, size: scale(24), color: Colors.grey.shade400),
              ),
              
              SizedBox(height: scale(12)),
              
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
                      : () async {
                          final password = await showDialog<String>(
                            context: context,
                            builder: (context) => WifiPasswordDialog(networkName: selectedNetwork!),
                          );
                          if (password != null && mounted) {
                            Navigator.pop(context, {'network': selectedNetwork, 'password': password});
                          }
                        },
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

class WifiPasswordDialog extends StatefulWidget {
  final String networkName;
  const WifiPasswordDialog({super.key, required this.networkName});

  @override
  State<WifiPasswordDialog> createState() => _WifiPasswordDialogState();
}

class _WifiPasswordDialogState extends State<WifiPasswordDialog> {
  final TextEditingController _passwordController = TextEditingController();

  @override
  void dispose() {
    _passwordController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final double screenWidth = MediaQuery.of(context).size.width;
    double scale(double size) => size * (screenWidth / 375.0).clamp(0.7, 1.15);

    return Dialog(
      shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(20)),
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
              'Enter the network security key',
              textAlign: TextAlign.center,
              style: TextStyle(
                fontSize: scale(13),
                color: Colors.black54,
              ),
            ),
            SizedBox(height: scale(20)),
            
            Container(
              padding: EdgeInsets.symmetric(horizontal: scale(16), vertical: scale(8)),
              decoration: BoxDecoration(
                color: Colors.grey.shade100,
                borderRadius: BorderRadius.circular(15),
              ),
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  Text(
                    'Password*',
                    style: TextStyle(
                      color: Colors.grey.shade600,
                      fontSize: scale(12),
                    ),
                  ),
                  TextField(
                    controller: _passwordController,
                    obscureText: true,
                    autofocus: true,
                    decoration: const InputDecoration(
                      isDense: true,
                      border: InputBorder.none,
                      contentPadding: EdgeInsets.symmetric(vertical: 4),
                    ),
                    style: const TextStyle(
                      fontSize: 16,
                      fontWeight: FontWeight.w500,
                    ),
                  ),
                ],
              ),
            ),
            
            SizedBox(height: scale(24)),
            
            Wrap(
              children: [
                TextButton(
                  onPressed: () => Navigator.pop(context),
                  child: Text(
                    'BACK',
                    style: TextStyle(
                      color: Colors.cyan,
                      fontWeight: FontWeight.bold,
                      fontSize: scale(14),
                    ),
                  ),
                ),
                SizedBox(width: scale(16)),
                TextButton(
                  onPressed: () => Navigator.pop(context, _passwordController.text),
                  child: Text(
                    'FINISH',
                    style: TextStyle(
                      color: Colors.cyan,
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
    );
  }
}
