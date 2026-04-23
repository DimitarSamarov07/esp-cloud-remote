import {Moment} from 'moment'

export enum Mode {
    Auto = "auto",
    Cool = "cool",
    Dry = "dry",
    Heat = "heat"
}

export enum FanSpeed {
    Auto = "auto",
    Quiet = "quiet"
}

export class AirConditioner {
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

export class AirConditionerSet {
    Brand: string;
    Power: number;
    Temperature: number;
    Mode: Mode | null;
    FanSpeed: number | FanSpeed;
    Swing: boolean;

    constructor(brand: string,power: number, mode: Mode, temp: number, fanSpeed: number | FanSpeed, swing: boolean) {
        this.Power = power;
        this.Brand = brand;
        this.Temperature = temp;
        this.Mode = mode;
        this.FanSpeed = fanSpeed;
        this.Swing = swing;
    }
}



export class AirConditionerStatus{
    IsOnline: boolean | null;
    IsPowered: boolean | null;
    Temperature: number | null;
    TargetTemperature: number | null;
    Mode: Mode | null;
    FanSpeed: number | FanSpeed | null;
    Swing: boolean | null;
    LastUpdated: Moment | null;

    constructor(isOnline: boolean | null, isPowered: boolean | null, temperature: number | null, targetTemperature: number | null, mode: Mode | null, fanspeed: number | FanSpeed | null, swing: boolean | null, lastUpdated: Moment | null) {
        this.IsOnline = isOnline;
        this.IsPowered = isPowered;
        this.Temperature = temperature;
        this.TargetTemperature = targetTemperature;
        this.Mode = mode;
        this.FanSpeed = fanspeed;
        this.Swing = swing;
        this.LastUpdated = lastUpdated;
    }
}