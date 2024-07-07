import firebase_admin
from firebase_admin import credentials
from firebase_admin import db
import time

cred = credentials.Certificate('weatherstation_credentials.json')

firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://weatherstation-980f2-default-rtdb.europe-west1.firebasedatabase.app'
})

ref = db.reference('weather_station/')
hourly_weather_records_ref = ref.child('hourly_weather_records')
weekly_weather_records_ref = ref.child('weekly_weather_records')
# weather_records_ref.set({
#     'ali': {
#         'dob': '12-12-1997'
#     },
#     'rooh': {
#         'dob': '30-12-1978'
#     }
# })



def add_weather_record(temperature, humidity, pressure, aqi):
    # Get the current timestamp
    timestamp = str(int(time.time() * 1000))  # Unix timestamp in milliseconds

    hourly_weather_data = {
        "timestamp": timestamp,
        "hour": 2,
        "temperature": temperature,
        "humidity": humidity,
        "pressure": pressure,
        "aqi": aqi
    }

    weekly_weather_data = {
        "timestamp": timestamp,
        "day": 1,
        "temperature": temperature,
        "humidity": humidity,
        "pressure": pressure,
        "aqi": aqi
    }
    
    # Reference to the 'weather_station/records' path in the database
    # ref = db.reference('weather_station/records')

    # Store the weather data under the current timestamp
    # ref.child(timestamp).set(weather_data)
    hourly_weather_records_ref.push(hourly_weather_data)
    weekly_weather_records_ref.push(weekly_weather_data)

temperature = 25.73
humidity = 60
pressure = 1012
aqi = 42

i = 1
while i < 5:
    add_weather_record(temperature, humidity, pressure, aqi)
    i += 1

handle = db.reference('weather_station/records')
print(ref.get())