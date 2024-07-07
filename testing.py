import pandas as pd
from statsmodels.tsa.statespace.sarimax import SARIMAX
import matplotlib.pyplot as plt

# Load the data
data = pd.read_csv('training_data.csv', parse_dates=['Date_Time'], index_col='Date_Time')

# Ensure data is sorted by date
data = data.sort_index()

# Inspect the data
print(data.head())
print(data.tail())

# Handle missing values
data = data.fillna(method='ffill').fillna(method='bfill')

# Define the endogenous variable (temperature) and exogenous variables (humidity, pressure, PM2.5, PM4, PM10)
endog = data['TempC']
exog = data[['Rel_Hum', 'PresskPa', 'PM1_0', 'PM2_5', 'PM4', 'PM10']]

# Split the data into training set (we use all available data to train)
train_endog = endog
train_exog = exog

# Define and fit the SARIMAX model
model = SARIMAX(train_endog, exog=train_exog, order=(1, 1, 1), seasonal_order=(1, 1, 1, 24))
model_fit = model.fit(disp=False)

# Generate future exogenous variables (we assume they remain constant for simplicity)
last_exog = exog.iloc[-1]
future_exog = pd.concat([last_exog.to_frame().T] * (24 * 7), ignore_index=True)
future_exog.index = pd.date_range(start=data.index[-1] + pd.Timedelta(hours=1), periods=24 * 7, freq='H')

# Forecast temperatures for the next 7 days (168 hours)
forecast = model_fit.get_forecast(steps=24*7, exog=future_exog)

# Extract forecasted values and confidence intervals
forecast_mean = forecast.predicted_mean
forecast_next_week_mean_daily = forecast_mean.resample("D").mean().iloc[1:]

forecast_ci = forecast.conf_int()

# Plot the results
plt.figure(figsize=(15, 8))

# Plot historical data
plt.plot(train_endog.index[-24*7:], train_endog[-24*7:], label='Historical Data', color='blue')

# Plot forecasted data
plt.plot(forecast_mean.index, forecast_mean.values, label='Forecasted Data', color='red')

# Plot confidence intervals
plt.fill_between(forecast_ci.index, forecast_ci.iloc[:, 0], forecast_ci.iloc[:, 1], color='pink', alpha=0.2, label='Confidence Interval')

plt.legend()
plt.xlabel('Date')
plt.ylabel('Temperature °C')
plt.title('Temperature Forecast for the Next 7 Days')
plt.grid(True)
plt.show()

# # Print the forecasted temperatures for the next 7 days
print("Forecasted temperatures for the next 7 days:")
print(forecast_next_week_mean_daily)
