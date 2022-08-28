const app = require("https-localhost")()
const express = require("express");

app.use(function(req, res, next) {
    res.header("Access-Control-Allow-Origin", "*"); // update to match the domain you will make the request from
    res.header("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
    next();
});
// app is an express app, do what you usually do with express
app.use("/", express.static("./build"));
app.listen(6766)