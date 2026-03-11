#pragma once
#include <pgmspace.h>

const char HTML[] PROGMEM = R"HTML(<!DOCTYPE html>
<html lang="de">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>RPICT Monitor</title>
<script src="https://cdn.jsdelivr.net/npm/chart.js@4.4.0/dist/chart.umd.min.js"></script>
<style>
:root{
  --bg:#07090f;--panel:#0d1220;--panel2:#111827;--border:#1a2840;
  --c1:#00d4ff;--c2:#ff6b35;--c3:#7c3aed;--c4:#10b981;
  --green:#10b981;--red:#ef4444;--yellow:#f59e0b;--amber:#f97316;
  --text:#dde6f0;--muted:#4a6080;--w:1px;
  --font:'Courier New',monospace;
}
*{box-sizing:border-box;margin:0;padding:0}
body{background:var(--bg);color:var(--text);font-family:var(--font);min-height:100vh}
.grid-bg{position:fixed;inset:0;
  background-image:linear-gradient(rgba(0,212,255,.025) 1px,transparent 1px),
    linear-gradient(90deg,rgba(0,212,255,.025) 1px,transparent 1px);
  background-size:36px 36px;pointer-events:none;z-index:0}
.wrap{position:relative;z-index:1;max-width:1280px;margin:0 auto;padding:14px}

/* ── Header ── */
header{display:flex;align-items:center;justify-content:space-between;
  border-bottom:var(--w) solid var(--border);padding-bottom:10px;margin-bottom:14px;flex-wrap:wrap;gap:8px}
.logo{font-size:1.2rem;font-weight:700;letter-spacing:.22em;color:var(--c1);
  text-shadow:0 0 20px rgba(0,212,255,.55)}
.logo em{color:var(--c2);font-style:normal}
.sbadge{font-size:.6rem;background:rgba(0,212,255,.08);border:var(--w) solid var(--c1);
  color:var(--c1);padding:2px 7px;border-radius:2px;margin-left:8px;letter-spacing:.1em}
.statusbar{display:flex;gap:12px;align-items:center;font-size:.7rem;flex-wrap:wrap}
.led{width:7px;height:7px;border-radius:50%;display:inline-block;margin-right:4px}
.led.on{background:var(--green);box-shadow:0 0 6px var(--green)}
.led.off{background:var(--red)}
.led.warn{background:var(--yellow);box-shadow:0 0 6px var(--yellow)}
.muted{color:var(--muted);font-size:.65rem}

/* ── Tabs ── */
.tabs{display:flex;gap:2px;margin-bottom:14px;flex-wrap:wrap}
.tab{padding:7px 16px;background:var(--panel);border:var(--w) solid var(--border);
  color:var(--muted);cursor:pointer;font-family:var(--font);font-size:.7rem;
  letter-spacing:.1em;text-transform:uppercase;transition:.15s;border-radius:3px}
.tab.active{background:var(--c1);color:#000;font-weight:700;border-color:var(--c1);
  box-shadow:0 0 14px rgba(0,212,255,.3)}
.tab:hover:not(.active){background:var(--border);color:var(--text)}
.page{display:none}.page.active{display:block}

/* ── Karten ── */
.card{background:var(--panel);border:var(--w) solid var(--border);border-radius:5px;padding:16px}
.card-hdr{font-size:.65rem;color:var(--c1);letter-spacing:.14em;text-transform:uppercase;
  margin-bottom:12px;padding-bottom:7px;border-bottom:var(--w) solid var(--border)}
.sg2{display:grid;grid-template-columns:1fr 1fr;gap:12px}
.sg3{display:grid;grid-template-columns:1fr 1fr 1fr;gap:12px}
@media(max-width:700px){.sg2,.sg3{grid-template-columns:1fr}}
.gap{margin-top:12px}

/* ── Dashboard: momentane Werte ── */
.phase-grid{display:grid;grid-template-columns:repeat(auto-fill,minmax(220px,1fr));gap:10px;margin-bottom:12px}
.phase-card{background:var(--panel);border:var(--w) solid var(--border);border-radius:5px;
  padding:14px;position:relative;overflow:hidden}
