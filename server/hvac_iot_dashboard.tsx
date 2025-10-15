import React, { useState, useEffect } from 'react';
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, Legend, ResponsiveContainer } from 'recharts';
import { Thermometer, Droplets, Database, Trash2, Download } from 'lucide-react';

const HVACDashboard = () => {
  const MAX_RECORDS = 150;
  
  // Initialize state with sample data
  const [records, setRecords] = useState([
    { id: 1, timestamp: new Date().toISOString(), temperature: 22.5, humidity: 45 },
    { id: 2, timestamp: new Date(Date.now() - 300000).toISOString(), temperature: 22.8, humidity: 46 },
    { id: 3, timestamp: new Date(Date.now() - 600000).toISOString(), temperature: 23.1, humidity: 44 }
  ]);
  
  const [temperature, setTemperature] = useState('');
  const [humidity, setHumidity] = useState('');
  const [currentTemp, setCurrentTemp] = useState(22.5);
  const [currentHumidity, setCurrentHumidity] = useState(45);

  // Auto-generate sample data every 10 seconds (simulating IoT device)
  useEffect(() => {
    const interval = setInterval(() => {
      const newTemp = (20 + Math.random() * 10).toFixed(1);
      const newHum = (40 + Math.random() * 20).toFixed(1);
      addRecord(parseFloat(newTemp), parseFloat(newHum));
    }, 10000);

    return () => clearInterval(interval);
  }, [records]);

  const addRecord = (temp, hum) => {
    const newRecord = {
      id: Date.now(),
      timestamp: new Date().toISOString(),
      temperature: temp,
      humidity: hum
    };

    setRecords(prev => {
      const updated = [newRecord, ...prev];
      // Keep only last 150 records
      return updated.slice(0, MAX_RECORDS);
    });

    setCurrentTemp(temp);
    setCurrentHumidity(hum);
  };

  const handleSubmit = () => {
    if (temperature && humidity) {
      addRecord(parseFloat(temperature), parseFloat(humidity));
      setTemperature('');
      setHumidity('');
    }
  };

  const clearAllRecords = () => {
    if (window.confirm('Are you sure you want to clear all records?')) {
      setRecords([]);
    }
  };

  const exportData = () => {
    const csv = [
      ['Timestamp', 'Temperature (°C)', 'Humidity (%)'],
      ...records.map(r => [
        new Date(r.timestamp).toLocaleString(),
        r.temperature,
        r.humidity
      ])
    ].map(row => row.join(',')).join('\n');

    const blob = new Blob([csv], { type: 'text/csv' });
    const url = window.URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = `hvac_data_${new Date().toISOString().split('T')[0]}.csv`;
    a.click();
  };

  // Prepare chart data (last 20 records for visibility)
  const chartData = records.slice(0, 20).reverse().map(r => ({
    time: new Date(r.timestamp).toLocaleTimeString(),
    temperature: r.temperature,
    humidity: r.humidity
  }));

  const getTemperatureStatus = (temp) => {
    if (temp < 18) return { status: 'Too Cold', color: 'text-blue-600' };
    if (temp > 26) return { status: 'Too Hot', color: 'text-red-600' };
    return { status: 'Optimal', color: 'text-green-600' };
  };

  const getHumidityStatus = (hum) => {
    if (hum < 30) return { status: 'Too Dry', color: 'text-orange-600' };
    if (hum > 60) return { status: 'Too Humid', color: 'text-blue-600' };
    return { status: 'Optimal', color: 'text-green-600' };
  };

  const tempStatus = getTemperatureStatus(currentTemp);
  const humStatus = getHumidityStatus(currentHumidity);

  return (
    <div className="min-h-screen bg-gradient-to-br from-blue-50 to-indigo-100 p-6">
      <div className="max-w-7xl mx-auto">
        {/* Header */}
        <div className="bg-white rounded-lg shadow-lg p-6 mb-6">
          <h1 className="text-3xl font-bold text-gray-800 mb-2">HVAC IoT Monitoring System</h1>
          <p className="text-gray-600">Real-time temperature and humidity monitoring</p>
        </div>

        {/* Current Readings */}
        <div className="grid md:grid-cols-2 gap-6 mb-6">
          <div className="bg-gradient-to-br from-red-500 to-orange-500 rounded-lg shadow-lg p-6 text-white">
            <div className="flex items-center justify-between mb-4">
              <Thermometer className="w-12 h-12" />
              <span className="text-sm bg-white bg-opacity-20 px-3 py-1 rounded-full">
                Live
              </span>
            </div>
            <div className="text-5xl font-bold mb-2">{currentTemp}°C</div>
            <div className="text-xl opacity-90">Temperature</div>
            <div className={`mt-3 font-semibold ${tempStatus.color} bg-white px-3 py-1 rounded inline-block`}>
              {tempStatus.status}
            </div>
          </div>

          <div className="bg-gradient-to-br from-blue-500 to-cyan-500 rounded-lg shadow-lg p-6 text-white">
            <div className="flex items-center justify-between mb-4">
              <Droplets className="w-12 h-12" />
              <span className="text-sm bg-white bg-opacity-20 px-3 py-1 rounded-full">
                Live
              </span>
            </div>
            <div className="text-5xl font-bold mb-2">{currentHumidity}%</div>
            <div className="text-xl opacity-90">Humidity</div>
            <div className={`mt-3 font-semibold ${humStatus.color} bg-white px-3 py-1 rounded inline-block`}>
              {humStatus.status}
            </div>
          </div>
        </div>

        {/* Data Input Form */}
        <div className="bg-white rounded-lg shadow-lg p-6 mb-6">
          <h2 className="text-xl font-bold text-gray-800 mb-4">Manual Data Entry</h2>
          <div className="flex flex-wrap gap-4">
            <div className="flex-1 min-w-[200px]">
              <div className="block text-sm font-medium text-gray-700 mb-2">
                Temperature (°C)
              </div>
              <input
                type="number"
                step="0.1"
                value={temperature}
                onChange={(e) => setTemperature(e.target.value)}
                className="w-full px-4 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-transparent"
                placeholder="e.g., 23.5"
              />
            </div>
            <div className="flex-1 min-w-[200px]">
              <div className="block text-sm font-medium text-gray-700 mb-2">
                Humidity (%)
              </div>
              <input
                type="number"
                step="0.1"
                value={humidity}
                onChange={(e) => setHumidity(e.target.value)}
                className="w-full px-4 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-transparent"
                placeholder="e.g., 45"
              />
            </div>
            <div className="flex items-end">
              <button
                onClick={handleSubmit}
                className="px-6 py-2 bg-blue-600 text-white rounded-lg hover:bg-blue-700 transition-colors font-medium"
              >
                Add Reading
              </button>
            </div>
          </div>
        </div>

        {/* Chart */}
        <div className="bg-white rounded-lg shadow-lg p-6 mb-6">
          <h2 className="text-xl font-bold text-gray-800 mb-4">Trend Analysis (Last 20 Readings)</h2>
          <ResponsiveContainer width="100%" height={300}>
            <LineChart data={chartData}>
              <CartesianGrid strokeDasharray="3 3" />
              <XAxis dataKey="time" />
              <YAxis yAxisId="left" />
              <YAxis yAxisId="right" orientation="right" />
              <Tooltip />
              <Legend />
              <Line
                yAxisId="left"
                type="monotone"
                dataKey="temperature"
                stroke="#ef4444"
                strokeWidth={2}
                name="Temperature (°C)"
              />
              <Line
                yAxisId="right"
                type="monotone"
                dataKey="humidity"
                stroke="#3b82f6"
                strokeWidth={2}
                name="Humidity (%)"
              />
            </LineChart>
          </ResponsiveContainer>
        </div>

        {/* Records Table */}
        <div className="bg-white rounded-lg shadow-lg p-6">
          <div className="flex justify-between items-center mb-4">
            <div className="flex items-center gap-2">
              <Database className="w-6 h-6 text-gray-700" />
              <h2 className="text-xl font-bold text-gray-800">
                Historical Records ({records.length}/{MAX_RECORDS})
              </h2>
            </div>
            <div className="flex gap-2">
              <button
                onClick={exportData}
                disabled={records.length === 0}
                className="flex items-center gap-2 px-4 py-2 bg-green-600 text-white rounded-lg hover:bg-green-700 transition-colors disabled:bg-gray-400"
              >
                <Download className="w-4 h-4" />
                Export CSV
              </button>
              <button
                onClick={clearAllRecords}
                disabled={records.length === 0}
                className="flex items-center gap-2 px-4 py-2 bg-red-600 text-white rounded-lg hover:bg-red-700 transition-colors disabled:bg-gray-400"
              >
                <Trash2 className="w-4 h-4" />
                Clear All
              </button>
            </div>
          </div>
          
          <div className="overflow-x-auto max-h-96 overflow-y-auto">
            <table className="w-full">
              <thead className="bg-gray-100 sticky top-0">
                <tr>
                  <th className="px-4 py-3 text-left text-sm font-semibold text-gray-700">Timestamp</th>
                  <th className="px-4 py-3 text-left text-sm font-semibold text-gray-700">Temperature (°C)</th>
                  <th className="px-4 py-3 text-left text-sm font-semibold text-gray-700">Humidity (%)</th>
                </tr>
              </thead>
              <tbody>
                {records.length === 0 ? (
                  <tr>
                    <td colSpan="3" className="px-4 py-8 text-center text-gray-500">
                      No records yet. Add your first reading above.
                    </td>
                  </tr>
                ) : (
                  records.map((record) => (
                    <tr key={record.id} className="border-t hover:bg-gray-50">
                      <td className="px-4 py-3 text-sm text-gray-700">
                        {new Date(record.timestamp).toLocaleString()}
                      </td>
                      <td className="px-4 py-3 text-sm font-medium text-red-600">
                        {record.temperature}°C
                      </td>
                      <td className="px-4 py-3 text-sm font-medium text-blue-600">
                        {record.humidity}%
                      </td>
                    </tr>
                  ))
                )}
              </tbody>
            </table>
          </div>
        </div>
      </div>
    </div>
  );
};

export default HVACDashboard;