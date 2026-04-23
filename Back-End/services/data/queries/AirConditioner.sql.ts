export class AirConditionerQueries {
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
    static readonly SELECT_DEVICE_AND_ACS = `SELECT Devices.Name,
                                                    Devices.IsOnline as 'isDeviceOn',
                                                    ACS.IsOnline     as 'isAcOn'
                                             FROM Devices
                                                      JOIN esp_remote_db.AirConditioners AC on Devices.ID = AC.DeviceID
                                                      JOIN esp_remote_db.AirConditionersState ACS on AC.ID = ACS.ID`


    static readonly UPDATE_AIR_CONDITIONER_BY_ID = `CALL UpdateAirConditionerAndState((?), (?), (?), (?), (?), (?), (?), (?), (?), (?), (?))`;
    static readonly DELETE_AIR_CONDITIONER_BY_ID = `CALL DeleteAirConditionerByDeviceID((?));`

    static readonly CREATE_AIR_CONDITIONER = `CALL RegisterAirConditionerWithDevice((?), (?), (?), (?), (?), (?), (?), (?), (?), (?), (?), (?), (?), (?), (?), (?), (?), (?))`;
}