import express from 'express';
import {fileURLToPath} from 'url';
import {dirname} from 'path';
import * as path from "node:path";

const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

const app = express();
const port = 3000;

let sendOptions = { root: "../Front-End"};
app.use(express.json());
app.use("/public", express.static(path.join(__dirname, '/../Front-End/public/')));

app.get('/', (req, res) => {
    res.sendFile( 'index.html', sendOptions);
})

app.post('/double', (req, res) => {
    const number = req.body.number;

    if(typeof number === 'number') {
        const doubled = 2*number;
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
