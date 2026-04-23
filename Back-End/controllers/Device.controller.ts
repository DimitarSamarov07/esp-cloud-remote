import {Request, Response} from "express";
import {fetchAirConditionerStatus} from "../services/AirConditioner.ts";
import esp from "../services/MQTTService.ts";
import {register} from "../services/Device.ts";

export const getDeviceByID = async (req: Request, res: Response) => {
    try {
        const {deviceID} = req.params;
        //@ts-ignore
        const mqttClient: any = req.mqttClient;

        const status = await fetchAirConditionerStatus(deviceID, mqttClient);
        return res.status(200).json(status);
    } catch (err: any) {
        return res.status(500).json({error: err.message || 'Internal server error'});
    }
};

export const registerDevice = async (req: Request, res: Response) => {
    const { deviceID, password } = req.body;
    // Validation Guard
    if (!deviceID || !password) {
        return res.status(400).json({
            error: "Missing fields",
            received: { deviceID: !!deviceID, password: !!password }
        });
    }

    register(res, {
        isDevice: true,
        deviceID: deviceID,
        password: password,
        targetUserID: 1
    });
}

export const changeWifi = async (req: Request, res: Response) => {
    let {deviceID, ssid, pass} = req.body;

    await esp.changeWifi(deviceID, ssid, pass);
    res.sendStatus(200);
}
