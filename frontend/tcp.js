var net = require("net");
var host = "127.0.0.1";
var port = 6000;

var client = new net.Socket();
client.connect(port, host, function(){
    client.write("")
})