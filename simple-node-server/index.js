const express = require("express");
const Influx = require("influx");
const schedule = require('node-schedule');

var app = express();
var server = require("http").createServer(app);
const port = 3001;

const MEASURE_DAILY = "SOLAR_DAILY";
const MEASURE_OVERVIEW = "SOLAR_OVERVIEW";

// Express
app.use(
  express.urlencoded({
    extended: true,
  })
);

app.use(express.json());

// influx db
const influx = new Influx.InfluxDB({
  host: "localhost:8086",
  database: "solar",
  schema: [
    {
      measurement: MEASURE_DAILY,
      fields: {
        gridVoltage: Influx.FieldType.FLOAT,
        gridCurrent: Influx.FieldType.FLOAT,
        gridPower: Influx.FieldType.INTEGER,
        pv1Voltage: Influx.FieldType.FLOAT,
        pv1Current: Influx.FieldType.FLOAT,
        pv1Power: Influx.FieldType.INTEGER,
        temp: Influx.FieldType.FLOAT,
      },
      tags: ["power"],
    },

    {
      measurement: MEASURE_OVERVIEW,
      fields: {
        eToday: Influx.FieldType.FLOAT,
        eTotal: Influx.FieldType.INTEGER,
      },
    },
  ],
});

// create schedule 

schedule.scheduleJob("saveEndOfDay", '* 59 23 * *', saveEndOfDay())


//global data holder for ram
let ram = {
  eToday: 0.0,
  eTotal: 0,
  currentTimeStamp: new Date(),
};

// express 

app.post("/data", (req, res, next) => {
  if (req.body.hasOwnProperty("eToday")) {
    //TODO: save it into the db
    ram.eToday = req.body.eToday;
    ram.eTotal = req.body.eTotal;
    ram.currentTimeStamp = new Date();
    saveIntoDB(req);
    res.send(200);
  }
});

// deliver template website
app.get("/", (req, res, next) => {
  res.send(ram);
});

server.listen(port, function () {
  console.log("Webserver created and listen on port: %d", port);
});

/** Functions */

async function saveIntoDB(req) {
  influx.writeMeasurement(MEASURE_DAILY, [
    {
      fields: {
        gridVoltage: req.body.gridVoltage,
        gridCurrent: req.body.gridCurrent,
        gridPower: req.body.gridPower,
        pv1Voltage: req.body.pv1Voltage,
        pv1Current: req.body.pv1Current,
        pv1Power: req.body.pv1Power,
        temp: req.body.temp,
      },
    },
  ]);
}

async function saveEndOfDay(){
    influx.writeMeasurement(MEASURE_OVERVIEW, [{
        fields: {
            eToday: ram.eToday,
            eTotal: ram.eTotal
        }
    }])
    //reset 
    ram.eToday = 0;
}