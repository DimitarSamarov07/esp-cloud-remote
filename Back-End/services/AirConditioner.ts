import DatabaseQueries from "../DatabaseQueries.ts";
import {pool} from "./Pool.ts";

export const fetchAirConditionerStatus = async (deviceID: string, mqttClient: any) => {
    if (!deviceID) {
        throw new Error('Please provide a valid device ID!');
    }

    if (!mqttClient) {
        throw new Error('MQTT client not available');
    }

    const status = mqttClient.getStatusByDeviceId(deviceID);
    if (status) return status;

    const [rows] = await pool.query(DatabaseQueries.SELECT_AIR_CONDITIONER_BY_ID, [deviceID]);
    //@ts-ignore
    if (rows.length === 0) {
        throw new Error('No status found for the given device ID');
    }
    //@ts-ignore
    return rows[0];
};