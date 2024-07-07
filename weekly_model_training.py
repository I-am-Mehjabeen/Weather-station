import pandas as pd
from statsmodels.tsa.statespace.sarimax import SARIMAX
import joblib

file_path = 'training_data.csv'
model_file_path = 'weekly_trained_model.pkl'

# Train and save the model only if the model file does not exist
try:
    # data = pd.read_csv(file_path)
    data = pd.read_csv(file_path, parse_dates=['Date_Time'], index_col='Date_Time')

    data = data.sort_index()
    data = data.ffill().bfill()

    # Convert Date_Time to datetime format
    # data["Date_Time"] = pd.to_datetime(data["Date_Time"])

    # Define the endogenous variable (temperature) and exogenous variables (humidity, pressure, PM2.5, PM4, PM10)
    endog = data['TempC']
    exog = data[['Rel_Hum', 'PresskPa', 'PM1_0', 'PM2_5', 'PM4', 'PM10', 'AQI']]
    # Split the data into training set (we use all available data to train)
    train_endog = endog
    train_exog = exog

    # Set Date_Time as index (if it's not already)
    # data.set_index("Date_Time", inplace=True)

    # Handle missing values (if any)
    # data.dropna(inplace=True)  # Example: dropping rows with missing values

    # train_data = data["TempC"].iloc[:-48]  # Exclude last 48 hours for testing

    # Define SARIMA parameters (order and seasonal_order)
    order = (1, 1, 1)  # Example order (p, d, q)
    seasonal_order = (1, 1, 1, 24)  # Example seasonal order (P, D, Q, S)

    # Create SARIMA model
    # model = SARIMAX(data["TempC"], order=order, seasonal_order=seasonal_order)

    # Fit the model
    # model_fit = model.fit()

    model = SARIMAX(train_endog, exog=train_exog, order=order, seasonal_order=seasonal_order)
    model_fit = model.fit(disp=False)

    # Save the model to a file
    joblib.dump(model_fit, model_file_path)
    print("Model trained and saved to", model_file_path)

except Exception as e:
    print(e)
