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
    this.swing = false,
  });
}
