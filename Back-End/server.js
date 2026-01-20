import express from 'express';
import {fileURLToPath} from 'url';
import {dirname} from 'path';
import * as path from 'node:path';
import bcrypt from 'bcrypt';
import MqttClient from './MqttClient.js';
import * as AirConditionerController from "./controllers/AirConditioner.controller.js";
import airConditionerRoutes from './routes/AirConditioner.routes.js';
import dotenv from 'dotenv'
import mysql from "mysql2/promise";

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

const dbConfig = {
    host: 'localhost',
    port: 3306,
    user: processEnv.DB_USER,
    password: processEnv.DB_PASSWORD,
    database: processEnv.DATABASE,
};

const connection = await mysql.createConnection(dbConfig);
const saltRounds = 14;
const deviceStatusMap = new Map();
const deviceId = '';

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


app.use('/airconditioner', airConditionerRoutes);

let sendOptions = { root: path.join(__dirname, '../Front-End') };
app.use(express.json());
app.use("/public", express.static(path.join(__dirname, '/../Front-End/public/')));

app.post("/changeWifi", async function (req, res) {
    let {deviceID, ssid, pass} = req.body;

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
        //Gets a device by the deviceID given in the request.
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

app.post('/register', async (req, res) => {
    let {email, password} = req.body;
    if (!password || !email) {
        res.status(406).json({error: 'Please provide username and password'});
    }
    try {
        await register(res, email, password);
    } catch (err) {
        console.error(err);
        return res.status(500).json({error: 'Internal server error'});
    }
})

/*
IMPORTANT: The EMQX broker only works with codes 200,204,4XX,5XX.
If the 4XX or 5XX codes are sent, the broker will presume that the request is
wrong and determine the result to be ignore which would practically deny the
authentication.
 */
app.post('/validateCredentials', async (req, res) => {
    let {username, password} = req.body;

    /*
    EMQX code the result field that is being sent is so the broker
    can know whether to allow,deny or ignore the request
    */
    if (!password || !username) {
        return res.status(200).json({ result: "deny", error: 'Missing username or password' });
    }

    try {
        const [rows] = await connection.execute(
            `SELECT Password FROM Users WHERE Username = ?`,
            [username]
        );
        //If no rows found - deny
        if (rows.length === 0) {
            return res.status(200).json({ result: "deny", error: 'User not found.' });
        }
        /*
        If there is a row, bcrypt compares the hashed password
        with the password provided and decides whether the
        user can authenticate.
        */
        const match = await bcrypt.compare(password, rows[0].Password);
        if (match) {
            return res.status(200).json({ result: "allow" });
        } else {
            return res.status(200).json({ result: "deny", error: 'Invalid credentials.' });
        }
    } catch (err) {
        console.error(err);
        return res.status(200).json({ result: "deny", error: 'Internal server error.' });
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
async function register(res,email, passwordFromDevice) {
    let username = "mqtt_pc_" + Str_Random(16);
    const hashedPass = await hashPassword(passwordFromDevice);
    try {
        const [result] = await connection.execute(
            `INSERT INTO Users(Username,Email, Password, IsAdmin)
             VALUES (?, ?, ?,0)`,
            [username,email, hashedPass]
        );
        const userId = result.insertId;


        return res.status(200).send({username, password: passwordFromDevice});

    } catch (err) {
        console.error('Registration error:', err);
        return res.status(500).json({error: 'Database error during registration.'});
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


