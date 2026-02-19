#ifndef WEB_CONFIG_HTML_H
#define WEB_CONFIG_HTML_H

static const char PAGE_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>SW20 Gauge Config</title>
<style>
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:system-ui,sans-serif;background:#1a1a1a;color:#e0e0e0;padding:12px;max-width:480px;margin:0 auto}
h1{font-size:1.3em;text-align:center;padding:12px 0;border-bottom:1px solid #444}
h2{font-size:1em;color:#aaa;padding:10px 0 6px;border-bottom:1px solid #333;margin-top:14px}
.f{display:flex;justify-content:space-between;align-items:center;padding:6px 0}
.f label{flex:1;font-size:0.9em}
.f input[type=number]{width:100px;padding:4px 6px;background:#2a2a2a;color:#fff;border:1px solid #555;border-radius:4px;font-size:0.9em}
.f input[type=checkbox]{width:20px;height:20px}
.btn{display:block;width:100%;padding:12px;margin:18px 0 8px;background:#2196F3;color:#fff;border:none;border-radius:6px;font-size:1em;cursor:pointer}
.btn:active{background:#1976D2}
.rst{background:#f44336}.rst:active{background:#c62828}
.msg{text-align:center;padding:8px;margin:8px 0;border-radius:4px;display:none}
.ok{background:#2e7d32;display:block}
.foot{text-align:center;color:#666;font-size:0.75em;padding:12px 0}
</style>
</head>
<body>
<h1>SW20 Gauge Config</h1>
<div id="msg" class="msg"></div>
<form method="POST" action="/save">

<h2>Simulation</h2>
<div class="f"><label>Simulated Oil Pressure</label><input type="checkbox" name="simData" %SIM_DATA%></div>
<div class="f"><label>Simulated Temperature</label><input type="checkbox" name="simTemp" %SIM_TEMP%></div>
<div class="f"><label>Simulated Headlight</label><input type="checkbox" name="simHL" %SIM_HL%></div>

<h2>Sensor Calibration</h2>
<div class="f"><label>Min Voltage (V)</label><input type="number" name="sensMinV" step="0.01" value="%SENS_MIN_V%"></div>
<div class="f"><label>Max Voltage (V)</label><input type="number" name="sensMaxV" step="0.01" value="%SENS_MAX_V%"></div>
<div class="f"><label>Max PSI</label><input type="number" name="sensMaxP" step="0.1" value="%SENS_MAX_P%"></div>
<div class="f"><label>Divider R1 (&Omega;)</label><input type="number" name="vdR1" step="1" value="%VD_R1%"></div>
<div class="f"><label>Divider R2 (&Omega;)</label><input type="number" name="vdR2" step="1" value="%VD_R2%"></div>

<h2>Safety Thresholds</h2>
<div class="f"><label>Oil Min Safe (PSI)</label><input type="number" name="oilSafe" step="0.1" value="%OIL_SAFE%"></div>
<div class="f"><label>Oil Min Warn (PSI)</label><input type="number" name="oilWarn" step="0.1" value="%OIL_WARN%"></div>
<div class="f"><label>Temp Warning (&deg;C)</label><input type="number" name="tempWarn" step="0.1" value="%TEMP_WARN%"></div>

<h2>Backlight</h2>
<div class="f"><label>Day Brightness (0-255)</label><input type="number" name="blDay" min="0" max="255" value="%BL_DAY%"></div>
<div class="f"><label>Night Brightness (0-255)</label><input type="number" name="blNight" min="0" max="255" value="%BL_NIGHT%"></div>
<div class="f"><label>Fade Duration (ms)</label><input type="number" name="blFade" min="0" max="5000" value="%BL_FADE%"></div>

<h2>Display</h2>
<div class="f"><label>EMA Smoothing (0.01-1.0)</label><input type="number" name="emaAlpha" step="0.01" min="0.01" max="1.0" value="%EMA_ALPHA%"></div>

<button class="btn" type="submit">Save &amp; Apply</button>
</form>
<form method="POST" action="/reset">
<button class="btn rst" type="submit" onclick="return confirm('Reset all settings to factory defaults?')">Reset to Defaults</button>
</form>
<p class="foot">SW20 Cluster Gauge &bull; 192.168.4.1</p>
<script>
var p=location.search;
if(p==='?saved=1'){var m=document.getElementById('msg');m.textContent='Settings saved!';m.className='msg ok';setTimeout(function(){m.style.display='none'},3000)}
if(p==='?reset=1'){var m=document.getElementById('msg');m.textContent='Defaults restored!';m.className='msg ok';setTimeout(function(){m.style.display='none'},3000)}
</script>
</body>
</html>
)rawliteral";

#endif // WEB_CONFIG_HTML_H
