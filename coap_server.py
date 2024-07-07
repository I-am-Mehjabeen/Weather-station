import aiocoap.resource as resource
import aiocoap
import asyncio
import firebase_admin
from firebase_admin import credentials
from firebase_admin import db
import json
import csv
import datetime
import time
import atexit
import os
import sys

# Define the path for the PID file
pid_file = '/tmp/coap_server.pid'

def remove_pid_file():
    if os.path.exists(pid_file):
        os.remove(pid_file)

# Check if the PID file already exists
if os.path.exists(pid_file):
    print(f"Script is already running. PID file {pid_file} exists.")
    sys.exit()

# Write the current process ID to the PID file
with open(pid_file, 'w') as f:
    f.write(str(os.getpid()))

# Ensure the PID file is removed when the script exits
atexit.register(remove_pid_file)

class CoAPResource(resource.Resource):
    cred = credentials.Certificate('weatherstation_credentials.json')
    firebase_admin.initialize_app(cred, {
        'databaseURL': 'https://weatherstation-980f2-default-rtdb.europe-west1.firebasedatabase.app'
    })

    def __init__(self):
        super().__init__()
        ref = db.reference('weather_station/')
        self.current_weather_ref = ref.child('current_weather')
        self.csv_filename = 'training_data.csv'
        self.last_hour = datetime.datetime.now().hour

    async def render_put(self, request):
        payload = request.payload.decode('ascii')
        print('Payload: %s' % payload)
        data = json.loads(payload)
        record = {}
        record['Date_Time'] = datetime.datetime.now().strftime('%m/%d/%Y %H:%M')
        record['TempC'] = data['temperature']
        record['Rel_Hum'] = 54
        record['PresskPa'] = 101.80
        record['PM1_0'] = 0.31
        record['PM2_5'] = 2.10
        record['PM4'] = 2.50
        record['PM10'] = 0.50
        record['AQI'] = 22

        current_hour = datetime.datetime.now().hour
        if current_hour != self.last_hour:
            self.append_record_to_file(record)
            self.last_hour = current_hour
        
        self.current_weather_ref.set({
            'temperature': data['temperature'],
            'humidity': 54,
            'pressure': 101.80,
            'aqi': 22
        })

        return aiocoap.Message(code=aiocoap.CHANGED, payload="")
    
    def append_record_to_file(self, record):
        # Write records to CSV
        csv_columns = ['Date_Time', 'TempC', 'Rel_Hum', 'PresskPa', 'PM1_0', 'PM2_5', 'PM4', 'PM10', 'AQI']
        with open(self.csv_filename, 'a', newline='') as csvfile:
            writer = csv.DictWriter(csvfile, fieldnames=csv_columns)
            writer.writerow(record)


def main():
    # Resource tree creation
    root = resource.Site()
    root.add_resource(['storedata'], CoAPResource())

    asyncio.Task(aiocoap.Context.create_server_context(root, bind=('::', 5683)))

    asyncio.get_event_loop().run_forever()

if __name__ == "__main__":
    main()