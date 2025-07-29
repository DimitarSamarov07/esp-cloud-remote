/*`
 * @brief for now we are presuming that the DB is MySQL
 */
//deviceId: string, isOnline: boolean, isPowered: boolean | null,
// temperature: number | null, mode: Mode | null,
// fanspeed: number | FanSpeed | null, swing: boolean | null,
// lastseen: Moment | null,
class DatabaseQueries {

    /**
     * SQL query string to retrieve information about a specific air conditioner by its device ID.
     *
     * The query joins the `AirConditionersState` table with the `AirConditioners` and `Devices` tables
     * to fetch details about the air conditioner's power status, current temperature, target temperature,
     * mode, fan speed, swing status, and the last seen timestamp.
     *
     * Variables:
     * - `DeviceID`: Parameterized input required to identify the specific air conditioner's records.
     *
     * Returned fields:
     * - `IsPowered`: Indicates whether the air conditioner is powered on or off.
     * - `CurrentTemp`: The current temperature being reported by the air conditioner.
     * - `TargetTemp`: The target temperature set for the air conditioner.
     * - `Mode`: The current mode of the air conditioner (e.g., cooling, heating, etc.).
     * - `FanSpeed`: The fan speed setting of the air conditioner.
     * - `Swing`: Indicates whether the air conditioner's swing functionality is active.
     * - `LastSeen`: Timestamp representing when the device was last active or updated.
     */
    static readonly SELECT_AIR_CONDITIONER_BY_ID = `SELECT IsPowered,
                                                           CurrentTemp,
                                                           TargetTemp,
                                                           Mode,
                                                           FanSpeed,
                                                           Swing,
                                                           d.LastSeen
                                                    FROM AirConditionersState
                                                             JOIN AirConditioners ac on AirConditionersState.ID = ac.ID
                                                             JOIN Devices d on d.ID = ac.DeviceID
                                                    WHERE DeviceID = (?)`;

    /**
     * Stored Procedure: RegisterAirConditionerWithDevice
     *
     * Description:
     * This procedure handles the registration of a new air conditioner device and associates it with a user.
     * If the user doesn't exist, it creates a new user account. The procedure manages the entire registration
     * process within a single transaction to ensure data consistency.
     *
     * Parameters:
     * @param in_username VARCHAR(100) - The username for the user account
     * @param in_email VARCHAR(100) - Email address of the user (used as unique identifier)
     * @param in_password VARCHAR(100) - User's password (should be pre-hashed)
     * @param in_device_id VARCHAR(20) - Unique identifier for the device
     * @param in_device_name VARCHAR(100) - Display name for the device
     * @param in_location VARCHAR(100) - Physical location of the device
     * @param in_ac_name VARCHAR(100) - Name of the air conditioner unit
     * @param in_ac_brand VARCHAR(100) - Brand of the air conditioner
     * @param in_ac_model VARCHAR(100) - Model number/name of the air conditioner
     * @param in_current_temp DECIMAL(5,2) - Current temperature reading
     * @param in_target_temp DECIMAL(5,2) - Desired temperature setting
     * @param in_fan_speed VARCHAR(20) - Fan speed setting (Low/Medium/High/Auto)
     * @param in_mode VARCHAR(20) - Operating mode (Cool/Heat/Fan/Dry/Auto)
     * @param in_swing BOOLEAN - Swing function state (TRUE/FALSE)
     *
     * Tables Affected:
     * - Users: Creates new user if email doesn't exist
     * - Devices: Registers new device
     * - Users_devices: Links user with device
     * - AirConditioners: Stores AC unit information
     * - AirConditionersState: Stores AC current state
     *
     * Returns: None
     *
     * Error Handling:
     * - Uses transaction to ensure all operations complete successfully
     * - Rolls back all changes if any operation fails
     *
     * Example Usage:
     * CALL RegisterAirConditionerWithDevice(
     *     'JohnDoe',
     *     'john@example.com',
     *     'hashedpassword123',
     *     'DEV123',
     *     'Living Room AC',
     *     'Living Room',
     *     'CoolMaster 3000',
     *     'Samsung',
     *     'AM3000',
     *     25.5,
     *     23.0,
     *     'Auto',
     *     'Cool',
     *     TRUE);
     */
    static readonly CREATE_AIR_CONDITIONER = `CALL RegisterAirConditionerWithDevice((?), (?), (?), (?), (?), (?), (?), (?), (?), (?), (?), (?), (?), (?), (?), (?), (?), (?))`;
    static readonly UPDATE_AIR_CONDITIONER_BY_ID = `CALL UpdateAirConditionerAndState(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)`;
    static readonly DELETE_AIR_CONDITIONER_BY_ID = ``
}

export default DatabaseQueries;