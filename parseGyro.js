// parseGyro.js — парсер строк гироскопа для Max for Live
// Формат входной строки: "X:12.34 Y:-5.67 Z:0.00"
// Выход: 3 float (X, Y, Z) на отдельныхخرجات

inlets = 1;
outlets = 3;

var buf = "";

function msg_string(s) {
  buf += s;
  var lines = buf.split("\n");
  buf = lines.pop();
  for (var i = 0; i < lines.length; i++) {
    parseLine(lines[i]);
  }
}

function parseLine(line) {
  var m = line.match(/X:([-\d.]+)\s+Y:([-\d.]+)\s+Z:([-\d.]+)/);
  if (m) {
    outlet(0, parseFloat(m[1]));
    outlet(1, parseFloat(m[2]));
    outlet(2, parseFloat(m[3]));
  }
}