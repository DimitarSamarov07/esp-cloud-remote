import {pool} from "./Pool.ts";
import DatabaseQueries from "../DatabaseQueries.ts";
import {DeviceStatus} from "../data_models/DeviceStatus.ts";

export const fetchDevices = async () => {

    const [rows] = await pool.query(DatabaseQueries.SELECT_DEVICE_AND_ACS);
    //@ts-ignore
    if (rows.length === 0) {
        throw new Error('No status found for the given device ID');
    }
    console.log(rows);
    //@ts-ignore
    return rows as DeviceStatus[];
};
