// This file is required by the index.html file and will
// be executed in the renderer process for that window.
// All of the Node.js APIs are available in this process.
var map = new BMap.Map("container");
// 创建地图实例  
var point = new BMap.Point(116.404, 39.915);
// 创建点坐标  
map.centerAndZoom(point, 15);