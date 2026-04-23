import {FanSpeed, Mode} from "./AirConditionerStatus";

export default class AirConditionerSet {
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
