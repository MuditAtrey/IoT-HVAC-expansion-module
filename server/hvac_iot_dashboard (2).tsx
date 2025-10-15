import React, { useState, useEffect } from 'react';
import { Thermometer, Droplets, Database, Trash2, Download, Plus, Menu, X, RefreshCw, Clock } from 'lucide-react';

const HVACDashboard = () => {
  const MAX_RECORDS = 100;
  
  const [records, setRecords] = useState([
    { id: 1, timestamp: new Date().toISOString(), temperature: 22.5, humidity: 45 },
    { id: 2, timestamp: new Date(Date.now() - 300000).toISOString(), temperature: 22.8, humidity: 46 },
    { id: 3, timestamp: new Date(Date.now() - 600000).toISOString(), temperature: 23.1, humidity: 44 }
  ]);
  
  const [temperature, setTemperature] = useState('');
  const [humidity, setHumidity] = useState('');
  const [currentTemp, setCurrentTemp] = useState(22.5);
  const [currentHumidity, setCurrentHumidity] = useState(45);
  const [showMenu, setShowMenu] = useState(false);
  const [showInput, setShowInput] = useState(false);
  const [refreshInterval, setRefreshInterval] = useState(10);
  const [showIntervalModal, setShowIntervalModal] = useState(false);
  const [isAutoRefresh, setIsAutoRefresh] = useState(true);

  useEffect(() => {
    if (!isAutoRefresh) return;
    
    const interval = setInterval(() => {
      const newTemp = (20 + Math.random() * 10).toFixed(1);
      const newHum = (40 + Math.random() * 20).toFixed(1);
      addRecord(parseFloat(newTemp), parseFloat(newHum));
    }, refreshInterval * 1000);

    return () => clearInterval(interval);
  }, [records, refreshInterval, isAutoRefresh]);

  const addRecord = (temp, hum) => {
    const newRecord = {
      id: Date.now(),
      timestamp: new Date().toISOString(),
      temperature: temp,
      humidity: hum
    };

    setRecords(prev => {
      const updated = [newRecord, ...prev];
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
      setShowInput(false);
    }
  };

  const manualRefresh = () => {
    const newTemp = (20 + Math.random() * 10).toFixed(1);
    const newHum = (40 + Math.random() * 20).toFixed(1);
    addRecord(parseFloat(newTemp), parseFloat(newHum));
    setShowMenu(false);
  };

  const updateInterval = (newInterval) => {
    if (newInterval >= 1 && newInterval <= 60) {
      setRefreshInterval(newInterval);
    }
  };

  const clearAllRecords = () => {
    if (window.confirm('Are you sure you want to clear all records?')) {
      setRecords([]);
      setShowMenu(false);
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
    setShowMenu(false);
  };

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
    <div className="min-h-screen bg-gradient-to-br from-blue-50 to-indigo-100">
      {/* Mobile Header */}
      <div className="sticky top-0 z-50 bg-white shadow-md">
        <div className="flex items-center justify-between p-4">
          <div>
            <h1 className="text-lg sm:text-xl font-bold text-gray-800">HVAC Monitor</h1>
            <p className="text-xs text-gray-600 hidden sm:block">IoT System</p>
          </div>
          <button
            onClick={() => setShowMenu(!showMenu)}
            className="p-2 rounded-lg bg-gray-100 hover:bg-gray-200 transition-colors"
          >
            {showMenu ? <X className="w-6 h-6" /> : <Menu className="w-6 h-6" />}
          </button>
        </div>

        {/* Dropdown Menu */}
        {showMenu && (
          <div className="absolute top-full left-0 right-0 bg-white shadow-lg border-t">
            <button
              onClick={manualRefresh}
              className="w-full flex items-center gap-3 px-4 py-3 hover:bg-gray-50 transition-colors border-b"
            >
              <RefreshCw className="w-5 h-5 text-blue-600" />
              <span className="font-medium">Refresh Now</span>
            </button>
            <button
              onClick={() => {
                setShowIntervalModal(true);
                setShowMenu(false);
              }}
              className="w-full flex items-center justify-between px-4 py-3 hover:bg-gray-50 transition-colors border-b"
            >
              <div className="flex items-center gap-3">
                <Clock className="w-5 h-5 text-purple-600" />
                <span className="font-medium">Auto-Refresh Interval</span>
              </div>
              <span className="text-sm text-gray-600">{refreshInterval}s</span>
            </button>
            <button
              onClick={exportData}
              disabled={records.length === 0}
              className="w-full flex items-center gap-3 px-4 py-3 hover:bg-gray-50 transition-colors disabled:opacity-50 border-b"
            >
              <Download className="w-5 h-5 text-green-600" />
              <span className="font-medium">Export CSV</span>
            </button>
            <button
              onClick={clearAllRecords}
              disabled={records.length === 0}
              className="w-full flex items-center gap-3 px-4 py-3 hover:bg-gray-50 transition-colors disabled:opacity-50"
            >
              <Trash2 className="w-5 h-5 text-red-600" />
              <span className="font-medium">Clear All Records</span>
            </button>
          </div>
        )}
      </div>

      <div className="p-3 sm:p-6 pb-20">
        {/* Auto-Refresh Status Badge */}
        <div className="flex items-center justify-between mb-3">
          <div className="flex items-center gap-2 bg-white rounded-lg px-3 py-2 shadow-sm">
            <div className={`w-2 h-2 rounded-full ${isAutoRefresh ? 'bg-green-500 animate-pulse' : 'bg-gray-400'}`}></div>
            <span className="text-xs font-medium text-gray-700">
              {isAutoRefresh ? `Auto-refresh: ${refreshInterval}s` : 'Auto-refresh: OFF'}
            </span>
          </div>
          <button
            onClick={() => setIsAutoRefresh(!isAutoRefresh)}
            className={`px-3 py-2 rounded-lg text-xs font-medium transition-colors ${
              isAutoRefresh 
                ? 'bg-green-100 text-green-700 hover:bg-green-200' 
                : 'bg-gray-200 text-gray-700 hover:bg-gray-300'
            }`}
          >
            {isAutoRefresh ? 'ON' : 'OFF'}
          </button>
        </div>
        {/* Current Readings - Mobile Optimized */}
        <div className="grid grid-cols-2 gap-3 mb-4">
          <div className="bg-gradient-to-br from-red-500 to-orange-500 rounded-xl shadow-lg p-4 text-white">
            <div className="flex items-center justify-between mb-2">
              <Thermometer className="w-8 h-8 sm:w-10 sm:h-10" />
              <span className="text-xs bg-white bg-opacity-20 px-2 py-1 rounded-full">Live</span>
            </div>
            <div className="text-3xl sm:text-4xl font-bold mb-1">{currentTemp}°C</div>
            <div className="text-sm sm:text-base opacity-90 mb-2">Temperature</div>
            <div className={`text-xs font-semibold ${tempStatus.color} bg-white px-2 py-1 rounded inline-block`}>
              {tempStatus.status}
            </div>
          </div>

          <div className="bg-gradient-to-br from-blue-500 to-cyan-500 rounded-xl shadow-lg p-4 text-white">
            <div className="flex items-center justify-between mb-2">
              <Droplets className="w-8 h-8 sm:w-10 sm:h-10" />
              <span className="text-xs bg-white bg-opacity-20 px-2 py-1 rounded-full">Live</span>
            </div>
            <div className="text-3xl sm:text-4xl font-bold mb-1">{currentHumidity}%</div>
            <div className="text-sm sm:text-base opacity-90 mb-2">Humidity</div>
            <div className={`text-xs font-semibold ${humStatus.color} bg-white px-2 py-1 rounded inline-block`}>
              {humStatus.status}
            </div>
          </div>
        </div>

        {/* Records Table - Mobile Optimized */}
        <div className="bg-white rounded-xl shadow-lg p-3 sm:p-4">
          <div className="flex justify-between items-center mb-3">
            <div className="flex items-center gap-2">
              <Database className="w-5 h-5 text-gray-700" />
              <h2 className="text-base sm:text-lg font-bold text-gray-800">
                Records <span className="text-sm font-normal text-gray-600">({records.length}/{MAX_RECORDS})</span>
              </h2>
            </div>
          </div>
          
          <div className="overflow-x-auto">
            <div className="max-h-64 sm:max-h-96 overflow-y-auto">
              <table className="w-full text-sm">
                <thead className="bg-gray-100 sticky top-0">
                  <tr>
                    <th className="px-2 py-2 text-left text-xs font-semibold text-gray-700">Time</th>
                    <th className="px-2 py-2 text-left text-xs font-semibold text-gray-700">Temp</th>
                    <th className="px-2 py-2 text-left text-xs font-semibold text-gray-700">Hum</th>
                  </tr>
                </thead>
                <tbody>
                  {records.length === 0 ? (
                    <tr>
                      <td colSpan="3" className="px-2 py-6 text-center text-gray-500 text-xs">
                        No records yet
                      </td>
                    </tr>
                  ) : (
                    records.map((record) => (
                      <tr key={record.id} className="border-t hover:bg-gray-50">
                        <td className="px-2 py-2 text-xs text-gray-700">
                          {new Date(record.timestamp).toLocaleString([], { 
                            month: 'short', 
                            day: 'numeric', 
                            hour: '2-digit', 
                            minute: '2-digit' 
                          })}
                        </td>
                        <td className="px-2 py-2 text-xs font-medium text-red-600">
                          {record.temperature}°C
                        </td>
                        <td className="px-2 py-2 text-xs font-medium text-blue-600">
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

      {/* Floating Action Button */}
      <button
        onClick={() => setShowInput(!showInput)}
        className="fixed bottom-6 right-6 w-14 h-14 bg-blue-600 text-white rounded-full shadow-lg hover:bg-blue-700 transition-all flex items-center justify-center z-40"
      >
        {showInput ? <X className="w-6 h-6" /> : <Plus className="w-6 h-6" />}
      </button>

      {/* Input Modal - Mobile Optimized */}
      {showInput && (
        <div className="fixed inset-0 bg-black bg-opacity-50 z-50 flex items-end sm:items-center justify-center p-0 sm:p-4">
          <div className="bg-white w-full sm:max-w-md rounded-t-3xl sm:rounded-2xl p-6 shadow-2xl">
            <h3 className="text-xl font-bold text-gray-800 mb-4">Add Reading</h3>
            
            <div className="space-y-4">
              <div>
                <div className="text-sm font-medium text-gray-700 mb-2">Temperature (°C)</div>
                <input
                  type="number"
                  step="0.1"
                  value={temperature}
                  onChange={(e) => setTemperature(e.target.value)}
                  className="w-full px-4 py-3 border-2 border-gray-300 rounded-xl focus:ring-2 focus:ring-blue-500 focus:border-transparent text-lg"
                  placeholder="23.5"
                />
              </div>
              
              <div>
                <div className="text-sm font-medium text-gray-700 mb-2">Humidity (%)</div>
                <input
                  type="number"
                  step="0.1"
                  value={humidity}
                  onChange={(e) => setHumidity(e.target.value)}
                  className="w-full px-4 py-3 border-2 border-gray-300 rounded-xl focus:ring-2 focus:ring-blue-500 focus:border-transparent text-lg"
                  placeholder="45"
                />
              </div>

              <div className="flex gap-3 pt-2">
                <button
                  onClick={() => setShowInput(false)}
                  className="flex-1 px-4 py-3 bg-gray-200 text-gray-700 rounded-xl font-medium hover:bg-gray-300 transition-colors"
                >
                  Cancel
                </button>
                <button
                  onClick={handleSubmit}
                  className="flex-1 px-4 py-3 bg-blue-600 text-white rounded-xl font-medium hover:bg-blue-700 transition-colors"
                >
                  Add Reading
                </button>
              </div>
            </div>
          </div>
        </div>
      )}

      {/* Interval Settings Modal */}
      {showIntervalModal && (
        <div className="fixed inset-0 bg-black bg-opacity-50 z-50 flex items-end sm:items-center justify-center p-0 sm:p-4">
          <div className="bg-white w-full sm:max-w-md rounded-t-3xl sm:rounded-2xl p-6 shadow-2xl">
            <h3 className="text-xl font-bold text-gray-800 mb-2">Auto-Refresh Interval</h3>
            <p className="text-sm text-gray-600 mb-6">Set how often new readings are collected (1-60 seconds)</p>
            
            <div className="space-y-6">
              <div>
                <div className="flex items-center justify-between mb-3">
                  <span className="text-3xl font-bold text-blue-600">{refreshInterval}s</span>
                  <div className="flex gap-2">
                    <button
                      onClick={() => updateInterval(refreshInterval - 1)}
                      disabled={refreshInterval <= 1}
                      className="w-10 h-10 bg-gray-200 rounded-lg font-bold text-xl hover:bg-gray-300 disabled:opacity-50 disabled:cursor-not-allowed transition-colors"
                    >
                      −
                    </button>
                    <button
                      onClick={() => updateInterval(refreshInterval + 1)}
                      disabled={refreshInterval >= 60}
                      className="w-10 h-10 bg-gray-200 rounded-lg font-bold text-xl hover:bg-gray-300 disabled:opacity-50 disabled:cursor-not-allowed transition-colors"
                    >
                      +
                    </button>
                  </div>
                </div>
                
                <input
                  type="range"
                  min="1"
                  max="60"
                  value={refreshInterval}
                  onChange={(e) => updateInterval(parseInt(e.target.value))}
                  className="w-full h-2 bg-gray-200 rounded-lg appearance-none cursor-pointer"
                  style={{
                    background: `linear-gradient(to right, #3b82f6 0%, #3b82f6 ${((refreshInterval - 1) / 59) * 100}%, #e5e7eb ${((refreshInterval - 1) / 59) * 100}%, #e5e7eb 100%)`
                  }}
                />
                
                <div className="flex justify-between text-xs text-gray-500 mt-2">
                  <span>1s (Fast)</span>
                  <span>60s (Slow)</span>
                </div>
              </div>

              <div className="grid grid-cols-3 gap-2">
                {[5, 10, 15, 20, 30, 60].map(val => (
                  <button
                    key={val}
                    onClick={() => updateInterval(val)}
                    className={`px-3 py-2 rounded-lg text-sm font-medium transition-colors ${
                      refreshInterval === val
                        ? 'bg-blue-600 text-white'
                        : 'bg-gray-100 text-gray-700 hover:bg-gray-200'
                    }`}
                  >
                    {val}s
                  </button>
                ))}
              </div>

              <div className="flex gap-3 pt-2">
                <button
                  onClick={() => setShowIntervalModal(false)}
                  className="flex-1 px-4 py-3 bg-blue-600 text-white rounded-xl font-medium hover:bg-blue-700 transition-colors"
                >
                  Done
                </button>
              </div>
            </div>
          </div>
        </div>
      )}
    </div>
  );
};

export default HVACDashboard;