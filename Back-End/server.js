import express from 'express';
import {fileURLToPath} from 'url';
import {dirname} from 'path';
import * as path from "node:path";
import {esp_dummy} from "./mqtt-node/mqtt-communication.js";

const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

const app = express();
const port = 8690;

let sendOptions = {root: "../Front-End"};
app.use(express.json());
app.use("/public", express.static(path.join(__dirname, '/../Front-End/public/')));


app.post("/led", function (req, res) {
    let led_state = req.query.led_state;
    switch (led_state) {
        case "ON":
            esp_dummy.turnLEDOn();
            break;
        case "OFF":
            esp_dummy.turnLEDOff()
        default:
            res.sendStatus(404);
            return;
    }
    res.sendStatus(200);
})

app.post("/changeWifi", function (req, res) {
    let ssid = req.query.ssid;
    let pass = req.query.password;

    esp_dummy.changeWifi(ssid, pass);
    res.sendStatus(200);
})


app.get('/', (req, res) => {
    res.sendFile('index.html', sendOptions);
})

app.post('/double', (req, res) => {
    const number = req.body.number;

    if (typeof number === 'number') {
        const doubled = 2 * number;
        res.json({doubledNumber: doubled});
    } else {
        res.status(400).json({error: 'Please enter a number!'});
    }
});

app.post('/random', (req, res) => {
    const randomNumber = Math.floor(Math.random() * 10) + 1;
    res.send({
        message: `Post received ${randomNumber}`,
    });
});

app.listen(port, () => {
    console.log(`Server started on port ${port}`);
});
