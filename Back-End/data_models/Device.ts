export class Device {
    deviceID: string;
    deviceName: string;
    location: string;

    constructor(deviceID: string, deviceName: string, location: string) {
        this.deviceID = deviceID;
        this.deviceName = deviceName;
        this.location = location;
    }
}
export class DeviceStatus {
    Name: string;
    IsDeviceOnline: boolean;
    IsACOnline: boolean;

    constructor(name: string, isDeviceOnline: boolean, isACOnline: boolean) {
        this.Name = name;
        this.IsACOnline = isACOnline;
        this.IsDeviceOnline = isDeviceOnline;
    }
}

