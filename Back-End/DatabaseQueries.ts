class DatabaseQueries {

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
    static readonly CREATE_AIR_CONDITIONER = `CALL RegisterAirConditionerWithDevice((?), (?), (?), (?), (?), (?), (?), (?), (?), (?), (?), (?), (?), (?), (?), (?), (?), (?))`;

    static readonly UPDATE_AIR_CONDITIONER_BY_ID = `CALL UpdateAirConditionerAndState((?), (?), (?), (?), (?), (?), (?), (?), (?), (?), (?))`;
    static readonly DELETE_AIR_CONDITIONER_BY_ID = `CALL DeleteAirConditionerByDeviceID((?));`
}

export default DatabaseQueries;