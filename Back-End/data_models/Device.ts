class Device {
    deviceID: string;
    deviceName: string;
    location: string;

    constructor(deviceID: string, deviceName: string, location: string) {
        this.deviceID = deviceID;
        this.deviceName = deviceName;
        this.location = location;
    }
}

export default Device;