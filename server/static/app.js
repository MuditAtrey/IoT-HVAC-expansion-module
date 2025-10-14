async function fetchLatest() {
  const res = await fetch('/data/all');
  if (!res.ok) {
    console.error('fetch failed', res.status);
    return;
  }
  const json = await res.json();
  const recent = document.getElementById('recent');
  recent.innerHTML = '';
  if (!json.data || json.data.length === 0) {
    recent.innerText = '(no data)';
    document.getElementById('latest').innerText = '(no data yet)';
    return;
  }
  document.getElementById('latest').innerHTML = formatItem(json.data[0]);
  json.data.forEach(item => {
    const div = document.createElement('div');
    div.className = 'item';
    div.innerHTML = formatItem(item);
    recent.appendChild(div);
  });
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
