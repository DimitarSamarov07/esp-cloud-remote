
enum Mode{
    Auto= "auto",
    Cool = "cool",
    Dry = "dry",
    Heat = "heat"
}
enum FanSpeed {
    Auto= "auto",
    Off= "off",
    Strong= "strong",
    Silent= "silent",
}
class AirConditionerStatus {
    IsOnline: boolean;
    IsPowered: boolean | null;
    Temperature: number | null;
    Mode: Mode | null;
    FanSpeed: number | FanSpeed | null ;
    constructor(isOnline: boolean ,isPowered :boolean | null, temperature: number | null ,public mode: Mode | null,fanspeed: number |FanSpeed| null,) {
        this.IsOnline = isOnline;
        this.IsPowered = isPowered;
        this.Temperature = temperature;
        this.Mode = this.mode;
        this.FanSpeed = fanspeed;
    }

}
export default AirConditionerStatus;