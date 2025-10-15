async function fetchLatest() {
  const res = await fetch('/data/all');
  if (!res.ok) {
    console.error('fetch failed', res.status);
    return;
  }
  const json = await res.json();
  if (!json.data || json.data.length === 0) {
    document.getElementById('outside-temp').innerText = '--';
    document.getElementById('outside-hum').innerText = '--';
    document.getElementById('indoor-temp').innerText = '--';
    document.getElementById('indoor-hum').innerText = '--';
    return;
  }
  
  // Find latest temperatures and humidity from both NodeMCUs
  let outdoorTemp = '--';
  let outdoorHum = '--';
  let indoorTemp = '--';
  let indoorHum = '--';
  
  for (const item of json.data) {
    if (item.type === 'encoder' && outdoorTemp === '--') {
      outdoorTemp = item.payload.temperature || '--';
      outdoorHum = item.payload.humidity || '--';
    } else if (item.type === 'indoor' && indoorTemp === '--') {
      indoorTemp = item.payload.temperature || '--';
      indoorHum = item.payload.humidity || '--';
    }
    if (outdoorTemp !== '--' && indoorTemp !== '--') break;
  }
  
  document.getElementById('outside-temp').innerText = outdoorTemp;
  document.getElementById('outside-hum').innerText = outdoorHum;
  document.getElementById('indoor-temp').innerText = indoorTemp;
  document.getElementById('indoor-hum').innerText = indoorHum;
}

function formatItem(item) {
  const p = document.createElement('div');
  const meta = `<div><strong>From:</strong> ${item.remote_addr}</div>`;
  const payload = `<pre>${JSON.stringify(item.payload, null, 2)}</pre>`;
  return meta + payload;
}

document.getElementById('refresh').addEventListener('click', fetchLatest);

let timer = null;
function startTimer() {
  const cb = document.getElementById('autorefresh').checked;
  const interval = Number(document.getElementById('interval').value) * 1000;
  if (!cb) {
    if (timer) { clearInterval(timer); timer = null; }
    return;
  }
  if (timer) clearInterval(timer);
  timer = setInterval(fetchLatest, Math.max(500, interval));
}

document.getElementById('autorefresh').addEventListener('change', startTimer);
document.getElementById('interval').addEventListener('change', startTimer);

startTimer();
fetchLatest();
