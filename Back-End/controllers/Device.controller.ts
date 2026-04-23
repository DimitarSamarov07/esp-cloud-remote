import {Request, Response} from "express";
import {fetchAirConditionerStatus} from "../services/data/AirConditioner.ts";
import esp from "../services/MQTTService.ts";
import {register, verifyCredentials} from "../services/data/Device.ts";

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
    try{
        await register({
            isDevice: true,
            deviceID: deviceID,
            password: password,
            targetUserID: 1
        });
        return res.sendStatus(201);
    }
    catch (err){
        console.error("Webhook Error:", err);
        return res.status(500);
    }
}

/*
IMPORTANT: The EMQX broker only works with codes 200,204,4XX,5XX.
If the 4XX or 5XX codes are sent, the broker will presume that the request is
wrong and determine the result to be ignored which would practically deny the
authentication.
*/
export const validateCredentials = async (req: Request, res: Response) => {
    const { username, password, isDevice } = req.body;

    try {
        const isValid = await verifyCredentials(username, password, isDevice);
        return res.status(200).json({
            result: isValid ? "allow" : "deny"
        });

    } catch (err) {
        console.error("Webhook Error:", err);
        return res.status(200).json({ result: "deny" });
    }
};

export const changeWifi = async (req: Request, res: Response) => {
    let {deviceID, ssid, pass} = req.body;
    try {
        await esp.changeWifi(deviceID, ssid, pass);
        res.sendStatus(200);
    }
    catch (err) {
        console.error("Webhook Error:", err);
        return res.status(500);
    }
}
