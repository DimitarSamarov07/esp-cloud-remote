import express from 'express';
import {fileURLToPath} from 'url';
import {dirname} from 'path';
import * as path from 'node:path';
import bcrypt from 'bcrypt';
import MqttClient from './MqttClient.js';
import * as AirConditionerController from "./controllers/AirConditioner.controller.js";
import airConditionerRoutes from './routes/AirConditioner.routes.js';
import dotenv from 'dotenv'
import {pool} from "./services/Pool.ts";

dotenv.config()
const processEnv = process.env;
const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

const app = express();
app.use(express.json());
const port = 8690;

// IMPORTANT: HTTPS config SHOULD BE UNCOMMENTED IN PRODUCTION
// const options = {
//     key: fs.readFileSync(path.join(__dirname, 'certs/key.pem')),
//     cert: fs.readFileSync(path.join(__dirname, 'certs/cert.pem'))
// };
const saltRounds = 14;
const deviceStatusMap = new Map();

const esp = new MqttClient({
    clientId: 'server',
    username: processEnv.MQTT_USERNAME,
    password: processEnv.MQTT_PASSWORD,
    clean: false,
    connectTimeout: 1000,
    reconnectPeriod: 5000,
    protocolVersion: 5,

});
app.use((req, res, next) => {
    req.mqttClient = esp;
    next();
});
esp.registerControllerHandler('ac/status', AirConditionerController.getAirConditionerByID);


app.use('/air-conditioner', airConditionerRoutes);

let sendOptions = { root: path.join(__dirname, '../Front-End') };
app.use(express.json());
app.use("/public", express.static(path.join(__dirname, '/../Front-End/public/')));

app.post("/changeWifi", async function (req, res) {
    let {deviceID, ssid, pass} = req.body;
    console.log(deviceID, ssid, pass);

    await esp.changeWifi(deviceID, ssid, pass);
    res.sendStatus(200);
})

app.get('/', (req, res) => {
    res.sendFile('index.html', sendOptions);
});


esp.onAcStatusReceived = (status) => {
    deviceStatusMap.set(status.DeviceId, status);
};
app.get('/status', (req, res) => {
    const {deviceId} = req.query;
    if (!deviceId) {
        return res.status(406).json({error: 'Please insert a deviceID'});
    }
    try {
        const status = deviceStatusMap.get(deviceId);
        if (!status) {
            return res.status(404).json({error: 'Device not found or no data yet'});
        }


        const result = {
            Temp: status.Temperature,
            Swing: status.Swing,
            LastSeen: status.LastSeen ? status.LastSeen.toISOString() : null
        };

        res.send(result);
    } catch (err) {
        console.error(err);
        res.status(500).json({error: 'Internal server error'});
    }

});

app.post('/register/device', (req, res) => {
    register(res, {
        isDevice: true,
        deviceID: req.body.deviceID,
        password: req.body.password,
        targetUserID: 1 // Forcing User 1 as the owner
    });
});

/*
IMPORTANT: The EMQX broker only works with codes 200,204,4XX,5XX.
If the 4XX or 5XX codes are sent, the broker will presume that the request is
wrong and determine the result to be ignored which would practically deny the
authentication.
 */
app.post('/validateCredentials', async (req, res) => {
    const { username, password, isDevice } = req.body;

    try {
        let dbPassword;
        if (isDevice === "true") {
            // Username here represents the DeviceID
            const [rows] = await pool.execute(
                `SELECT DevicePassword FROM Devices WHERE ID = ?`,
                [username]
            );
            if (rows.length > 0) dbPassword = rows[0].DevicePassword;
        } else {
            const [rows] = await pool.execute(
                `SELECT Password FROM Users WHERE Username = ?`,
                [username]
            );
            if (rows.length > 0) dbPassword = rows[0].Password;
        }

        if (!dbPassword) return res.status(200).json({ result: "deny" });

        const match = await bcrypt.compare(password, dbPassword);
        return res.status(200).json({ result: match ? "allow" : "deny" });

    } catch (err) {
        return res.status(200).json({ result: "deny" });
    }
});


/**
 * Function that registers a device in the DB with a given password and email.Hashes a password with the specified salt round. It also generates a username using the Str_random function declared below.
 *
 * @async
 *
 * @param {http.ServerResponse} res
 * @param {string} email
 * @param {string} passwordFromDevice
 *
 * @returns {Promise<Response>} Will return 200 if the entry is inserted into the DB and 500 if there was an error during the registration.
 * */

/**
 * Combined registration logic
 * @param {Object} data - Contains the necessary fields based on isDevice
 */
async function register(res, data) {
    const { isDevice, password } = data;
    const hashedPass = await hashPassword(password);

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


const hashPassword = async (password) => await bcrypt.hash(password, saltRounds);


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

// IMPORTANT: HTTPS config SHOULD BE UNCOMMENTED IN PRODUCTION
// https.createServer(options, app).listen(port, () => {
//     console.log(`HTTPS server running at https://localhost:${port}`);
// });
app.listen(port, () => {
    console.log(`Listening on port ${port}`);
})


