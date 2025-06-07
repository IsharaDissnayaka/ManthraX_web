
from flask import Flask, request, jsonify
import joblib
import numpy as np

# Load all models
models = {
    "realtime": joblib.load("energy_prediction_model.pkl"),
    "hour": joblib.load("hourly_energy_forecast_model.pkl"),
    "daily_energy": joblib.load("daily_energy_forecast_model.pkl"),
    "daily_cost": joblib.load("daily_cost_forecast_model.pkl"),
    "weekly_energy": joblib.load("weekly_energy_forecast_model.pkl"),
    "weekly_cost": joblib.load("weekly_cost_forecast_model.pkl"),
    "monthly_energy": joblib.load("monthly_energy_forecast_model.pkl"),
    "monthly_cost": joblib.load("monthly_cost_forecast_model.pkl"),
}

# Rule-based NILM logic
def infer_possible_devices(power):
    signatures = {
        "Refrigerator": (100, 250),
        "Fan": (50, 100),
        "Iron": (1000, 1500),
        "Microwave": (800, 1200),
        "Washing Machine": (400, 600),
        "LED TV": (50, 150)
    }
    return [device for device, (low, high) in signatures.items() if low <= power <= high] or ["Unknown or multiple devices"]

app = Flask(__name__)

@app.route('/predict', methods=['POST'])
def predict():
    data = request.json
    voltage = data.get("voltage")
    current = data.get("current")
    power = data.get("power")
    hour = data.get("hour")
    rolling_avg = data.get("rolling_power_avg")

    if None in (voltage, current, power, hour, rolling_avg):
        return jsonify({"error": "Missing one or more input fields."}), 400

    # Prepare input
    base_input = np.array([[voltage, current, power]])
    extended_input = np.array([[voltage, current, power, hour, rolling_avg]])
    summary_input = np.array([[power, voltage, current, rolling_avg]])

    # Perform predictions
    results = {
        "predicted_energy": round(models["realtime"].predict(base_input)[0], 4),
        "predicted_hourly_energy": round(models["hour"].predict(extended_input)[0], 4),
        "predicted_daily_energy": round(models["daily_energy"].predict(summary_input)[0], 2),
        "predicted_daily_cost": round(models["daily_cost"].predict(summary_input)[0], 2),
        "predicted_weekly_energy": round(models["weekly_energy"].predict(summary_input)[0], 2),
        "predicted_weekly_cost": round(models["weekly_cost"].predict(summary_input)[0], 2),
        "predicted_monthly_energy": round(models["monthly_energy"].predict(summary_input)[0], 2),
        "predicted_monthly_cost": round(models["monthly_cost"].predict(summary_input)[0], 2),
        "appliances_detected": infer_possible_devices(power)
    }

    return jsonify(results)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
