const express = require("express");
const { InfluxDB, Point, HttpError } = require("@influxdata/influxdb-client");
const {PingAPI} = require('@influxdata/influxdb-client-apis')
const schedule = require("node-schedule");
require("dotenv").config();

var app = express();
var server = require("http").createServer(app);
const port = 3001;
const writeApi = new InfluxDB({
  url: "http://influxdb2:8086",
  token: process.env.INFLUXDB_ADMIN_TOKEN,
  timeout: 3000,
}).getWriteApi(process.env.INFLUXDB_ORG, process.env.INFLUXDB_BUCKET, "ns");

const pingAPI = new PingAPI(writeApi);
const SOLAR_HOURLY = "SOLAR_HOURLY";
const SOLAR_OVERVIEW = "SOLAR_OVERVIEW";

// Express
app.use(
  express.urlencoded({
    extended: true,
  })
);

app.use(express.json());

//global data holder for ram
let ram = {
  eToday: 0.0,
  eTotal: 0,
  currentTimeStamp: new Date(),
};


// create schedule

schedule.scheduleJob("saveEndOfDay", "59 23 * * *", ()=> {saveEndOfDay()});

// express

app.post("/data", (req, res, next) => {
  if (req.body.hasOwnProperty("eToday")) {
    //TODO: save it into the db
    ram.eToday = req.body.eToday;
    ram.eTotal = req.body.eTotal;
    ram.currentTimeStamp = new Date();
    saveIntoDB(req);
    res.sendStatus(200);
  }
});

// deliver template website
app.get("/", (req, res, next) => {
  res.send(ram);
});

server.listen(port, function () {
  console.log("[WEB] Webserver created and listen on port: %d", port);
});

pingAPI
  .getPing()
  .then(() => {
    console.log("[DB] Ping SUCCESS");
  })
  .catch((error) => {
    console.error(error);
    console.log("[DB] Finished ERROR");
    process.exit();
  });

/** Functions */

async function saveIntoDB(req) {
  const point = new Point(SOLAR_HOURLY)
    .floatField("gridVoltage", req.body.gridVoltage)
    .floatField("gridCurrent", req.body.gridCurrent)
    .intField("gridPower", req.body.gridPower)
    .floatField("pv1Voltage", req.body.pv1Voltage)
    .floatField("pv1Current", req.body.pv1Current)
    .intField("pv1Power", req.body.pv1Power)
    .floatField("temp", req.body.temp);

  writeApi.writePoint(point);
  writeApi.flush();
}

function saveEndOfDay() {
  const point = new Point(SOLAR_OVERVIEW)
    .floatField("eToday", ram.eToday)
    .intField("eTotal", ram.eTotal);

  //reset
  ram.eToday = 0;
}

/**
 *
 */
process.on("SIGTERM", async () => {
  console.info("SIGTERM signal received.");
  console.log("[WEB] Closing http server.");
  server.close(() => {
    console.log("[WEB] Http server closed.");
  });
  console.log("[DB] Save data into InfluxDB");
  await writeApi.flush();
  await writeApi.close();
});
