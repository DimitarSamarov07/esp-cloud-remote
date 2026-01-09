import {Request, Response} from "express";
import {fetchAirConditionerStatus} from "../services/AirConditioner.ts";

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