.phase-card::before{content:'';position:absolute;top:0;left:0;right:0;height:2px}
.pc0::before{background:linear-gradient(90deg,var(--c1),var(--c3))}
.pc1::before{background:linear-gradient(90deg,var(--c2),var(--amber))}
.pc2::before{background:linear-gradient(90deg,var(--c4),#6ee7b7)}
.pc3::before{background:linear-gradient(90deg,#a78bfa,var(--c3))}
.pc4::before{background:linear-gradient(90deg,var(--yellow),#fde68a)}
.pc5::before{background:linear-gradient(90deg,#f472b6,#fb7185)}
.pc6::before{background:linear-gradient(90deg,#34d399,var(--c1))}
.ph-title{font-size:.62rem;color:var(--muted);letter-spacing:.12em;text-transform:uppercase;margin-bottom:10px}
.ph-row{display:flex;align-items:baseline;justify-content:space-between;margin-bottom:6px}
.ph-label{font-size:.6rem;color:var(--muted);width:36px}
.ph-val{font-size:1.5rem;font-weight:700;line-height:1;flex:1;text-align:right}
.ph-unit{font-size:.62rem;color:var(--muted);margin-left:4px;width:28px}
.val-w{color:var(--c1)}
.val-a{color:var(--c2)}
.val-v{color:var(--c4)}

/* Vrms gesamt */
.vrms-bar{background:var(--panel);border:var(--w) solid var(--border);border-radius:5px;
  padding:12px 16px;display:flex;align-items:center;gap:20px;margin-bottom:12px;flex-wrap:wrap}
.vrms-lbl{font-size:.62rem;color:var(--muted);letter-spacing:.12em;text-transform:uppercase}
.vrms-val{font-size:2.2rem;font-weight:700;color:var(--c4)}
.vrms-unit{font-size:.75rem;color:var(--muted)}

/* ── Energie-Tabelle ── */
.energy-grid{display:grid;grid-template-columns:repeat(auto-fill,minmax(200px,1fr));gap:10px;margin-bottom:12px}
.en-card{background:var(--panel);border:var(--w) solid var(--border);border-radius:5px;padding:12px}
.en-title{font-size:.6rem;color:var(--muted);letter-spacing:.12em;text-transform:uppercase;margin-bottom:8px}
.en-row{display:flex;justify-content:space-between;align-items:baseline;padding:3px 0;
  border-bottom:var(--w) solid rgba(26,40,64,.6)}
.en-row:last-child{border-bottom:none}
.en-period{font-size:.6rem;color:var(--muted)}
.en-val{font-size:.95rem;font-weight:700}
.en-val.today{color:var(--c1)}
.en-val.month{color:var(--c2)}
.en-val.year{color:var(--c4)}
.en-unit{font-size:.58rem;color:var(--muted);margin-left:2px}

/* ── Charts ── */
.chart-card{background:var(--panel);border:var(--w) solid var(--border);border-radius:5px;
  padding:16px;margin-bottom:10px}
.chart-hdr{font-size:.65rem;color:var(--muted);letter-spacing:.13em;text-transform:uppercase;margin-bottom:12px}
.chart-wrap{position:relative;height:185px}
.ts{font-size:.62rem;color:var(--muted);text-align:right;margin-top:5px}

/* ── Settings ── */
.f{margin-bottom:9px}
.f label{display:block;font-size:.6rem;color:var(--muted);letter-spacing:.1em;
  text-transform:uppercase;margin-bottom:3px}
.f input,.f select{width:100%;background:#060910;border:var(--w) solid var(--border);
  border-radius:3px;padding:7px 8px;color:var(--text);font-family:var(--font);font-size:.8rem;
  outline:none;transition:.15s}
.f input:focus,.f select:focus{border-color:var(--c1);box-shadow:0 0 0 2px rgba(0,212,255,.07)}
.f select option{background:#0d1220}
.f.inline{display:flex;align-items:center;gap:8px}
.f.inline input[type=checkbox]{width:auto;margin:0}
.f.inline label{margin:0;white-space:nowrap}
.row2{display:grid;grid-template-columns:1fr 1fr;gap:8px}
.btn{padding:8px 18px;border:var(--w) solid var(--c1);border-radius:3px;background:transparent;
  color:var(--c1);cursor:pointer;font-family:var(--font);font-size:.7rem;
  letter-spacing:.1em;text-transform:uppercase;transition:.15s;margin-top:5px;margin-right:5px}
.btn:hover{background:var(--c1);color:#000}
.btn.red{border-color:var(--red);color:var(--red)}
.btn.red:hover{background:var(--red);color:#fff}
.btn.amber{border-color:var(--amber);color:var(--amber)}
.btn.amber:hover{background:var(--amber);color:#000}
.note{font-size:.62rem;color:var(--yellow);margin-top:5px;padding:5px 7px;
  background:rgba(245,158,11,.07);border-radius:3px;border-left:2px solid var(--yellow)}
.msg{font-size:.7rem;margin-top:7px;padding:7px;border-radius:3px;display:none}
.msg.ok{background:rgba(16,185,129,.1);border:var(--w) solid var(--green);color:var(--green)}
.msg.err{background:rgba(239,68,68,.1);border:var(--w) solid var(--red);color:var(--red)}

/* ── Raw ── */
.rawbox{background:#040710;border:var(--w) solid var(--border);border-radius:3px;
  padding:10px;font-size:.7rem;color:var(--c1);min-height:45px;
  word-break:break-all;white-space:pre-wrap;line-height:1.7}
</style>
</head>
<body>
<div class="grid-bg"></div>
<div class="wrap">

<header>
  <div>
    <span class="logo">RPICT<em>■</em>Monitor</span>
    <span class="sbadge" id="modelBadge">—</span>
    <span class="sbadge" id="boardBadge">—</span>
  </div>
  <div class="statusbar">
    <div><span class="led" id="ledW"></span><span id="lblW">WiFi</span></div>
    <div><span class="led" id="ledM"></span><span id="lblM">MQTT</span></div>
    <span class="muted" id="lblIp"></span>
    <span class="muted" id="lblHost"></span>
  </div>
</header>

<div class="tabs">
  <button class="tab active" onclick="showTab('dash')">◉ Live</button>
  <button class="tab" onclick="showTab('energy')">⚡ Energie</button>
  <button class="tab" onclick="showTab('charts')">▲ Charts</button>
  <button class="tab" onclick="showTab('settings')">⚙ Settings</button>
  <button class="tab" onclick="showTab('raw')">⌨ Raw</button>
</div>

<!-- ════════════════════ LIVE DASHBOARD ════════════════════ -->
<div id="p-dash" class="page active">
  <!-- Vrms gesamt -->
  <div class="vrms-bar">
    <div>
      <div class="vrms-lbl">Netzspannung</div>
      <span class="vrms-val" id="vrmsBig">—</span>
      <span class="vrms-unit">V</span>
    </div>
    <div style="flex:1"></div>
    <div class="muted" id="dashTs">—</div>
  </div>
  <!-- Phase-Karten: V, A, W -->
  <div class="phase-grid" id="phaseGrid"></div>
</div>

<!-- ════════════════════ ENERGIE ════════════════════ -->
<div id="p-energy" class="page">
  <div class="energy-grid" id="energyGrid"></div>
  <div style="margin-top:6px">
    <button class="btn amber" onclick="if(confirm('Energiezähler zurücksetzen?'))resetEnergy()">↺ Zähler zurücksetzen</button>
  </div>
</div>

<!-- ════════════════════ CHARTS ════════════════════ -->
<div id="p-charts" class="page">
  <div class="chart-card">
    <div class="chart-hdr">Wirkleistung [W]</div>
    <div class="chart-wrap"><canvas id="cPow"></canvas></div>
  </div>
  <div class="chart-card">
    <div class="chart-hdr">Stromstärke [A]</div>
    <div class="chart-wrap"><canvas id="cCur"></canvas></div>
  </div>
  <div class="chart-card">
    <div class="chart-hdr">Spannung Vrms [V]</div>
    <div class="chart-wrap"><canvas id="cVol"></canvas></div>
  </div>
  <div class="ts" id="tsLabel">—</div>
</div>

<!-- ════════════════════ SETTINGS ════════════════════ -->
<div id="p-settings" class="page">
  <div class="sg2">

    <!-- WiFi -->
    <div class="card">
      <div class="card-hdr">WiFi</div>
      <div class="f"><label>SSID</label><input id="wSSID" type="text"></div>
      <div class="f"><label>Passwort</label><input id="wPASS" type="password"></div>
      <div class="f"><label>Hostname</label><input id="hname" type="text" placeholder="rpict-monitor"></div>
    </div>

    <!-- IP-Konfiguration -->
    <div class="card">
      <div class="card-hdr">IP-Konfiguration</div>
      <div class="f inline">
        <input type="checkbox" id="sIP" onchange="toggleStaticIP()">
        <label for="sIP">Statische IP verwenden (sonst DHCP)</label>
      </div>
      <div id="staticFields" style="display:none;margin-top:8px">
        <div class="f"><label>IP-Adresse</label><input id="sAddr" type="text" placeholder="192.168.1.200"></div>
        <div class="row2">
          <div class="f"><label>Gateway</label><input id="sGW" type="text" placeholder="192.168.1.1"></div>
          <div class="f"><label>Subnetz</label><input id="sSN" type="text" placeholder="255.255.255.0"></div>
        </div>
        <div class="f"><label>DNS-Server</label><input id="sDNS" type="text" placeholder="8.8.8.8"></div>
      </div>
      <div class="note" id="ipNote" style="display:none">
        Statische IP: Gerät muss zur Netzwerk-Konfiguration passen.<br>
        Nach Speichern Gerät unter neuer IP aufrufen.
      </div>
    </div>

    <!-- MQTT -->
    <div class="card">
      <div class="card-hdr">MQTT Broker</div>
      <div class="row2">
        <div class="f"><label>Host / IP</label><input id="mHOST" type="text"></div>
        <div class="f"><label>Port</label><input id="mPORT" type="number" min="1" max="65535"></div>
      </div>
      <div class="row2">
        <div class="f"><label>User (opt.)</label><input id="mUSER" type="text"></div>
        <div class="f"><label>Passwort (opt.)</label><input id="mPASS" type="password"></div>
      </div>
      <div class="f"><label>Topic-Pfad</label><input id="mPATH" type="text"></div>
      <div class="f"><label>Publish-Intervall (ms)</label><input id="mINT" type="number" min="500" max="300000" step="500"></div>
    </div>

    <!-- Hardware -->
    <div class="card">
      <div class="card-hdr">Hardware / RPICT</div>
      <div class="row2">
        <div class="f"><label>RX-Pin (GPIO)</label><input id="rxPin" type="number" min="0" max="39"></div>
        <div class="f"><label>TX-Pin (GPIO)</label><input id="txPin" type="number" min="0" max="39"></div>
      </div>
      <div class="note" id="pinNote"></div>
      <div class="f" style="margin-top:10px">
        <label>RPICT-Modell</label>
        <select id="selModel" onchange="onModelChange()">
          <option value="0">RPICT3V1 — 3 Phasen</option>
          <option value="1">RPICT7V1 — 7 Phasen</option>
        </select>
      </div>
    </div>

    <!-- Web-Auth -->
    <div class="card">
      <div class="card-hdr">🔒 Web-Zugangsdaten</div>
      <div class="f"><label>Benutzername</label><input id="wbUSR" type="text" autocomplete="username"></div>
      <div class="f"><label>Neues Passwort</label><input id="wbPSS" type="password" autocomplete="new-password" placeholder="leer = unverändert"></div>
      <div class="f"><label>Bestätigung</label><input id="wbPSS2" type="password" autocomplete="new-password" placeholder="leer = unverändert"></div>
    </div>

    <!-- Aktionen -->
    <div class="card">
      <div class="card-hdr">Aktionen</div>
      <button class="btn" onclick="saveSettings()">💾 Speichern &amp; Neustart</button><br>
      <button class="btn red" onclick="if(confirm('Werkseinstellungen zurücksetzen?'))fetch('/reset')">↺ Factory Reset</button>
      <div id="sMsg" class="msg"></div>
    </div>

  </div>

  <!-- Kanal-Namen -->
  <div class="card gap">
    <div class="card-hdr">Kanal-Namen</div>
    <div class="sg3" id="chNameGrid"></div>
  </div>
</div>

<!-- ════════════════════ RAW ════════════════════ -->
<div id="p-raw" class="page">
  <div class="card">
    <div class="card-hdr">Letzter Serial-String (RPICT)</div>
    <div class="rawbox" id="rawLine">—</div>
    <div class="card-hdr gap">MQTT Topics (live)</div>
    <div class="rawbox" id="mqttTopics">—</div>
  </div>
</div>

</div><!-- /wrap -->
<script>
// ── Tab ──────────────────────────────────────────────────────────────────────
const TABS=['dash','energy','charts','settings','raw'];
function showTab(n){
  TABS.forEach((t,i)=>{
    document.querySelectorAll('.tab')[i].classList.toggle('active',t===n);
    document.getElementById('p-'+t).classList.toggle('active',t===n);
  });
}

// ── State ────────────────────────────────────────────────────────────────────
let names=[],model=0,np=3,ni=3,initialized=false;
let cPow=null,cCur=null,cVol=null;
const COLORS=['#00d4ff','#ff6b35','#10b981','#a78bfa','#f59e0b','#f472b6','#34d399',
              '#60a5fa','#fb923c','#6ee7b7','#c084fc','#fbbf24','#fb7185','#38bdf8','#a3e635'];
const PC=['pc0','pc1','pc2','pc3','pc4','pc5','pc6'];

// ── Phase-Grid aufbauen (Live-Tab) ───────────────────────────────────────────
function buildPhaseGrid(names,mdl){
  const g=document.getElementById('phaseGrid'); g.innerHTML='';
  const n=mdl===1?7:3;
  for(let i=0;i<n;i++){
    const d=document.createElement('div');
    d.className='phase-card '+PC[i%7];
    d.innerHTML=`
      <div class="ph-title">${names[i]||'Phase '+(i+1)}</div>
      <div class="ph-row">
        <span class="ph-label">W</span>
        <span class="ph-val val-w" id="pw${i}">—</span>
        <span class="ph-unit">W</span>
      </div>
      <div class="ph-row">
        <span class="ph-label">A</span>
        <span class="ph-val val-a" id="pa${i}">—</span>
        <span class="ph-unit">A</span>
      </div>`;
    g.appendChild(d);
  }
}

// ── Energie-Grid aufbauen ────────────────────────────────────────────────────
function buildEnergyGrid(names,mdl){
  const g=document.getElementById('energyGrid'); g.innerHTML='';
  const n=mdl===1?7:3;
  for(let i=0;i<n;i++){
    const d=document.createElement('div'); d.className='en-card';
    d.innerHTML=`
      <div class="en-title" style="border-left:2px solid ${COLORS[i]};padding-left:6px">${names[i]||'Phase '+(i+1)}</div>
      <div class="en-row"><span class="en-period">Heute</span>   <span><span class="en-val today" id="ed${i}">—</span><span class="en-unit">kWh</span></span></div>
      <div class="en-row"><span class="en-period">Monat</span>   <span><span class="en-val month" id="em${i}">—</span><span class="en-unit">kWh</span></span></div>
      <div class="en-row"><span class="en-period">Jahr</span>    <span><span class="en-val year"  id="ey${i}">—</span><span class="en-unit">kWh</span></span></div>`;
    g.appendChild(d);
  }
}

// ── Charts ───────────────────────────────────────────────────────────────────
function mkDS(label,color){return{label,data:[],borderColor:color,backgroundColor:color+'18',
  borderWidth:1.5,pointRadius:0,tension:.3};}
function mkChart(id,datasets,yLabel){
  return new Chart(document.getElementById(id),{type:'line',
    data:{labels:[],datasets},
    options:{responsive:true,maintainAspectRatio:false,animation:{duration:0},
      plugins:{legend:{labels:{color:'#4a6080',font:{family:'Courier New',size:9},boxWidth:10}}},
      scales:{
        x:{ticks:{color:'#1a2840',maxTicksLimit:8,font:{size:9}},grid:{color:'#0d1220'}},
        y:{ticks:{color:'#4a6080',font:{family:'Courier New',size:9}},
           grid:{color:'#1a2840'},title:{display:true,text:yLabel,color:'#4a6080',font:{size:9}}}
      }}});
}
function buildCharts(names,mdl){
  [cPow,cCur,cVol].forEach(c=>{if(c)c.destroy();});
  const n=mdl===1?7:3,ni2=mdl===1?7:3;
  cPow=mkChart('cPow', names.slice(0,n).map((nm,i)=>mkDS(nm,COLORS[i])),'W');
  cCur=mkChart('cCur', names.slice(n,n+ni2).map((nm,i)=>mkDS(nm,COLORS[n+i])),'A');
  cVol=mkChart('cVol',[mkDS(names[n+ni2]||'Vrms','#10b981')],'V');
}
function pushHistory(hist,mdl){
  if(!hist||!hist.length)return;
  const labels=hist.map(h=>new Date(h.ts).toLocaleTimeString());
  const n=mdl===1?7:3,ni2=mdl===1?7:3;
  [cPow,cCur,cVol].forEach(c=>{c.data.labels=labels;});
  cPow.data.datasets.forEach((ds,i)=>{ds.data=hist.map(h=>h.v[i]);});
  cCur.data.datasets.forEach((ds,i)=>{ds.data=hist.map(h=>h.v[n+i]);});
  cVol.data.datasets[0].data=hist.map(h=>h.v[n+ni2]);
  [cPow,cCur,cVol].forEach(c=>c.update());
}

// ── Settings UI ──────────────────────────────────────────────────────────────
function toggleStaticIP(){
  const on=document.getElementById('sIP').checked;
  document.getElementById('staticFields').style.display=on?'block':'none';
  document.getElementById('ipNote').style.display=on?'block':'none';
}

const MODEL_NOTE={
  0:'RPICT3V1: 3× Leistung + 3× Irms + Vrms = 7 Kanäle',
  1:'RPICT7V1: 7× Leistung + 7× Irms + Vrms = 15 Kanäle'
};
const BOARD_PIN_NOTE={
  'ESP32'   :'Standard: RX=16, TX=17 (UART2). Beliebige GPIOs möglich.',
  'ESP32-C3':'Standard: RX=4, TX=5 (UART1). GPIOs 19/20 nicht verwenden (USB-JTAG)!',
  'ESP8266' :'Standard: RX=14 (D5), TX=12 (D6). SoftwareSerial. GPIOs 0,2,15 meiden!'
};
function onModelChange(){
  const v=document.getElementById('selModel').value;
}
function buildChNameGrid(names,n){
  const g=document.getElementById('chNameGrid'); g.innerHTML='';
  const mdl=parseInt(document.getElementById('selModel').value)||0,np2=mdl===1?7:3,ni2=mdl===1?7:3;
  for(let i=0;i<n;i++){
    let grp=i<np2?'Leistung '+(i+1):(i<np2+ni2?'Irms '+(i-np2+1):'Vrms');
    const d=document.createElement('div');d.className='f';
    d.innerHTML=`<label>${grp}</label><input id="ch${i}" type="text" value="${names[i]||''}">`;
    g.appendChild(d);
  }
}
function loadSettingsUI(cfg,board){
  document.getElementById('wSSID').value=cfg.wSSID||'';
  document.getElementById('wPASS').value='';
  document.getElementById('hname').value=cfg.hname||'';
  document.getElementById('sIP').checked=!!cfg.sIP; toggleStaticIP();
  document.getElementById('sAddr').value=cfg.sAddr||'';
  document.getElementById('sGW').value=cfg.sGW||'';
  document.getElementById('sSN').value=cfg.sSN||'';
  document.getElementById('sDNS').value=cfg.sDNS||'';
  document.getElementById('rxPin').value=cfg.rxPin??16;
  document.getElementById('txPin').value=cfg.txPin??17;
  document.getElementById('mHOST').value=cfg.mHOST||'';
  document.getElementById('mPORT').value=cfg.mPORT||1883;
  document.getElementById('mUSER').value=cfg.mUSER||'';
  document.getElementById('mPASS').value='';
  document.getElementById('mPATH').value=cfg.mPATH||'';
  document.getElementById('mINT').value=cfg.mINT||5000;
  document.getElementById('selModel').value=cfg.model||0;
  document.getElementById('wbUSR').value=cfg.webUser||'';
  document.getElementById('wbPSS').value='';
  document.getElementById('wbPSS2').value='';
  if(board)document.getElementById('pinNote').textContent=BOARD_PIN_NOTE[board]||'';
  buildChNameGrid(cfg.chNames||[],cfg.model===1?15:7);
}
async function saveSettings(){
  const msg=document.getElementById('sMsg');
  const p1=document.getElementById('wbPSS').value,p2=document.getElementById('wbPSS2').value;
  if(p1!==p2){msg.className='msg err';msg.textContent='✗ Passwörter stimmen nicht überein';msg.style.display='block';return;}
  const mdl=parseInt(document.getElementById('selModel').value)||0;
  const n=mdl===1?15:7,chNames=[];
  for(let i=0;i<n;i++)chNames.push(document.getElementById('ch'+i)?.value||'CH'+(i+1));
  const body={
    wSSID:document.getElementById('wSSID').value,
    wPASS:document.getElementById('wPASS').value,
    hname:document.getElementById('hname').value,
    sIP:document.getElementById('sIP').checked,
    sAddr:document.getElementById('sAddr').value,
    sGW:document.getElementById('sGW').value,
    sSN:document.getElementById('sSN').value,
    sDNS:document.getElementById('sDNS').value,
    rxPin:parseInt(document.getElementById('rxPin').value)||16,
    txPin:parseInt(document.getElementById('txPin').value)||17,
    mHOST:document.getElementById('mHOST').value,
    mPORT:parseInt(document.getElementById('mPORT').value)||1883,
    mUSER:document.getElementById('mUSER').value,
    mPASS:document.getElementById('mPASS').value,
    mPATH:document.getElementById('mPATH').value,
    mINT:parseInt(document.getElementById('mINT').value)||5000,
    model:mdl,chNames,
    wbUSR:document.getElementById('wbUSR').value,
    wbPSS:p1
  };
  try{
    const r=await fetch('/api/config',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)});
    if(r.ok){msg.className='msg ok';msg.textContent='✓ Gespeichert – Neustart...';}
    else{msg.className='msg err';msg.textContent='✗ Fehler beim Speichern';}
  }catch(e){msg.className='msg err';msg.textContent='✗ '+e.message;}
  msg.style.display='block';
}
async function resetEnergy(){
  await fetch('/api/energy/reset',{method:'POST'});
}

// ── Poll ──────────────────────────────────────────────────────────────────────
async function poll(){
  try{
    const d=await(await fetch('/api/data')).json();

    // Status
    const led=(id,on)=>document.getElementById(id).className='led '+(on?'on':'off');
    led('ledW',d.wifi); led('ledM',d.mqtt);
    document.getElementById('lblW').textContent=d.wifi?'WiFi ✓':'WiFi ✗';
    document.getElementById('lblM').textContent=d.mqtt?'MQTT ✓':'MQTT –';
    document.getElementById('lblIp').textContent=d.ip||'';
    document.getElementById('lblHost').textContent=d.hostname?'['+d.hostname+']':'';
    document.getElementById('modelBadge').textContent=d.model===1?'RPICT7V1':'RPICT3V1';
    document.getElementById('boardBadge').textContent=d.board||'—';

    // Erstinitialisierung
    if(!initialized&&d.names&&d.names.length){
      names=d.names; model=d.model||0; np=model===1?7:3; ni=model===1?7:3;
      buildPhaseGrid(names,model);
      buildEnergyGrid(names,model);
      buildCharts(names,model);
      loadSettingsUI(d.cfg||{},d.board);
      initialized=true;
    }

    // Live-Werte
    if(d.valid&&d.values){
      const vrmsIdx=np+ni;
      const vrms=d.values[vrmsIdx];
      document.getElementById('vrmsBig').textContent=vrms?vrms.toFixed(1):'—';
      document.getElementById('dashTs').textContent='↻ '+new Date().toLocaleTimeString();
      for(let i=0;i<np;i++){
        const w=d.values[i], a=d.values[np+i];
        const eW=document.getElementById('pw'+i), eA=document.getElementById('pa'+i);
        if(eW)eW.textContent=w!=null?parseFloat(w).toFixed(1):'—';
        if(eA)eA.textContent=a!=null?(parseFloat(a)/1000).toFixed(3):'—'; // mA→A
      }
    }

    // Energie
    if(d.energy){
      d.energy.forEach((ph,i)=>{
        const ed=document.getElementById('ed'+i),em=document.getElementById('em'+i),ey=document.getElementById('ey'+i);
        if(ed)ed.textContent=ph.today!=null?ph.today.toFixed(3):'—';
        if(em)em.textContent=ph.month!=null?ph.month.toFixed(3):'—';
        if(ey)ey.textContent=ph.year!=null?ph.year.toFixed(3):'—';
      });
    }

    // Raw
    document.getElementById('rawLine').textContent=d.raw||'—';
    if(d.names&&d.cfg)
      document.getElementById('mqttTopics').textContent=
        d.names.map((n,i)=>`${d.cfg.mPATH||'RPICT'}/${n}  →  ${d.valid?parseFloat(d.values[i]).toFixed(2):'—'}`).join('\n');

    // Charts
    if(d.history)pushHistory(d.history,model);
    document.getElementById('tsLabel').textContent='Aktualisiert: '+new Date().toLocaleTimeString();

  }catch(e){console.warn('[poll]',e);}
}
poll(); setInterval(poll,2000);
</script>
</body>
</html>)HTML";
