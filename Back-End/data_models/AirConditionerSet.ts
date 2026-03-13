import {FanSpeed, Mode} from "./AirConditionerStatus";

export default class AirConditionerSet {
    Power: boolean;
    Temperature: number;
    Mode: Mode | null;
    FanSpeed: number | FanSpeed;
    Swing: boolean;

    constructor(power: boolean, mode: Mode, temp: number, fanSpeed: number | FanSpeed, swing: boolean) {
        this.Power = power;
        this.Temperature = temp;
        this.Mode = mode;
        this.FanSpeed = fanSpeed;
        this.Swing = swing;
    }
}
