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
    clientId: 'mqtt_pc1',
    clean: false,
    connectTimeout: 1000,
    reconnectPeriod: 20000,
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

app.post("/changeWifi", function (req, res) {
    let {ssid, pass} = req.query;

    mqttClient.changeWifi(ssid, pass);
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
});


app.post('/credentials', async (req, res) => {
    let {usernameFromDevice,passwordFromDevice} = req.body;

    if (!passwordFromDevice) {
        return res.status(400).json({ error: 'Password is required.' });
    }

    if (!usernameFromDevice) {
        await register(res, usernameFromDevice, passwordFromDevice);
    } else {
        try {
            const [rows] = await connection.execute(`SELECT Password
                                                     FROM Users
                                                     WHERE Username = ?`, [usernameFromDevice]);
            if (rows.length === 0) {
                return res.status(401).json({error: 'User not found.'});
            }

            const match = await bcrypt.compare(passwordFromDevice, rows[0].Password);
            if (match) {
                return res.json({success: true, username: usernameFromDevice});
            } else {
                return res.status(403).json({error: 'Invalid credentials.'});
            }
        } catch (err) {
            return res.status(500).json({error: 'Database error during login.'});
        }
    }
});

async function register(res, usernameFromDevice, passwordFromDevice) {
    let username = "mqtt_pc_" + Str_Random(16);
    const hashedPass = await hashPassword(passwordFromDevice);
    try {
        const [result] = await connection.execute(
            `INSERT INTO Users(Username, Password)
             VALUES (?, ?)`,
            [username, hashedPass]
        );
        const userId = result.insertId;
        await connection.execute(
            `INSERT INTO ACL_Table (UserID, TopicID, RW)
             SELECT ?, ID, 3
             FROM Topics`,
            [userId]
        );

        return res.status(200).send({username, password: passwordFromDevice});

    } catch (err) {
        console.error('Registration error:', err);
        return res.status(500).json({error: 'Database error during registration.'});
    }
}
const hashPassword = async (password) => await bcrypt.hash(password, saltRounds);

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


