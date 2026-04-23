export class DeviceQueries {
    static readonly SELECT_DEVICE_AND_ACS = `SELECT Devices.Name,
                                                    Devices.IsOnline as 'isDeviceOn',
                                                    ACS.IsOnline     as 'isAcOn'
                                             FROM Devices
                                                      JOIN esp_remote_db.AirConditioners AC on Devices.ID = AC.DeviceID
                                                      JOIN esp_remote_db.AirConditionersState ACS on AC.ID = ACS.ID`
    static readonly ADD_DEVICE = `INSERT INTO Devices (ID, Name, DevicePassword, RegisteredAt)
                                  VALUES (?, ?, ?, ?)`;

    static readonly CONNECT_USER_TO_DEVICE = `INSERT INTO Users_devices (ID, UserID, DeviceID)
                                              VALUES (UUID(), ?, ?)`;
}