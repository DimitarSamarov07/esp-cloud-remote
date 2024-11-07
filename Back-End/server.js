import express from 'express';
import {fileURLToPath} from 'url';
import {dirname} from 'path';
import * as path from "node:path";

import express_ws from "express-ws";

const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

const app = express();
const port = 8690;
let expressWs = express_ws(app);

let global_ws;
let sendOptions = {root: "../Front-End"};
app.use(express.json());
app.use("/public", express.static(path.join(__dirname, '/../Front-End/public/')));

app.ws("/return-ws", function (ws, req) {
    global_ws = ws;
    ws.on("message", function(msg){
        ws.send("I agree");
    })
})

app.post("/led", function (req, res) {
    let led_state = req.query.led_state;
    let objToSend = {};
    switch (led_state) {
        case "ON":
            objToSend["led_state"] = "ON";
            break;
        case "OFF":
            objToSend["led_state"] = "OFF";
            break;
    }
    ws.send(JSON.stringify(objToSend));
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
