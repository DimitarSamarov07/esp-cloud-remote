import express from 'express';
import { fileURLToPath } from 'url';
import { dirname } from 'path';
import * as path from 'node:path';
import https from 'https';
import fs from 'fs';
import bcrypt from 'bcrypt';
import mysql from 'mysql2/promise';

const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

const app = express();
const port = 8690;

// IMPORTANT: HTTPS config SHOULD BE UNCOMMENTED IN PRODUCTION
// const options = {
//     key: fs.readFileSync(path.join(__dirname, 'certs/key.pem')),
//     cert: fs.readFileSync(path.join(__dirname, 'certs/cert.pem'))
// };

const dbConfig = {
    host: 'localhost',
    port: 3306,
    user: 'root',
    password: 'a1n2g3e4l5',
    database: 'esp_remote',
};

const connection = await mysql.createConnection(dbConfig);
const saltRounds = 14;

let sendOptions = { root: path.join(__dirname, '../Front-End') };
app.use(express.json());
app.use("/public", express.static(path.join(__dirname, '/../Front-End/public/')));

app.post("/led", (req, res) => {
    let led_state = req.query.led_state;
    // handle LED state logic here
    res.sendStatus(200);
});

app.post("/changeWifi", (req, res) => {
    let ssid = req.query.ssid;
    let pass = req.query.password;
    // handle WiFi change logic here
    res.sendStatus(200);
});

app.get('/', (req, res) => {
    res.sendFile('index.html', sendOptions);
});

app.post('/credentials', async (req, res) => {
    let { usernameFromDevice, passwordFromDevice } = req.query;


    if (!passwordFromDevice) {
        return res.status(400).json({ error: 'Password is required.' });
    }

    if (!usernameFromDevice) {
        let username = "mqtt_pc_" + Str_Random(16);
        const hashedPass = await hashPassword(passwordFromDevice);
        try {
            await connection.execute(`INSERT INTO Users(Username, Password) VALUES (?, ?)`, [username, hashedPass]);
            return res.send({ username, password: passwordFromDevice }).status(200);
        } catch (err) {
            return res.status(500).json({ error: 'Database error during registration.' });
        }
    } else {
        try {
            const [rows] = await connection.execute(`SELECT Password FROM Users WHERE Username = ?`, [usernameFromDevice]);
            if (rows.length === 0) {
                return res.status(401).json({ error: 'User not found.' });
            }

            const match = await bcrypt.compare(passwordFromDevice, rows[0].Password);
            if (match) {
                return res.json({ success: true, username: usernameFromDevice });
            } else {
                return res.status(403).json({ error: 'Invalid credentials.' });
            }
        } catch (err) {
            return res.status(500).json({ error: 'Database error during login.' });
        }
    }
});

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
