import {Request, Response} from 'express';
import {pool} from "../Pool.ts";
import DatabaseQueries from "../DatabaseQueries.ts";
import {User} from "../data_models/User.ts";
import AirConditioner from "../data_models/AirConditioner.ts";
import Device from "../data_models/Device.ts";
import AirConditionerStatus from "../data_models/AirConditionerStatus.ts";

export const getAirConditionerByID = async (req: Request, res: Response) => {
    const {deviceID} = req.params;
    if (!deviceID) {
        return res.status(406).send('Please provide a valid device ID!');
    }
    const [rows] = await pool.query(DatabaseQueries.SELECT_AIR_CONDITIONER_BY_ID, [deviceID]);
    if (!rows) {
        return res.status(404).send('No Air conditioner found!.');
    }
    return res.status(200).send(rows);
}

//     in_username,
//     in_email,
//     in_password,
//     in_device_id,
//     in_device_name,
//     in_location,
//     in_ac_name,
//     in_ac_brand,
//     in_ac_model,
//     in_current_temp,
//     in_target_temp,
//     in_fan_speed,
//     in_mode,
//     in_swing,
// )
interface CreateAirConditionerRequest {
    user: User;
    device: Device;
    airConditioner: AirConditioner;
    status: AirConditionerStatus;
}
interface CreateAirConditionerUpdateRequest {
    ID: string,
    airConditioner: AirConditioner;
    status: AirConditionerStatus;
}

export const createAirConditioner = async (req: Request, res: Response) => {
    const {user, device, airConditioner, status} = req.body as CreateAirConditionerRequest;

    if (!user || !device || !airConditioner || !status) {
        return res.status(406).send('Please provide all the necessary arguments!');
    }

    try {
        const params = [
            // User data
            user.Username,
            user.Email,
            user.Password,
            // Device data
            device.deviceID,
            device.deviceName,
            device.location,
            // AirConditioner data
            airConditioner.Name,
            airConditioner.Brand,
            airConditioner.Model,
            airConditioner.LastSeen,
            // Status data
            status.IsOnline,
            status.IsPowered,
            status.Temperature,
            status.TargetTemperature,
            status.FanSpeed,
            status.Mode,
            status.Swing,
            status.LastUpdated
        ]
        const [rows] = await pool.query(DatabaseQueries.CREATE_AIR_CONDITIONER, params );

        if (!rows) {
            return res.status(422).send('The AC was NOT created successfully');
        }

        return res.status(201).send({
            message: 'The AC was created successfully',
            data: rows
        });
    } catch (error) {
        return res.status(500).send('Error in creating ac: ' + error);
    }
}

//TODO: Add parameters so the user can change whatever they want
//NOTE: Use coalesce
export const updateAirConditionerByID = async (req: Request, res: Response) => {
    try {
        const { ID, airConditioner, status } = req.body as CreateAirConditionerUpdateRequest;
        console.log(ID)
        console.log(airConditioner)
        console.log(status)
        if (!ID) {
            return res.status(400).json({ error: 'AirConditioner ID is required' });
        }

        const params = [
            airConditioner.ID,
            airConditioner.Name ?? null,
            airConditioner.Brand ?? null,
            airConditioner.Model ?? null,
            status?.IsPowered ?? null,
            status?.Temperature ?? null,
            status?.TargetTemperature ?? null,
            status?.FanSpeed ?? null,
            status?.Mode ?? null,
            status?.Swing ?? null
        ];

        await pool.query(DatabaseQueries.UPDATE_AIR_CONDITIONER_BY_ID, params);

        return res.status(200).json({ message: 'Air conditioner updated successfully.' });
    } catch (err) {
        console.error('Error updating AC:', err);
        return res.status(500).json({ error: 'Internal server error' });
    }
}
export const deleteAirConditionerByID = async (req: Request, res: Response) => {
    const {deviceID} = req.params;
    if (!deviceID) {
        return res.status(406).send('Please provide a valid device ID!');
    }
    const [rows] = await pool.query(DatabaseQueries.DELETE_AIR_CONDITIONER_BY_ID, [deviceID]);
    if (!rows) {
        return res.status(404).send('No Air conditioner found!.');
    }
    return res.status(200).send(rows);
}