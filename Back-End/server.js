import express from 'express';

const app = express();
const port = 3000;

let sendOptions = { root: "../Front-End"};
app.use(express.json());

app.get('/site_style.css', function(req, res) {
    res.sendFile( "styles.css", sendOptions);
});

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