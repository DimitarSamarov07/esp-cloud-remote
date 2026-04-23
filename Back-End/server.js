import express from 'express';
import { fileURLToPath } from 'url';
import { dirname } from 'path';
import * as path from 'node:path';

import * as AirConditionerController from "./controllers/AirConditioner.controller.js";
import airConditionerRoutes from './routes/AirConditioner.routes.js';
import { fetchDevices } from "./services/data/Device.ts";
import esp from "./services/MQTTService.ts";
import dotenv from "dotenv";

const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

dotenv.config();
const app = express();
app.use(express.json());
const port = 8690;

const deviceStatusMap = new Map();

// Inject MQTT client into request object for route controllers to use
app.use((req, res, next) => {
    req.mqttClient = esp;
    next();
});

// Safe to register handlers before connect() is called
esp.registerControllerHandler('ac/status', AirConditionerController.getAirConditionerByID);

app.use('/air-conditioner', airConditionerRoutes);

let sendOptions = { root: path.join(__dirname, '../Front-End') };
app.use("/public", express.static(path.join(__dirname, '/../Front-End/public/')));

app.get('/', (req, res) => {
    res.sendFile('index.html', sendOptions);
});

app.get('/listDevices', async (req, res) => {
    const devices = await fetchDevices();
    res.json(devices);
});

app.get('/status', (req, res) => {
    const { deviceId } = req.query;
    if (!deviceId) {
        return res.status(406).json({ error: 'Please insert a deviceID' });
    }
    try {
        const status = deviceStatusMap.get(deviceId);
        if (!status) {
            return res.status(404).json({ error: 'Device not found or no data yet' });
        }

        const result = {
            Temp: status.Temperature,
            Swing: status.Swing,
            LastSeen: status.LastSeen ? status.LastSeen.toISOString() : null
        };

        res.send(result);
    } catch (err) {
        console.error(err);
        res.status(500).json({ error: 'Internal server error' });
    }
});




// IMPORTANT: HTTPS config SHOULD BE UNCOMMENTED IN PRODUCTION
// https.createServer(options, app).listen(port, () => { ...
app.listen(port, () => {
    console.log(`Listening on port ${port}`);

    // Server is up and ready for Webhooks. Connect MQTT now.
    esp.connect();
});