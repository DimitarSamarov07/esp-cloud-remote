import {Request, Response} from 'express';
import {pool} from "../Pool.ts";
import DatabaseQueries from "../DatabaseQueries.ts";
import {User} from "../data_models/User.ts";
import AirConditioner from "../data_models/AirConditioner.ts";
import Device from "../data_models/Device.ts";
import AirConditionerStatus from "../data_models/AirConditionerStatus.ts";

interface CreateAirConditionerRequest {
    user: User;
    device: Device;
    airConditioner: AirConditioner;
    status: AirConditionerStatus;
}
interface CreateAirConditionerUpdateRequest {
    airConditioner: AirConditioner;
    status: AirConditionerStatus;
}


/**
 * Handles the request to retrieve an air conditioner by its unique device ID.
 *
 * This function extracts the device ID from the request parameters and queries
 * the database to find an air conditioner matching the provided ID. Appropriate
 * HTTP status codes and responses are returned based on the presence or absence
 * of the device ID and the query result.
 *
 * @param {Request} req - The incoming HTTP request object containing the device ID in its parameters.
 * @param {Response} res - The HTTP response object used to send back the query results or error messages.
 * @returns {Promise<void>} A promise representing the asynchronous operation of querying the database and sending the HTTP response.
 *
 * @throws {Error} If the database query fails for any reason.
 */
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


/**
 * Asynchronous function to create a new air conditioner entry in the database.
 * It processes the incoming request to extract necessary information,
 * performs data validation, executes the database query for creating the air conditioner,
 * and sends appropriate responses based on the operation result.
 *
 * @param {Request} req - Express request object containing the information for the air conditioner to be created.
 *                         Expected to include details such as user information, device information,
 *                         air conditioner details, and status in the request body.
 * @param {Response} res - Express response object used to send back the appropriate HTTP response.
 *
 * @throws {Error} If there is an issue while interacting with the database or any other processing error occurs.
 *
 * Responses:
 *  - Returns HTTP 406 if required arguments (user, device, airConditioner, status) are missing in the request body.
 *  - Returns HTTP 201 if the air conditioner is successfully created, along with the response data.
 *  - Returns HTTP 422 if the air conditioner could not be created.
 *  - Returns HTTP 500 in case of a server error during processing.
 */
export const createAirConditioner = async (req: Request, res: Response) => {
    const {user, device, airConditioner, status} = req.body as CreateAirConditionerRequest;

    if (!user || !device || !airConditioner || !status) {
        return res.status(406).send('Please provide all the necessary arguments!');
    }

    try {
        const params = [
            user.Username,
            user.Email,
            user.Password,
            device.deviceID,
            device.deviceName,
            device.location,
            airConditioner.Name,
            airConditioner.Brand,
            airConditioner.Model,
            airConditioner.LastSeen,
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

/**
 * Updates the details of an air conditioner by its ID.
 *
 * This function handles the HTTP request to update an air conditioner's information in the database.
 * It extracts the `deviceId` from the request parameters and the air conditioner details and status
 * from the request body. The updated information is then stored in the database.
 *
 * @param {Request} req - The HTTP request object, which should include the `deviceId` in the route
 * parameters and the updated air conditioner details and status in the request body.
 * @param {Response} res - The HTTP response object for sending back success or error messages.
 * @throws Will respond with a 406 status code if the `deviceId` is missing in the request parameters.
 * @throws Will respond with a 500 status code if an internal server error occurs.
 */
export const updateAirConditionerByID = async (req: Request, res: Response) => {
    try {
        const {deviceId} = req.params;
        const {airConditioner, status} = req.body as CreateAirConditionerUpdateRequest;
        if (!deviceId) {
            return res.status(406).json({error: 'AirConditioner ID is required'});
        }


        const params = [
            deviceId,
            airConditioner.Name ?? null,
            airConditioner.Brand ?? null,
            airConditioner.Model ?? null,
            status?.IsOnline ?? null,
            status?.IsPowered ?? null,
            status?.Temperature ?? null,
            status?.TargetTemperature ?? null,
            status?.FanSpeed ?? null,
            status?.Mode ?? null,
            status?.Swing ?? null,
        ];
        await pool.query(DatabaseQueries.UPDATE_AIR_CONDITIONER_BY_ID, params);

        return res.status(200).json({ message: 'Air conditioner updated successfully.' });
    } catch (err) {
        console.error('Error updating AC:', err);
        return res.status(500).json({ error: 'Internal server error' });
    }
}
/**
 * Asynchronous function to delete an air conditioner by its unique device ID.
 * Validates the presence of the device ID, performs a database query to execute the deletion,
 * and sends the appropriate HTTP response based on the operation's result.
 *
 * @function deleteAirConditionerByID
 * @async
 * @param {Request} req - The HTTP request object containing the device ID in `req.params`.
 * @param {Response} res - The HTTP response object to send the status or result.
 * @throws Will send a 406 HTTP status if the `deviceId` is not provided.
 * @throws Will send a 200 HTTP status if the air conditioner is deleted successfully.
 * @throws Will send a 410 HTTP status if the specified air conditioner device ID does not exist.
 */
export const deleteAirConditionerByID = async (req: Request, res: Response) => {
    const {deviceId} = req.params;

    if (!deviceId) {
        return res.status(406).send('Please provide a valid device ID!');
    }
    const [rows] = await pool.query(DatabaseQueries.DELETE_AIR_CONDITIONER_BY_ID, [deviceId]);
    // @ts-ignore
    const result = rows[0][0]; //First result set, first row

    if (result.status.toString().toLowerCase() == 'deleted') {
        return res.sendStatus(200);
    } else if (result.status.toString().toLowerCase() == 'notfound') {
        return res.sendStatus(410);
    }
}