# this file is predicting weather for 24 hours
import sqlite3
import pandas as pd
import matplotlib.pyplot as plt
from statsmodels.tsa.statespace.sarimax import SARIMAX

# Connect to SQLite database
conn = sqlite3.connect("weather_station.db")

try:
    # Query data from SQLite
    query = "SELECT Date_Time, TempC, DewPoint, Rel_Hum, PresskPa, Weather, PM1_0, PM2_5, PM4, PM10 FROM weather_data"
    data = pd.read_sql_query(query, conn)

    # Convert Date_Time to datetime format
    data["Date_Time"] = pd.to_datetime(data["Date_Time"])

    # Set Date_Time as index (if it's not already)
    data.set_index("Date_Time", inplace=True)

    # Handle missing values (if any)
    data.dropna(inplace=True)  # Example: dropping rows with missing values

    # Optionally, create time features like hour, day of week, etc.
    data["Hour"] = data.index.hour
    data["DayOfWeek"] = data.index.dayofweek

    # Example: SARIMA model for temperature forecasting
    # Define SARIMA parameters (order and seasonal_order)
    order = (1, 1, 1)  # Example order (p, d, q)
    seasonal_order = (1, 1, 1, 24)  # Example seasonal order (P, D, Q, S)

    # Example: Splitting data into train and test sets
    train_data = data["TempC"].iloc[:-24]  # Exclude last 24 hours for testing
    test_data = data["TempC"].iloc[-24:]  # Last 24 hours for testing

    # Create SARIMA model
    model = SARIMAX(data["TempC"], order=order, seasonal_order=seasonal_order)

    # Fit the model
    model_fit = model.fit()

    # Summary of the model
    print(model_fit.summary())

    # Forecasting for the next 24 hours (hourly forecast)
    forecast_next_hours = model_fit.get_forecast(steps=24)
    # Example: Forecasting for the next day (daily forecast)
    forecast_next_day = model_fit.get_forecast(steps=2 * 24)

    # Compare forecasted values with actual test data
    forecasted_values = forecast_next_hours.predicted_mean
    actual_values = test_data.values
    # Example: Forecasting for the next 7 days (daily forecast)
    # forecast_next_week = model_fit.get_forecast(steps=7 * 24)
    # Print forecasted values for the next 24 hours
    print("\nForecasted Temperatures for the Next 24 Hours:")
    print(forecast_next_hours.predicted_mean)
    # print("\nForecasted Temperatures for the Next week")
    # print(forecast_next_day.predicted_mean)
    # Calculate Mean Absolute Error (MAE) or other metrics
    from sklearn.metrics import mean_absolute_error

    mae = mean_absolute_error(actual_values, forecasted_values)
    print(f"Mean Absolute Error (MAE): {mae}")

    # Plotting forecasts
    plt.figure(figsize=(12, 6))
    plt.plot(data["TempC"], label="Observed")
    plt.plot(forecast_next_hours.predicted_mean, label="Next 24 Hours Forecast")
    plt.xlabel("Date_Time")
    plt.ylabel("Temperature (C)")
    plt.title("Temperature Forecasting")
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.show()
    # ------------------------
    """
    # Plotting actual vs. forecasted values
    plt.figure(figsize=(12, 6))
    plt.plot(test_data.index, actual_values, label="Actual")
    plt.plot(test_data.index, forecasted_values, label="Forecasted")
    plt.xlabel("Date_Time")
    plt.ylabel("Temperature (C)")
    plt.title("Actual vs. Forecasted Temperature")
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.show()
    """
    # Plot diagnostics of the model
    model_fit.plot_diagnostics(figsize=(15, 10))
    plt.show()

finally:
    # Close connection
    conn.close()


"""
from sklearn.model_selection import TimeSeriesSplit

# Example: Time series cross-validation
tscv = TimeSeriesSplit(n_splits=5)

for train_index, test_index in tscv.split(data["TempC"]):
    train_data, test_data = (
        data["TempC"].iloc[train_index],
        data["TempC"].iloc[test_index],
    )
    model = SARIMAX(train_data, order=order, seasonal_order=seasonal_order)
    model_fit = model.fit()
    forecast = model_fit.get_forecast(steps=len(test_data))
    forecasted_values = forecast.predicted_mean
    actual_values = test_data.values
    mae = mean_absolute_error(actual_values, forecasted_values)
    print(f"Fold MAE: {mae}")
"""
