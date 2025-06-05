const BACKEND_URL = 'http://172.20.10.5:5000'; 

let map = L.map('map').setView([-5.233974437724341, 119.50056154246455], 20); 
L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
    attribution: '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors'
}).addTo(map);

let markers = {}; 


function getValidValue(value) {
    const invalidValues = [-1, -999, 2147483647]; 

    if (value === null || typeof value === 'undefined' || isNaN(value) || invalidValues.includes(value)) {
        return 'N/A';
    }
    return value;
}

function updateMapAndSensors() {
    fetch(`${BACKEND_URL}/get_sensor_data`)
        .then(response => {
            if (!response.ok) {
                throw new Error(`HTTP error! status: ${response.status}`);
            }
            return response.json();
        })
        .then(data => {
            console.log("Received data from backend:", data);

            for (let deviceId in markers) {
                if (!(deviceId in data)) {
                    map.removeLayer(markers[deviceId]);
                    delete markers[deviceId];
                }
            }

            for (let deviceId in data) {
                const deviceData = data[deviceId];
                const lat = deviceData.latitude;
                const lon = deviceData.longitude;

                if (deviceId === 'plant_area_1') {
                    displaySensorInfo(deviceData);
                }

                if (lat !== -1 && lon !== -1) {
                    const popupContent = `
                        <strong>Device ID: ${deviceId}</strong><br>
                        Latitude: ${getValidValue(lat)}<br>
                        Longitude: ${getValidValue(lon)}<br>
                        <hr>
                        <strong>Sensor 1:</strong><br>
                        Suhu Udara: ${getValidValue(deviceData.sensor1?.temperature_air)} °C<br>
                        Kelembaban Udara: ${getValidValue(deviceData.sensor1?.humidity_air)} %<br>
                        Suhu Tanah: ${getValidValue(deviceData.sensor1?.temperature_soil)} °C<br>
                        Kelembaban Tanah: ${getValidValue(deviceData.sensor1?.moisture_soil)} %<br>
                        Status Pompa: ${getValidValue(deviceData.sensor1?.pump_status)}<br>
                        <hr>
                        <strong>Sensor 2:</strong><br>
                        Suhu Udara: ${getValidValue(deviceData.sensor2?.temperature_air)} °C<br>
                        Kelembaban Udara: ${getValidValue(deviceData.sensor2?.humidity_air)} %<br>
                        Suhu Tanah: ${getValidValue(deviceData.sensor2?.temperature_soil)} °C<br>
                        Kelembaban Tanah: ${getValidValue(deviceData.sensor2?.moisture_soil)} %<br>
                        Status Pompa: ${getValidValue(deviceData.sensor2?.pump_status)}<br>
                    `;

                    if (markers[deviceId]) {
                        markers[deviceId].setLatLng([lat, lon]);
                        markers[deviceId].setPopupContent(popupContent);
                    } else {
                        markers[deviceId] = L.marker([lat, lon]).addTo(map)
                            .bindPopup(popupContent);
                        
                        markers[deviceId].on('click', function() {
                            displaySensorInfo(deviceData);
                        });
                    }
                } else {
                    console.warn(`GPS data invalid for device ${deviceId}. Not showing on map.`);
                    if (markers[deviceId]) {
                        map.removeLayer(markers[deviceId]);
                        delete markers[deviceId];
                    }
                }
            }
        })
        .catch(error => {
            console.error('Error fetching sensor data:', error);
        });
}

function displaySensorInfo(data) {
    if (!data) {
        console.error("No data provided to displaySensorInfo.");
        return;
    }

    document.getElementById('device-id').innerText = data.device_id ?? 'N/A';
    document.getElementById('lat').innerText = getValidValue(data.latitude);
    document.getElementById('lon').innerText = getValidValue(data.longitude);

    if (data.sensor1) {
        document.getElementById('humi-air1').innerText = getValidValue(data.sensor1.humidity_air);
        document.getElementById('temp-air1').innerText = getValidValue(data.sensor1.temperature_air);
        document.getElementById('temp-soil1').innerText = getValidValue(data.sensor1.temperature_soil);
        document.getElementById('moisture-soil1').innerText = getValidValue(data.sensor1.moisture_soil);
        // document.getElementById('pump-status1').innerText = getValidValue(data.sensor1.pump_status);
    } else {
        document.getElementById('humi-air1').innerText = 'N/A';
        document.getElementById('temp-air1').innerText = 'N/A';
        document.getElementById('temp-soil1').innerText = 'N/A';
        document.getElementById('moisture-soil1').innerText = 'N/A';
        // document.getElementById('pump-status1').innerText = 'N/A';
    }

    if (data.sensor2) {
        document.getElementById('humi-air2').innerText = getValidValue(data.sensor2.humidity_air);
        document.getElementById('temp-air2').innerText = getValidValue(data.sensor2.temperature_air);
        document.getElementById('temp-soil2').innerText = getValidValue(data.sensor2.temperature_soil);
        document.getElementById('moisture-soil2').innerText = getValidValue(data.sensor2.moisture_soil);
        // document.getElementById('pump-status2').innerText = getValidValue(data.sensor2.pump_status);
    } else {
        document.getElementById('humi-air2').innerText = 'N/A';
        document.getElementById('temp-air2').innerText = 'N/A';
        document.getElementById('temp-soil2').innerText = 'N/A';
        document.getElementById('moisture-soil2').innerText = 'N/A';
        // document.getElementById('pump-status2').innerText = 'N/A';
    }
}

updateMapAndSensors();

setInterval(updateMapAndSensors, 5000);