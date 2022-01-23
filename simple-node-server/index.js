const express = require('express');

var app = express();
var server = require('http').createServer(app);
var port = 3001;


// Express 
app.use(express.urlencoded({
    extended: true
}));
app.use(express.json());

//global data holder for ram
let Solar = {
    eToday: 0.0,
    eTotal: 0,
    gridVoltage: 0.0,
    gridCurrent: 0.0,
    gridPower: 0,
    pv1Voltage:0.0,
    pv1Current:0.0,
    pv1Power: 0,
    pv2Voltage:0.0,
    pv2Current:0.0,
    pv2Power: 0,
    temp: 0,
    currentTimeStamp: new  Date(),
    dalyEntrys: []
}

app.post('/data', (req, res, next) => {
    console.log(req.body);

    //TODO: save it into the db
    Solar.eToday = req.body.eToday;
    Solar.eTotal = req.body.eTotal; 
    Solar.gridVoltage = req.body.gridVoltage; 
    Solar.gridCurrent = req.body.gridCurrent; 
    Solar.gridPower = req.body.gridPower; 
    Solar.pv1Voltage = req.body.pv1Voltage; 
    Solar.pv1Current = req.body.pv1Current; 
    Solar.pv1Power = req.body.pv1Power; 
    Solar.pv2Voltage = req.body.pv2Voltage; 
    Solar.pv2Current = req.body.pv2Current; 
    Solar.pv2Power = req.body.pv2Power;
    Solar.temp = req.body.temp;
    Solar.currentTimeStamp = new  Date();
})


// deliver template website
app.get('/', (req, res, next) => {
    
    //TODO: create prerendert website
    res.send(Solar);
})

server.listen(port, function() {
    console.log('Webserver created and listen on port: %d', port);
});

