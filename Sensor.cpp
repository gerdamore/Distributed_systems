#include <iostream>
#include <fstream>
#include <ctime>
#include <thread>
#include <chrono>

struct SensorData
{
    int moistureLevel;
    std::time_t currentTime;
};
SensorData wifiData;

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
        wifiData.currentTime = currentTime;
        wifiData.moistureLevel = moistureLevel;
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

class Controller
{
public:
    Controller()
    {
        connectToWifi();
    }

    void receiveDataFromWifi()
    {
        std::ofstream outputFile("received_data.txt", std::ios::app);
        if (outputFile.is_open())
        {
            while (true)
            {
                std::time_t currentTime = std::time(nullptr);
                std::string data = "Moisture Level = " + std::to_string(wifiData.moistureLevel) + ", Time = " + std::ctime(&currentTime);
                outputFile << data << std::endl;
                std::cout << "Received data: " << data;
                std::this_thread::sleep_for(std::chrono::minutes(1));
            }
            outputFile.close();
        }
        else
        {
            std::cout << "Failed to open output file!" << std::endl;
        }
    }

private:
    void connectToWifi()
    {
        // Connect to wifi
        // Implement your logic here
        std::cout << "Connecting to wifi..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
        std::cout << "Wifi connected!" << std::endl;
    }
};



int main()
{
    Controller controller;
    MoistureSensor sensor(1, 4096); // Set frequency to 60 minutes and storage size to 4KB

    std::thread sensorThread(&MoistureSensor::readSensorData, &sensor);
    std::thread controllerThread(&Controller::receiveDataFromWifi, &controller);

    sensorThread.join();
    controllerThread.join();

    return 0;
}
