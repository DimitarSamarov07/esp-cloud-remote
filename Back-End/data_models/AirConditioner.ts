import {Moment} from 'moment'


class AirConditioner {
    DeviceID: string;
    Name: string | null;
    Brand: string | null;
    Model: string | null;
    LastSeen: Moment | null;

    constructor(deviceid: string, name: string | null, brand: string | null, model: string | null, lastseen: Moment | null,) {
        this.DeviceID = deviceid;
        this.Name = name;
        this.Brand = brand;
        this.Model = model;
        this.LastSeen = lastseen;
    }

}

export default AirConditioner;