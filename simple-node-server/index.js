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
    daly: 0,
    total: 0,
    current: 0,
    currentTimeStamp: new  Date(),
    dalyEntrys: []
}

app.post('/data', (req, res, next) => {
    console.log(req.body);

    //TODO: save it into the db

    //TODO: fill global var

})


// deliver template website
app.get('/', (req, res, next) => {
    
    //TODO: create prerendert website
    res.send();
})