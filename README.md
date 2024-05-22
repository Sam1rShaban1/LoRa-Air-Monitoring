# Cross-Network-LoRaMesher

This project aims to explore cross-network LoRaMesher applications.

## Installation

Configure your local EMQX broker and Grafana server with the following commands:

```bash
docker run -d --name emqx -p 1883:1883 -p 8083:8083 -p 8883:8883 -p 8084:8084 -p 18083:18083 emqx/emqx:latest

docker run -d -p 3000:3000 --name=grafana grafana/grafana-enterprise
```

## Usage


To start the EMQX broker and Grafana server use the following commands:

```bash
docker start emqx

docker start grafana
```
Credentials for your local emqx broker are user: "admin" password: "local"
Credentials for your local grafana server are user: "admin" password: "admin"

For the React app install npm and use the following comand in the Query app folder:

```bash
npm start
```
