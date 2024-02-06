const { SerialPort } = require("serialport");
const { DelimiterParser } = require("@serialport/parser-delimiter");

const express = require("express");
const app = express();
const http = require("http");
const server = http.createServer(app);
const { Server } = require("socket.io")

var comport = undefined;
var port = undefined;
var parser = undefined;

SerialPort.list().then((devices) => {
  devices.forEach((device) => {
    //  vendorId: '10C4',
    //  productId: 'EA60'
    if (device.vendorId == "10C4" && device.productId == "EA60") {
      comport = device.path;
    }
  });
  console.log(comport);
  if (comport != undefined) {
    port = new SerialPort({ path: comport, baudRate: 115200 });
    parser = port.pipe(new DelimiterParser({ delimiter: "\n" }));
    const io = new Server(server);
    io.on("connection", (socket) => {
      console.log("a user connected");
    });
    server.listen(3000, () => {
      console.log("listening on *:3000");
    });
    parser.on("data", (line) => {
      console.log(`Received data from serial port: ${line}`);
      io.emit("jack", line );
    });
    process.stdin.resume();
    // Keep the script running until a SIGINT signal is received (i.e. the user presses ctrl+c)
    process.on("SIGINT", () => {
      console.log("Exiting...");
      process.exit();
    });
  }
});
