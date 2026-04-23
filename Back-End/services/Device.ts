import {pool} from "./Pool.ts";
import DatabaseQueries from "../DatabaseQueries.ts";
import {DeviceStatus} from "../data_models/DeviceStatus.ts";
import bcrypt from 'bcrypt';


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

export async function register(res, data) {
    const { isDevice, password } = data;
    const hashedPass = await hashPassword(password, 14);

    try {
        if (isDevice) {
            // Needs: deviceID, password, targetUserID
            const { deviceID, targetUserID } = data;
            const now = new Date();
            const formatted = now.toISOString().replace('T', ' ').substring(0, 19);
            await pool.execute(
                `INSERT INTO Devices (ID, Name, DevicePassword, RegisteredAt)
                 VALUES (?, ?, ?, ? )`,
                [deviceID, `AC_Unit_${deviceID.substring(0, 10)}`, hashedPass, formatted]
            );

            await pool.execute(
                `INSERT INTO Users_devices (ID, UserID, DeviceID)
                 VALUES (UUID(), ?, ?)`,
                [targetUserID || 1, deviceID] // Defaults to User 1 if no ID provided
            );

            return res.status(200).json({ status: 'Device linked successfully', deviceID });

        } else {
            const { email } = data;
            let username = "mqtt_pc_" + Str_Random(16);

            await pool.execute(
                `INSERT INTO Users (Username, Email, Password, IsAdmin)
                 VALUES (?, ?, ?, 0)`,
                [username, email, hashedPass]
            );

            return res.status(200).json({ status: 'User created', username });
        }
    } catch (err) {
        console.error('Registration error:', err);
        return res.status(500).json({ error: 'Database error during registration.' });
    }
}

const hashPassword = async (password: string, saltRounds: number) => {
    return await bcrypt.hash(password, saltRounds);
};


/**
 * Generates a random string from a set collection of characters that can be used for username.
 * @param {number} length - The length of the string generated.
 * @returns {string} The randomized string that can later be concatenated with the mqtt_pc for the end username.
 */
function Str_Random(length) {
    let result = '';
    const characters = 'abcdefghijklmnopqrstuvwxyz0123456789';
    for (let i = 0; i < length; i++) {
        result += characters.charAt(Math.floor(Math.random() * characters.length));
    }
    return result;
}