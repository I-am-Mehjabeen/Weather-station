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
    # data = pd.read_csv(file_path)

    # # Convert Date_Time to datetime format
    # data["Date_Time"] = pd.to_datetime(data["Date_Time"])

    # # Set Date_Time as index (if it's not already)
    # data.set_index("Date_Time", inplace=True)

    # # Handle missing values (if any)
    # data.dropna(inplace=True)  # Example: dropping rows with missing values

    # Load the data
    data = pd.read_csv('training_data.csv', parse_dates=['Date_Time'], index_col='Date_Time')

    # Ensure data is sorted by date
    data = data.sort_index()

    # Handle missing values
    data = data.ffill().bfill()

    # Define the endogenous variable (temperature) and exogenous variables (humidity, pressure, PM2.5, PM4, PM10)
    endog = data['TempC']
    exog = data[['Rel_Hum', 'PresskPa', 'PM1_0', 'PM2_5', 'PM4', 'PM10', 'AQI']]

    # Split the data into training set (we use all available data to train)
    train_endog = endog
    train_exog = exog
    # Load the model from the file
    model_fit = joblib.load(model_file_path)

    # Forecasting for the next 24 hours (hourly forecast)
    # forecast_next_hours = model_fit.get_forecast(steps=24)

    # Example: Forecasting for the next day (daily forecast)
    # forecast_next_day = model_fit.get_forecast(steps=2 * 24)

    # Example: Splitting data into train and test sets
    # train_data = data["TempC"].iloc[:-168]  # Exclude last 48 hours for testing
    # test_data = data["TempC"].iloc[-168:]  # Last 48 hours for testing
    current_data = data.iloc[-1]

    # Forecasting for the next 7 days (168 hours)
    # forecast_next_week = model_fit.get_forecast(steps=7*24)
    # forecast_next_week_mean = forecast_next_week.predicted_mean
    # # print(forecast_next_week_mean)
    # # Convert forecasted hourly temperatures to daily averages
    # forecast_next_week_mean_daily = forecast_next_week_mean.resample("D").mean().iloc[1:]
    
    # Generate future exogenous variables (we assume they remain constant for simplicity)
    last_exog = exog.iloc[-1]
    future_exog = pd.concat([last_exog.to_frame().T] * (24 * 7), ignore_index=True)
    future_exog.index = pd.date_range(start=data.index[-1] + pd.Timedelta(hours=1), periods=24 * 7, freq='H')

    # Forecast temperatures for the next 7 days (168 hours)
    forecast = model_fit.get_forecast(steps=24*7, exog=future_exog)

    # Extract forecasted values and confidence intervals
    forecast_mean = forecast.predicted_mean
    forecast_next_week_mean_daily = forecast_mean.resample("D").mean().iloc[1:]

    # print("\nForecasted Average Daily Temperatures for the Next 7 Days:")
    ps = pd.Series(forecast_next_week_mean_daily)
    # print(ps)
    length = sum(1 for _ in ps.items())
    if length == 7:
        weekly_weather_records_ref.delete()

    for index, value in ps.items():
        timestamp = pd.Timestamp(index)
        unix_timestamp_ms = int(timestamp.timestamp() * 1000)
        add_weather_record(unix_timestamp_ms, timestamp.dayofweek, value, current_data['Rel_Hum'], current_data['PresskPa'], current_data['AQI'])
    
    # print("\nForecasted Temperatures for the Next 24 Hours:")
    # print(forecast_next_hours.predicted_mean)

    handle = db.reference('weather_station/weekly_weather_records')

    # forecast_ci = forecast.conf_int()
    # # Plot the results
    # plt.figure(figsize=(15, 8))

    # # Plot historical data
    # plt.plot(train_endog.index[-24*7:], train_endog[-24*7:], label='Historical Data', color='blue')

    # # Plot forecasted data
    # plt.plot(forecast_mean.index, forecast_mean.values, label='Forecasted Data', color='red')

    # # Plot confidence intervals
    # plt.fill_between(forecast_ci.index, forecast_ci.iloc[:, 0], forecast_ci.iloc[:, 1], color='pink', alpha=0.2, label='Confidence Interval')

    # plt.legend()
    # plt.xlabel('Date')
    # plt.ylabel('Temperature °C')
    # plt.title('Temperature Forecast for the Next 7 Days')
    # plt.grid(True)
    # plt.show()

    # # # Print the forecasted temperatures for the next 7 days
    # print("Forecasted temperatures for the next 7 days:")
    # print(forecast_next_week_mean_daily)

except Exception as e:
    print(e)
