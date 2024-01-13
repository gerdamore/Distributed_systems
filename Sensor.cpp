#include <iostream>
#include <fstream>
#include <ctime>
#include <thread>
#include <chrono>

class MoistureSensor
{
private:
    int frequency;
    int storageSize;
    int storageUsed;

public:
    MoistureSensor(int freq, int maxSize)
    {
        frequency = freq;
        storageSize = maxSize;
        storageUsed = 0;
    }

    void setFrequency(int freq)
    {
        frequency = freq;
    }

    void readSensorData()
    {
        // Simulate reading sensor data
        // Read sensor data at set frequency
        while (true)
        {
            int moistureLevel = 50; // Assuming 50 for demonstration purposes
            std::time_t currentTime = std::time(nullptr);

            if (isWifiConnected())
            {
                sendSensorDataToWifi(moistureLevel, currentTime);
            }
            else
            {
                storeSensorDataInStorage(moistureLevel, currentTime);
            }

            // Sleep for the set frequency in minutes
            std::this_thread::sleep_for(std::chrono::minutes(frequency));
        }
    }

private:
    bool isWifiConnected()
    {
        // Check if wifi is connected
        // Implement your logic here
        return true; // Assuming wifi is connected for demonstration purposes
    }

    void sendSensorDataToWifi(int moistureLevel, std::time_t currentTime)
    {
        // Send sensor data to wifi
        // Implement your logic here
        std::cout << "Sending sensor data to wifi: Moisture Level = " << moistureLevel << ", Time = " << std::ctime(&currentTime);
    }

    void storeSensorDataInStorage(int moistureLevel, std::time_t currentTime)
    {
        // Store sensor data in internal storage (file)
        // Implement your logic here
        std::ofstream storageFile("sensor_data.txt", std::ios::app);
        if (storageFile.is_open())
        {
            storageFile << "Moisture Level = " << moistureLevel << ", Time = " << std::ctime(&currentTime) << std::endl;
            storageFile.close();
            storageUsed += sizeof(moistureLevel) + sizeof(currentTime);
        }
        else
        {
            std::cout << "Failed to open storage file!" << std::endl;
        }
    }
};

int main()
{
    MoistureSensor sensor(1, 4096); // Set frequency to 60 minutes and storage size to 4KB

    sensor.readSensorData();

    return 0;
}
