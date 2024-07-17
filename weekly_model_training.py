import pandas as pd
from statsmodels.tsa.statespace.sarimax import SARIMAX
import joblib

file_path = 'training_data.csv'
model_file_path = 'weekly_trained_model.pkl'

# Train and save the model only if the model file does not exist
try:
    # Load data
    data = pd.read_csv(file_path)
    
    # Convert Date_Time to datetime format
    data["Date_Time"] = pd.to_datetime(data["Date_Time"])

    # Set Date_Time as index
    data.set_index("Date_Time", inplace=True)

    # Handle missing values (e.g., forward fill, backward fill, drop)
    data.ffill(inplace=True)  # Forward fill missing values
    data.bfill(inplace=True)  # Backward fill missing values

    # Define the endogenous variable (temperature)
    endog = data['TempC']

    # Define SARIMA parameters (order and seasonal_order)
    order = (1, 1, 1)  # Example order (p, d, q)
    seasonal_order = (1, 1, 1, 24)  # Example seasonal order (P, D, Q, S)

    # Create SARIMA model
    model = SARIMAX(endog, order=order, seasonal_order=seasonal_order)

    # Fit the model
    model_fit = model.fit(disp=False)

    # Save the model to a file
    joblib.dump(model_fit, model_file_path)
    print("Model trained and saved to", model_file_path)

except Exception as e:
    print(e)
