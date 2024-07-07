import csv
from datetime import datetime, timedelta
import random

# Start and end datetime for the records
start_datetime = datetime(2024, 7, 6, 0, 0, 0)
end_datetime = datetime(2024, 7, 6, 12, 0, 0)

# Define the range for each column
ranges = {
    'TempC': (16, 28),
    'Rel_Hum': (40, 90),
    'PresskPa': (100.80, 101.80),
    'PM1_0': (0.01, 0.99),
    'PM2_5': (0.99, 2.90),
    'PM4': (0.11, 3.99),
    'PM10': (0.20, 9.9),
    'AQI': (21, 25)
}

# Function to generate a random datetime between start and end
def random_datetime(start, end):
    return start + timedelta(hours=random.randint(0, int((end - start).total_seconds() / 3600)))

# Function to generate random values within given range
def generate_random_value(column_name):
    return round(random.uniform(ranges[column_name][0], ranges[column_name][1]), 2)

def generate_random_integer(column_name):
    return random.randint(ranges[column_name][0], ranges[column_name][1])

# Generate random records
records = []
current_datetime = start_datetime

while current_datetime <= end_datetime:
    record = {
        'Date_Time': current_datetime.strftime('%m/%d/%Y %H:%M')
    }
    
    for column in ranges:
        if column == 'AQI':
            record[column] = generate_random_integer(column)
        elif column != 'Date_Time':
            record[column] = generate_random_value(column)
    records.append(record)
    current_datetime += timedelta(hours=1)

# Write records to CSV
csv_filename = 'training_data.csv'
csv_columns = ['Date_Time', 'TempC', 'Rel_Hum', 'PresskPa', 'PM1_0', 'PM2_5', 'PM4', 'PM10', 'AQI']

file_exists = True
try:
    with open(csv_filename, 'r') as csvfile:
        reader = csv.reader(csvfile)
        if not list(reader):
            file_exists = False
except FileNotFoundError:
    file_exists = False
    
with open(csv_filename, 'a', newline='') as csvfile:
    writer = csv.DictWriter(csvfile, fieldnames=csv_columns)
    if not file_exists:
        writer.writeheader()  # Write headers only if the file is new
    for record in records:
        writer.writerow(record)

print(f"CSV file '{csv_filename}' has been generated successfully.")
