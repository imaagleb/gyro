#pragma once

const char INDEX_HTML[] PROGMEM = R"html(
<!DOCTYPE html>
<html lang="ru">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>MPU-6050 Live</title>
<style>
:root{--bg:#0d1117;--card:#161b22;--border:#30363d;--text:#c9d1d9;
--accent:#58a6ff;--green:#3fb950;--red:#f85149;--yellow:#d29922}
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:system-ui,sans-serif;background:var(--bg);color:var(--text);
padding:16px;max-width:800px;margin:0 auto}
h1{text-align:center;margin-bottom:12px;font-size:1.4rem;color:var(--accent)}
.status-bar{display:flex;gap:12px;justify-content:center;flex-wrap:wrap;
padding:8px;background:var(--card);border-radius:8px;margin-bottom:16px;font-size:.85rem}
.dot{width:10px;height:10px;border-radius:50%;display:inline-block;margin-right:4px}
.dot-ok{background:var(--green)}.dot-err{background:var(--red)}
.cards{display:grid;grid-template-columns:repeat(auto-fit,minmax(220px,1fr));gap:12px;margin-bottom:16px}
.card{background:var(--card);border:1px solid var(--border);border-radius:8px;padding:12px}
.card h3{font-size:.8rem;color:var(--accent);margin-bottom:8px;text-transform:uppercase}
.val{font-size:1.6rem;font-weight:700;font-variant-numeric:tabular-nums}
.val small{font-size:.7rem;color:#8b949e;font-weight:400}
.cube-wrap{display:flex;justify-content:center;perspective:300px;margin:20px 0;height:160px;align-items:center}
.cube{width:80px;height:80px;position:relative;transform-style:preserve-3d;transition:transform .08s}
.face{position:absolute;width:80px;height:80px;border:1px solid var(--accent);
background:rgba(88,166,255,.1);display:flex;align-items:center;justify-content:center;
font-size:.6rem;color:var(--accent)}
.face.front{transform:translateZ(40px)}.face.back{transform:translateZ(-40px) rotateY(180deg)}
.face.left{transform:translateX(-40px) rotateY(-90deg)}.face.right{transform:translateX(40px) rotateY(90deg)}
.face.top{transform:translateY(-40px) rotateX(90deg)}.face.bottom{transform:translateY(40px) rotateX(-90deg)}
.csv-section{background:var(--card);border:1px solid var(--border);border-radius:8px;padding:16px;margin-bottom:16px}
.csv-section h3{margin-bottom:10px;color:var(--accent);font-size:.95rem}
.row{display:flex;gap:8px;align-items:center;flex-wrap:wrap;margin-bottom:8px}
input[type="datetime-local"]{background:var(--bg);color:var(--text);border:1px solid var(--border);
border-radius:4px;padding:6px 10px;font-size:.85rem}
button{background:var(--accent);color:#fff;border:none;border-radius:4px;padding:6px 14px;
cursor:pointer;font-size:.85rem;transition:opacity .15s}
button:hover{opacity:.85}
button.ghost{background:transparent;border:1px solid var(--border);color:var(--text)}
.quick-btns{display:flex;gap:6px}
#log-info{font-size:.8rem;color:#8b949e;margin-top:6px}
</style>
</head>
<body>
<h1>MPU-6050 Gyro Position</h1>
<div class="status-bar">
  <span><span id="ws-dot" class="dot dot-err"></span> <span id="ws-status">WS Disconnected</span></span>
  <span>IP: <b id="ip">—</b></span>
  <span>Time: <b id="clock">—</b></span>
  <span>Logs: <b id="log-info-text">—</b></span>
</div>

<div class="cube-wrap">
  <div class="cube" id="cube">
    <div class="face front">F</div>
    <div class="face back">B</div>
    <div class="face left">L</div>
    <div class="face right">R</div>
    <div class="face top">T</div>
    <div class="face bottom">BTM</div>
  </div>
</div>

<div class="cards">
  <div class="card"><h3>Orientation</h3>
    <div class="val" id="v-roll">0.0<small>°</small></div>
    <div class="val" id="v-pitch">0.0<small>°</small></div>
    <div class="val" id="v-yaw">0.0<small>°</small></div>
  </div>
  <div class="card"><h3>Acceleration (g)</h3>
    <div>X: <span id="a-x">0.000</span></div>
    <div>Y: <span id="a-y">0.000</span></div>
    <div>Z: <span id="a-z">0.000</span></div>
  </div>
  <div class="card"><h3>Gyro (dps)</h3>
    <div>X: <span id="g-x">0.00</span></div>
    <div>Y: <span id="g-y">0.00</span></div>
    <div>Z: <span id="g-z">0.00</span></div>
  </div>
  <div class="card"><h3>Temp</h3>
    <div class="val" id="v-temp">25<small>°C</small></div>
  </div>
</div>

<div class="csv-section">
  <h3>📥 Export CSV</h3>
  <div class="row">
    <label>From: <input type="datetime-local" id="dt-from"></label>
    <label>To: <input type="datetime-local" id="dt-to"></label>
  </div>
  <div class="row">
    <div class="quick-btns">
      <button class="ghost" onclick="setRange(5)">5 min</button>
      <button class="ghost" onclick="setRange(15)">15 min</button>
      <button class="ghost" onclick="setRange(30)">30 min</button>
      <button class="ghost" onclick="setRange(60)">60 min</button>
    </div>
    <button onclick="downloadCSV()">Download CSV</button>
    <button class="ghost" onclick="sendZero()">⊘ Zero Yaw</button>
  </div>
  <div id="log-info">Select a time range, then click Download.</div>
</div>

<script>
const ws=new WebSocket("ws://"+location.host+":81");
let serverTime=0,logBytes=0,samples=0;

ws.onopen=()=>{document.getElementById("ws-dot").className="dot dot-ok";
  document.getElementById("ws-status").textContent="Connected (30 Hz)";};
ws.onclose=()=>{document.getElementById("ws-dot").className="dot dot-err";
  document.getElementById("ws-status").textContent="Disconnected";};
ws.onerror=()=>ws.close();
ws.onmessage=(e)=>{const d=JSON.parse(e.data);
  document.getElementById("v-roll").innerHTML=d.roll.toFixed(1)+"<small>&deg;</small>";
  document.getElementById("v-pitch").innerHTML=d.pitch.toFixed(1)+"<small>&deg;</small>";
  document.getElementById("v-yaw").innerHTML=d.yaw.toFixed(1)+"<small>&deg;</small>";
  document.getElementById("a-x").textContent=d.ax.toFixed(3);
  document.getElementById("a-y").textContent=d.ay.toFixed(3);
  document.getElementById("a-z").textContent=d.az.toFixed(3);
  document.getElementById("g-x").textContent=d.gx.toFixed(2);
  document.getElementById("g-y").textContent=d.gy.toFixed(2);
  document.getElementById("g-z").textContent=d.gz.toFixed(2);
  document.getElementById("v-temp").innerHTML=d.temp.toFixed(1)+"<small>&deg;C</small>";
  const c=document.getElementById("cube");
  c.style.transform=`rotateX(${-d.pitch}deg) rotateY(${d.yaw}deg) rotateZ(${d.roll}deg)`;
};

function setRange(min){
  const t=Date.now()-min*60000;
  document.getElementById("dt-from").value=new Date(t).toISOString().slice(0,16);
  document.getElementById("dt-to").value=new Date().toISOString().slice(0,16);
}

async function downloadCSV(){
  const from=document.getElementById("dt-from").value;
  const to=document.getElementById("dt-to").value;
  if(!from||!to){alert("Select from/to time!");return;}
  const tz=-new Date().getTimezoneOffset()*60;
  const url="/download?from="+new Date(from).getTime()+"&to="+new Date(to).getTime()+"&tz="+tz;
  window.location.href=url;
}

function sendZero(){ws.send("zero");}

async function fetchStatus(){
  try{const r=await fetch("/status");const j=await r.json();
    serverTime=j.server_time;logBytes=j.log_bytes;samples=j.samples;
    document.getElementById("ip").textContent=j.ip;
    document.getElementById("log-info-text").textContent=
      (logBytes/1024).toFixed(0)+" KB · "+samples+" samples";
    // set datetime-local defaults
    if(!document.getElementById("dt-from").value){
      setRange(5);
    }
  }catch(e){}
}
setInterval(fetchStatus,10000);
fetchStatus();
</script>
</body>
</html>)html";
