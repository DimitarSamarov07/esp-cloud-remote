import {pool} from "../Pool.ts";
import {DeviceStatus} from "../../data_models/Device.ts";
import bcrypt from 'bcrypt';
import {DeviceQueries} from "./queries/Device.sql.ts";
import {UserQueries} from "./queries/User.sql.ts";
import {AirConditionerQueries} from "./queries/AirConditioner.sql.ts";


export const fetchDevices = async () => {

    const [rows] = await pool.query(AirConditionerQueries.SELECT_DEVICE_AND_ACS);
    //@ts-ignore
    if (rows.length === 0) {
        throw new Error('No status found for the given device ID');
    }
    return rows as DeviceStatus[];
};

export interface RegisterData {
    isDevice?: boolean;
    password?: string;
    deviceID?: string;
    targetUserID?: number;
    email?: string;
}

export const register = async (data: RegisterData) => {
    const { isDevice, password, deviceID, targetUserID, email } = data;

    if (!password) {
        throw new Error("Password is required.");
    }

    const hashedPass = await hashPassword(password, 14);

    if (isDevice) {
        if (!deviceID) {
            throw new Error("Device ID is required for device registration.");
        }

        const now = new Date();
        const formatted = now.toISOString().replace('T', ' ').substring(0, 19);
        const deviceName = `AC_Unit_${deviceID.substring(0, 10)}`;

        // Note: Change pool.execute to pool.query if using the official 'mariadb' package
        await pool.execute(
            DeviceQueries.ADD_DEVICE,
            [deviceID, deviceName, hashedPass, formatted]
        );

        await pool.execute(
            DeviceQueries.CONNECT_USER_TO_DEVICE,
            [targetUserID || 1, deviceID] // Defaults to User 1
        );

        return { type: 'device', deviceID };

    } else {
        if (!email) {
            throw new Error("Email is required for user registration.");
        }

        const username = "mqtt_pc_" + generate_str(16);

        await pool.execute(
            UserQueries.ADD_USER,
            [username, email, hashedPass]
        );

        return { type: 'user', username };
    }
};
export const verifyCredentials = async (
    username: string,
    password: string,
    isDevice: string
): Promise<boolean> => {
    let dbPassword;

    if (isDevice == "true") {
        const [rows] = await pool.query(
            `SELECT DevicePassword FROM Devices WHERE ID = ?`,
            [username]
        );
        // @ts-ignore
        if (rows.length > 0) { // @ts-ignore
            dbPassword = rows[0].DevicePassword;
        }
    } else {
        const [rows] = await pool.query(
            `SELECT Password FROM Users WHERE Username = ?`,
            [username]
        );
        // @ts-ignore
        if (rows.length > 0) {
            // @ts-ignore
            dbPassword = rows[0].Password;
        }
    }

    if (!dbPassword) {
        console.log("No password found for user:", username);
        return false;
    }

    const match = await bcrypt.compare(password, dbPassword);
    return match;
};

const hashPassword = async (password: string, saltRounds: number) => {
    return await bcrypt.hash(password, saltRounds);
};


/**
 * Generates a random string from a set collection of characters that can be used for username.
 * @param {number} length - The length of the string generated.
 * @returns {string} The randomized string that can later be concatenated with the mqtt_pc for the end username.
 */
function generate_str(length: number): string {
    let result = '';
    const characters = 'abcdefghijklmnopqrstuvwxyz0123456789';
    for (let i = 0; i < length; i++) {
        result += characters.charAt(Math.floor(Math.random() * characters.length));
    }
    return result;
}