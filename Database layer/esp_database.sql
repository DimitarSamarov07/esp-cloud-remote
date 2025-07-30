CREATE DATABASE IF NOT EXISTS esp_remote_db;
use esp_remote_db;


DROP TABLE IF EXISTS AirConditionersState;
DROP TABLE IF EXISTS AirConditioners;
DROP TABLE IF EXISTS Users_devices;
DROP TABLE IF EXISTS Devices;
DROP TABLE IF EXISTS Users;


CREATE TABLE IF NOT EXISTS Users
(
    ID        BIGINT AUTO_INCREMENT PRIMARY KEY,
    Username  VARCHAR(50)  NOT NULL,
    Email     VARCHAR(100) NOT NULL,
    Password  VARCHAR(150) NOT NULL,
    IsAdmin   BOOLEAN   DEFAULT FALSE,
    CreatedAt TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);


CREATE TABLE IF NOT EXISTS Devices
(
    ID           VARCHAR(36) PRIMARY KEY,
    Name         VARCHAR(50) NOT NULL,
    Location     VARCHAR(50),
    IsOnline     BOOLEAN DEFAULT FALSE,
    LastSeen     TIMESTAMP   NULL,
    RegisteredAt BOOLEAN DEFAULT FALSE
);


CREATE TABLE IF NOT EXISTS Users_devices
(
    ID       VARCHAR(36) PRIMARY KEY,
    UserID   BIGINT      NOT NULL,
    DeviceID VARCHAR(36) NOT NULL,
    FOREIGN KEY (UserID) REFERENCES Users (ID) ON DELETE CASCADE,
    FOREIGN KEY (DeviceID) REFERENCES Devices (ID) ON DELETE CASCADE,
    UNIQUE (UserID, DeviceID)
);


CREATE TABLE IF NOT EXISTS AirConditioners
(
    ID        BIGINT AUTO_INCREMENT PRIMARY KEY,
    DeviceID  VARCHAR(36) NOT NULL,
    Name      VARCHAR(50) NOT NULL,
    Brand     VARCHAR(70),
    Model     VARCHAR(30),
    CreatedAt TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    LastSeen  TIMESTAMP   NULL,
    FOREIGN KEY (DeviceID) REFERENCES Devices (ID) ON DELETE CASCADE
);


CREATE TABLE IF NOT EXISTS AirConditionersState
(
    ID          BIGINT PRIMARY KEY,
    IsOnline    BOOLEAN                                     DEFAULT FALSE,
    IsPowered   BOOLEAN                                     DEFAULT FALSE,
    CurrentTemp DECIMAL(5, 2),
    TargetTemp  DECIMAL(5, 2),
    FanSpeed    ENUM ('Low', 'Medium', 'High', 'Auto')      DEFAULT 'Auto',
    Mode        ENUM ('Cool', 'Heat', 'Fan', 'Dry', 'Auto') DEFAULT 'Auto',
    Swing       BOOLEAN,
    LastUpdated TIMESTAMP                                   DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (ID) REFERENCES AirConditioners (ID) ON DELETE CASCADE
);

-- Insert sample users
INSERT INTO Users (Username, Email, Password, IsAdmin)
VALUES ('john_doe', 'john@example.com', 'hashed_password_123', FALSE),
       ('admin_user', 'admin@example.com', 'hashed_password_admin', TRUE),
       ('alice_smith', 'alice@example.com', 'hashed_password_alice', FALSE);

-- Insert sample devices
INSERT INTO Devices (ID, Name, Location, IsOnline, LastSeen, RegisteredAt)
VALUES ('device-uuid-001', 'Living Room Sensor', 'Living Room', TRUE, NOW(), TRUE),
       ('device-uuid-002', 'Bedroom Thermostat', 'Bedroom', FALSE, NULL, TRUE),
       ('device-uuid-003', 'Kitchen AC Unit', 'Kitchen', TRUE, NOW(), TRUE);

-- Link users and devices
INSERT INTO Users_devices (ID, UserID, DeviceID)
VALUES ('link-uuid-001', 1, 'device-uuid-001'), -- john_doe <-> Living Room Sensor
       ('link-uuid-002', 2, 'device-uuid-002'), -- admin_user <-> Bedroom Thermostat
       ('link-uuid-003', 3, 'device-uuid-003');
-- alice_smith <-> Kitchen AC Unit

-- Insert sample AirConditioners
INSERT INTO AirConditioners (DeviceID, Name, Brand, Model)
VALUES ('device-uuid-003', 'Kitchen AC', 'CoolBrand', 'CB-1234'),
       ('device-uuid-002', 'Bedroom AC', 'AirTech', 'AT-5678');

-- Insert sample AirConditionersState
INSERT INTO AirConditionersState (ID, IsOnline, IsPowered, CurrentTemp, TargetTemp, FanSpeed, Mode, Swing, LastUpdated)
VALUES (1, TRUE, TRUE, 24.5, 22.0, 'Auto', 'Cool', TRUE,'2025-04-01 00:00:00'),
       (2, FALSE, FALSE, 25, 26, 'Low', 'Heat', FALSE, '2025-03-01 00:00:00'),
       (3, TRUE, TRUE, 23.5, 22.0, 'Auto', 'Cool', TRUE,'2025-01-01 00:00:00');


