import streamlit as st
import requests
import time

# --- Configuration ---
# The ESP32's IP address when acting as an Access Point
ESP32_URL = "http://192.168.4.1/data"

st.set_page_config(page_title="ESP32 Direct Telemetry", layout="wide")

def fetch_data():
    try:
        # 1 second timeout so the app doesn't freeze if ESP32 is unreachable
        response = requests.get(ESP32_URL, timeout=1.0)
        if response.status_code == 200:
            return response.json()
    except requests.exceptions.RequestException:
        pass
    return None

# --- UI Setup ---
st.title("🚀 ESP32 Direct Telemetry Dashboard")
st.caption("Fetching data directly from `http://192.168.4.1/data`")

state = fetch_data()

if state is None:
    st.error("❌ Cannot connect to the ESP32.")
    st.info("Make sure your laptop is connected to the **`ESP32_Project`** Wi-Fi network!")
else:
    # --- Top Row: Status ---
    col1, col2 = st.columns(2)
    
    status_color = "🟢" if state.get("status") == "RUNNING" else "🔴"
    col1.metric("Device Status", f"{status_color} {state.get('status', 'OFFLINE')}")
    col2.metric("Firmware Version", state.get("version", "unknown"))

    st.markdown("---")
    
    # --- Middle Row: Sensor Data ---
    colA, colB = st.columns(2)
    
    with colA:
        st.subheader("🧭 MPU6050")
        accel = state.get("mpu6050", {}).get("accel", {})
        gyro = state.get("mpu6050", {}).get("gyro", {})
        
        st.write("**Accelerometer (g)**")
        st.write(f"X: `{accel.get('x', 0):.2f}` | Y: `{accel.get('y', 0):.2f}` | Z: `{accel.get('z', 0):.2f}`")
        
        st.write("**Gyroscope (°/s)**")
        st.write(f"X: `{gyro.get('x', 0):.2f}` | Y: `{gyro.get('y', 0):.2f}` | Z: `{gyro.get('z', 0):.2f}`")

    with colB:
        st.subheader("☁️ BMP180")
        bmp = state.get("bmp180", {})
        
        st.metric("Temperature", f"{bmp.get('temp', 0):.2f} °C")
        st.metric("Pressure", f"{bmp.get('pressure', 0):.2f} hPa")
        st.metric("Altitude", f"{bmp.get('altitude', 0):.2f} m")

# --- Auto Refresh ---
time.sleep(1)
st.rerun()
