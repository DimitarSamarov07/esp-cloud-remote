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