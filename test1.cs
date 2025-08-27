using System;
using System.Runtime.InteropServices;

namespace AquaControl
{
    public static class NativeAquaController
    {
        private const string LibraryName = "libaquacontrol.so";

        [DllImport(LibraryName, EntryPoint = "aqua_controller_init")]
        public static extern bool Initialize();

        [DllImport(LibraryName, EntryPoint = "aqua_controller_cleanup")]
        public static extern void Cleanup();

        [DllImport(LibraryName, EntryPoint = "aqua_read_sensors")]
        public static extern bool ReadSensors(
            out float phValue, 
            out float tdsValue, 
            out float temperature, 
            out float humidity);

        [DllImport(LibraryName, EntryPoint = "aqua_set_relay")]
        public static extern void SetRelay(int relayNumber, bool state);

        [DllImport(LibraryName, EntryPoint = "aqua_get_relay_state")]
        public static extern bool GetRelayState(int relayNumber);

        [DllImport(LibraryName, EntryPoint = "aqua_read_ph_voltage")]
        public static extern float ReadPhVoltage();

        [DllImport(LibraryName, EntryPoint = "aqua_read_tds_voltage")]
        public static extern float ReadTdsVoltage();
        
        [DllImport(LibraryName, EntryPoint = "aqua_get_last_error")]
        public static extern IntPtr GetLastError();
    }

    public class AquaController : IDisposable
    {
        private bool _isInitialized = false;

        public bool Initialize()
        {
            _isInitialized = NativeAquaController.Initialize();
            return _isInitialized;
        }

        public SensorData ReadSensors()
        {
            if (!_isInitialized)
                throw new InvalidOperationException("Controller not initialized");

            var success = NativeAquaController.ReadSensors(
                out float phValue,
                out float tdsValue,
                out float temperature,
                out float humidity);

            if (!success)
            {
                IntPtr errorPtr = NativeAquaController.GetLastError();
                string error = Marshal.PtrToStringAnsi(errorPtr) ?? "Unknown error";
                throw new Exception($"Sensor read failed: {error}");
            }

            return new SensorData
            {
                PhValue = phValue,
                TdsValue = tdsValue,
                Temperature = temperature,
                Humidity = humidity,
                IsValid = success
            };
        }

        public void SetRelay(int relayNumber, bool state)
        {
            if (!_isInitialized)
                throw new InvalidOperationException("Controller not initialized");

            NativeAquaController.SetRelay(relayNumber, state);
        }

        public bool GetRelayState(int relayNumber)
        {
            if (!_isInitialized)
                throw new InvalidOperationException("Controller not initialized");

            return NativeAquaController.GetRelayState(relayNumber);
        }

        public void Dispose()
        {
            if (_isInitialized)
            {
                NativeAquaController.Cleanup();
                _isInitialized = false;
            }
        }
    }

    public struct SensorData
    {
        public float PhValue { get; set; }
        public float TdsValue { get; set; }
        public float Temperature { get; set; }
        public float Humidity { get; set; }
        public bool IsValid { get; set; }
        
        public override string ToString()
        {
            return $"pH: {PhValue:F2}, TDS: {TdsValue:F1} ppm, Temp: {Temperature:F1}°C, Humidity: {Humidity:F1}%";
        }
    }
}