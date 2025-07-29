import {Moment} from "moment";

export enum Mode {
    Auto = "auto",
    Cool = "cool",
    Dry = "dry",
    Heat = "heat"
}

export enum FanSpeed {
    Auto = "auto",
    Off = "off",
    Strong = "strong",
    Silent = "silent",
}

class AirConditionerStatus{
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

export default AirConditionerStatus;