DELIMITER //

CREATE PROCEDURE RegisterAirConditionerWithDevice(
    IN in_username VARCHAR(100),
    IN in_email VARCHAR(100),
    IN in_password VARCHAR(100),
    IN in_device_id VARCHAR(36),
    IN in_device_name VARCHAR(100),
    IN in_location VARCHAR(100),
    IN in_ac_name VARCHAR(100),
    IN in_ac_brand VARCHAR(100),
    IN in_ac_model VARCHAR(100),
    IN in_ac_last_seen DATETIME,
    IN in_is_online BOOLEAN,
    IN in_is_powered BOOLEAN,
    IN in_current_temp DECIMAL(5, 2),
    IN in_target_temp DECIMAL(5, 2),
    IN in_fan_speed VARCHAR(20),
    IN in_mode VARCHAR(20),
    IN in_swing BOOLEAN,
    IN in_last_updated DATETIME
)
BEGIN
    DECLARE uid BIGINT;
    DECLARE ac_id BIGINT;

    START TRANSACTION;
    SELECT ID INTO uid FROM Users WHERE Email = in_email LIMIT 1;
    IF uid IS NULL THEN
        INSERT INTO Users (Username, Email, Password, IsAdmin)
        VALUES (in_username, in_email, in_password, FALSE);
        SET uid = LAST_INSERT_ID();
    END IF;
    INSERT INTO Devices (ID, Name, Location, IsOnline, LastSeen, RegisteredAt)
    VALUES (in_device_id, in_device_name, in_location, in_is_online, in_ac_last_seen, TRUE);

    INSERT INTO Users_devices (ID, UserID, DeviceID)
    VALUES (UUID(), uid, in_device_id);
    INSERT INTO AirConditioners (DeviceID, Name, Brand, Model, LastSeen)
    VALUES (in_device_id, in_ac_name, in_ac_brand, in_ac_model, in_ac_last_seen);
    SET ac_id = LAST_INSERT_ID();

    INSERT INTO AirConditionersState (ID, IsOnline, IsPowered, CurrentTemp, TargetTemp, FanSpeed, Mode, Swing, LastUpdated)
    VALUES (ac_id,in_is_online,in_is_powered, in_current_temp, in_target_temp,in_fan_speed, in_mode, in_swing, in_last_updated);
    COMMIT;
END //


DELIMITER ;


CREATE PROCEDURE UpdateAirConditionerAndState(
    IN in_ac_id VARCHAR(36),
    IN in_name VARCHAR(100),
    IN in_brand VARCHAR(100),
    IN in_model VARCHAR(100),
    IN in_is_online BOOLEAN,
    IN in_is_powered BOOLEAN,
    IN in_current_temp DECIMAL(5, 2),
    IN in_target_temp DECIMAL(5, 2),
    IN in_fan_speed ENUM ('Low', 'Medium', 'High', 'Auto'),
    IN in_mode ENUM ('Cool', 'Heat', 'Fan', 'Dry', 'Auto'),
    IN in_swing BOOLEAN
)
BEGIN
    START TRANSACTION;
    UPDATE AirConditioners
    SET Name  = COALESCE(in_name, Name),
        Brand = COALESCE(in_brand, Brand),
        Model = COALESCE(in_model, Model)
    WHERE DeviceID = in_ac_id;
    UPDATE AirConditionersState
        JOIN AirConditioners AC on AC.ID = AirConditionersState.ID
    SET IsPowered   = COALESCE(in_is_powered, IsPowered),
        IsOnline    = COALESCE(in_is_online, IsOnline),
        CurrentTemp = COALESCE(in_current_temp, CurrentTemp),
        TargetTemp  = COALESCE(in_target_temp, TargetTemp),
        FanSpeed    = COALESCE(in_fan_speed, FanSpeed),
        Mode        = COALESCE(in_mode, Mode),
        Swing       = COALESCE(in_swing, Swing),
        LastUpdated = NOW()
    WHERE AC.DeviceID = in_ac_id;
    COMMIT;
END;

DELIMITER $$

CREATE PROCEDURE DeleteAirConditionerByDeviceID(IN inputDeviceID VARCHAR(36))
BEGIN
    DECLARE acID BIGINT DEFAULT NULL;
    DECLARE CONTINUE HANDLER FOR NOT FOUND SET acID = NULL;

    -- Try to get the ID
    SELECT ID INTO acID FROM AirConditioners WHERE DeviceID = inputDeviceID LIMIT 1;

    -- Check if found
    IF acID IS NOT NULL THEN
        DELETE FROM AirConditionersState WHERE ID = acID;
        DELETE FROM AirConditioners WHERE ID = acID;
        SELECT 'Deleted' AS status;
    ELSE
        SELECT 'NotFound' AS status;
    END IF;
END$$

DELIMITER ;


drop procedure DeleteAirConditionerByDeviceID;


