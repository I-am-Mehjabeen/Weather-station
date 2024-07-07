import firebase_admin
from firebase_admin import credentials
from firebase_admin import db
import pandas as pd
import joblib
import pprint


file_path = 'training_data.csv'
model_file_path = 'hourly_trained_model.pkl'

cred = credentials.Certificate('weatherstation_credentials.json')

firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://weatherstation-980f2-default-rtdb.europe-west1.firebasedatabase.app'
})

ref = db.reference('weather_station/')
hourly_weather_records_ref = ref.child('hourly_weather_records')

def add_weather_record(timestamp, hour, temperature, humidity, pressure, aqi):
    hourly_weather_data = {
        "timestamp": timestamp,
        "hour": int(hour),
        "temperature": float(temperature),
        "humidity": float(humidity),
        "pressure": float(pressure),
        "aqi": float(aqi)
    }
    hourly_weather_records_ref.push(hourly_weather_data)
    


try:
    data = pd.read_csv(file_path)

    # Convert Date_Time to datetime format
    data["Date_Time"] = pd.to_datetime(data["Date_Time"])
    # Set Date_Time as index (if it's not already)
    data.set_index("Date_Time", inplace=True)

    # Handle missing values (if any)
    data.dropna(inplace=True)  # Example: dropping rows with missing values

    # Optionally, create time features like hour, day of week, etc.
    # data["Hour"] = data.index.hour
    # data["DayOfWeek"] = data.index.dayofweek
    # Load the model from the file
    model_fit = joblib.load(model_file_path)

    # Forecasting for the next 24 hours (hourly forecast)
    forecast_next_hours = model_fit.get_forecast(steps=24)

    # Example: Forecasting for the next day (daily forecast)
    forecast_next_day = model_fit.get_forecast(steps=2 * 24)

    # Example: Splitting data into train and test sets
    # train_data = data["TempC"].iloc[:-24]  # Exclude last 24 hours for testing
    # test_data = data["TempC"].iloc[-24:]  # Last 24 hours for testing
    current_data = data.iloc[-1]

    # Compare forecasted values with actual test data
    forecasted_values = forecast_next_hours.predicted_mean
    # actual_values = test_data.values

    # df = pd.DataFrame(forecasted_values)
    # df['datetime'] = pd.to_datetime(df['datetime'])
    # print(pd.DataFrame(forecasted_values).predicted_mean)

    ps = pd.Series(forecasted_values)
    # print(ps.index)
    length = sum(1 for _ in ps.items())
    if length == 24:
        hourly_weather_records_ref.delete()

    for index, value in ps.items():
        timestamp = pd.Timestamp(index)
        unix_timestamp_ms = int(timestamp.timestamp() * 1000)
        add_weather_record(unix_timestamp_ms, timestamp.hour, value, current_data['Rel_Hum'], current_data['PresskPa'], current_data['AQI'])
    
    # print("\nForecasted Temperatures for the Next 24 Hours:")
    # print(forecast_next_hours.predicted_mean)

    handle = db.reference('weather_station/hourly_weather_records')
    
    from sklearn.metrics import mean_absolute_error
    # mae = mean_absolute_error(actual_values, forecasted_values)
    # print(f"Mean Absolute Error (MAE): {mae}")
except Exception as e:
    print(e)