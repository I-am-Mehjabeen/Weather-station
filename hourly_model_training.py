import pandas as pd
from statsmodels.tsa.statespace.sarimax import SARIMAX
import joblib

file_path = 'training_data.csv'
model_file_path = 'hourly_trained_model.pkl'

# Train and save the model only if the model file does not exist
try:
    data = pd.read_csv(file_path)

    # Convert Date_Time to datetime format
    data["Date_Time"] = pd.to_datetime(data["Date_Time"])

    # Set Date_Time as index (if it's not already)
    data.set_index("Date_Time", inplace=True)

    # Handle missing values (if any)
    data.dropna(inplace=True)  # Example: dropping rows with missing values

    # Optionally, create time features like hour, day of week, etc.
    data["Hour"] = data.index.hour
    data["DayOfWeek"] = data.index.dayofweek

    # Define SARIMA parameters (order and seasonal_order)
    order = (1, 1, 1)  # Example order (p, d, q)
    seasonal_order = (1, 1, 1, 24)  # Example seasonal order (P, D, Q, S)

    # Example: Splitting data into train and test sets
    train_data = data["TempC"].iloc[:-24]  # Exclude last 24 hours for testing

    # Create SARIMA model
    model = SARIMAX(data["TempC"], order=order, seasonal_order=seasonal_order)

    # Fit the model
    model_fit = model.fit()

    # Save the model to a file
    joblib.dump(model_fit, model_file_path)
    print("Model trained and saved to", model_file_path)

except Exception as e:
    print(e)
