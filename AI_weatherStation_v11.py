# this file represents 7 days data.
import sqlite3
import pandas as pd
import matplotlib.pyplot as plt
from statsmodels.tsa.statespace.sarimax import SARIMAX

# Connect to SQLite database
conn = sqlite3.connect("weather_station.db")

# Query data from SQLite
query = "SELECT Date_Time, TempC FROM weather_data"
data = pd.read_sql_query(query, conn)

# Close connection
conn.close()

# Convert Date_Time to datetime format
data["Date_Time"] = pd.to_datetime(data["Date_Time"])

# Set Date_Time as index (if it's not already)
data.set_index("Date_Time", inplace=True)

# Handle missing values (if any)
data.dropna(inplace=True)

# Example: Splitting data into train and test sets
train_data = data["TempC"].iloc[:-48]  # Exclude last 48 hours for testing
test_data = data["TempC"].iloc[-48:]  # Last 48 hours for testing

# Define SARIMA parameters (order and seasonal_order)
order = (1, 1, 1)  # Example order (p, d, q)
seasonal_order = (1, 1, 1, 24)  # Example seasonal order (P, D, Q, S)

# Create SARIMA model using training data
model = SARIMAX(train_data, order=order, seasonal_order=seasonal_order)

# Fit the model
model_fit = model.fit()

# Summary of the model
print(model_fit.summary())

# Forecasting for the next 7 days (168 hours)
forecast_next_week = model_fit.get_forecast(steps=7 * 24)
forecast_next_week_mean = forecast_next_week.predicted_mean

# Convert forecasted hourly temperatures to daily averages
forecast_next_week_mean_daily = forecast_next_week_mean.resample("D").mean()

print("\nForecasted Average Daily Temperatures for the Next 7 Days:")
print(forecast_next_week_mean_daily)

# Plotting actual vs. forecasted values
plt.figure(figsize=(12, 6))

# Plot the training data
plt.plot(train_data.index, train_data, label="Training Data")

# Plot the actual values for the test set
plt.plot(test_data.index, test_data, label="Actual")

# Plot the forecasted values for the next 7 days
plt.plot(
    forecast_next_week_mean.index,
    forecast_next_week_mean,
    label="Forecasted - Next Week",
    color="red",
)

# Adding confidence intervals to the plot
conf_int = forecast_next_week.conf_int()
plt.fill_between(
    forecast_next_week_mean.index,
    conf_int.iloc[:, 0],
    conf_int.iloc[:, 1],
    color="red",
    alpha=0.3,
)
"""# Plot labels and title
plt.xlabel("Date_Time")
plt.ylabel("Temperature (C)")
plt.title("Actual vs. Forecasted Temperature")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.show()
"""
