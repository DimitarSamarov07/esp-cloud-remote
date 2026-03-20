import 'package:flutter/material.dart';
import 'package:my_flutter_app/connection.dart';
import 'package:my_flutter_app/device_setup.dart';
import 'login.dart';
import 'admin_panel.dart';
import 'wifi_dialog.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      debugShowCheckedModeBanner: false,
      theme: ThemeData(
        colorScheme: ColorScheme.fromSeed(seedColor: Colors.deepPurple),
        useMaterial3: true,
      ),
      home: AdminPage(),
      // home: WifiConnectionDialog(
      //     deviceName: "Goso's ESP",
      //     networks: ["OPTELA", "OPTELA 5G", "LordOfThePings5G","bjhfbksnfj","hfuioheiogfiojifjsi","ijfj"],
      //   ) ,
    );
  }
}
