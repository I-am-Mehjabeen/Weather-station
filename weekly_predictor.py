import firebase_admin
from firebase_admin import credentials
from firebase_admin import db
import pandas as pd
import joblib
import matplotlib.pyplot as plt

file_path = 'training_data.csv'
model_file_path = 'weekly_trained_model.pkl'

cred = credentials.Certificate('weatherstation_credentials.json')

firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://weatherstation-980f2-default-rtdb.europe-west1.firebasedatabase.app'
})

ref = db.reference('weather_station/')
weekly_weather_records_ref = ref.child('weekly_weather_records')

def add_weather_record(timestamp, day, temperature, humidity, pressure, aqi):
    weekly_weather_data = {
        "timestamp": timestamp,
        "day": int(day),
        "temperature": float(temperature),
        "humidity": float(humidity),
        "pressure": float(pressure),
        "aqi": float(aqi)
    }
    weekly_weather_records_ref.push(weekly_weather_data)

try:
    data = pd.read_csv(file_path)

    # Convert Date_Time to datetime format
    data["Date_Time"] = pd.to_datetime(data["Date_Time"])

    # Set Date_Time as index
    data.set_index("Date_Time", inplace=True)

    # Handle missing values (e.g., forward fill, backward fill, drop)
    data.ffill(inplace=True)
    data.bfill(inplace=True)

    # Load the model from the file
    model_fit = joblib.load(model_file_path)

    # Forecasting for the next 7 days (168 hours)
    forecast_next_week = model_fit.get_forecast(steps=7*24)
    forecast_next_week_mean = forecast_next_week.predicted_mean

    # Set the index for the forecast to a proper DatetimeIndex
    forecast_next_week_mean.index = pd.date_range(start=data.index[-1] + pd.Timedelta(hours=1), periods=7*24, freq='H')

    # Convert forecasted hourly temperatures to daily averages
    forecast_next_week_mean_daily = forecast_next_week_mean.resample("D").mean().iloc[1:]

    print("\nForecasted Average Daily Temperatures for the Next 7 Days:")
    ps = pd.Series(forecast_next_week_mean_daily)
    print(ps)
    length = sum(1 for _ in ps.items())
    if length == 7:
        weekly_weather_records_ref.delete()

    current_data = data.iloc[-1]

    for index, value in ps.items():
        timestamp = pd.Timestamp(index)
        unix_timestamp_ms = int(timestamp.timestamp() * 1000)
        add_weather_record(unix_timestamp_ms, timestamp.dayofweek, value, current_data['Rel_Hum'], current_data['PresskPa'], current_data['AQI'])

except Exception as e:
    print(e)
