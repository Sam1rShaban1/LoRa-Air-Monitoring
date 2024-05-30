# Cross-Network-LoRaMesher

This section aims to setup the web monitoring services.

## Usage

To start the Monitoring Services, in the "MonitoringService" folder execute:

```bash
docker-compose up -d
```

EMQX broker: [http://localhost:18083](http://localhost:18083)
Credentials for your local emqx broker are user: "admin" password: "local"

InfluxDB: [http://localhost:8086](http://localhost:8086)
Credentials for your local influxDB server are user: "root" password: "monitoringservice"

Grafana: [http://localhost:3000](http://localhost:3000)
Credentials for your local grafana server are user: "admin" password: "admin"

Query web app: [http://localhost:3006](http://localhost:3006)

To configure grafana go to: 

Option Menu -> Data sources and add the "influxdb" data source.

Change the query language to "Flux" (default is InfluxQL).

In the HTTP section enter the following URL: http://influxdb:8086.

Go down to "InfluxDB Details" and enter the following:

"Organization" = TFG
"Token" = tfg-iotdata-auth-token
"Default Bucket" = loramesher

Click "Save & test" and the datasource should be successfully added.

Then create a new dashboard, add a visualization and select the InfluxDB data source.
After that add the following example query:

```bash
from(bucket: "loramesher")
  |> range(start: v.timeRangeStart, stop:v.timeRangeStop)
  |> aggregateWindow(every: 1m,fn: mean)
  |> filter(fn: (r) =>
    r._measurement == "mqtt_consumer" and 
    r._field == "data_nServices"
  )
```
