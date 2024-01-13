#include <iostream>
#include <fstream>
#include <ctime>
#include <thread>
#include <chrono>
#include <vector>
#include <deque>

using namespace std;

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
        std::cout << "[MoistureSensor_INFO] Sending sensor data to wifi: Moisture Level = " << moistureLevel << ", Time = " << std::ctime(&currentTime);
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
            storageFile << "[MoistureSensor_INFO] Moisture Level = " << moistureLevel << ", Time = " << std::ctime(&currentTime) << std::endl;
            storageFile.close();
            storageUsed += sizeof(moistureLevel) + sizeof(currentTime);
        }
        else
        {
            std::cout << "[MoistureSensor_ERROR] Failed to open storage file!" << std::endl;
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
                std::cout << "[Controller_INFO] Received data: " << data;
                std::this_thread::sleep_for(std::chrono::minutes(1));
            }
            outputFile.close();
        }
        else
        {
            std::cout << "[Controller_ERROR] Failed to open output file!" << std::endl;
        }
    }

    std::vector<std::string> getLastNSensorData(int n)
    {
        std::ifstream inputFile("received_data.txt");
        std::vector<std::string> lastNSensorData;

        if (inputFile.is_open())
        {

            std::string line;
            std::deque<std::string> lines;

            while (std::getline(inputFile, line))
            {
                lines.push_back(line);
                if (lines.size() > n)
                {
                    lines.pop_front();
                }
            }

            for (const std::string &line : lines)
            {
                lastNSensorData.push_back(line);
            }

            inputFile.close();
        }
        else
        {
            std::cout << "[Controller_ERROR] Failed to open input file!" << std::endl;
        }

        return lastNSensorData;
    }

private:
    void connectToWifi()
    {
        // Connect to wifi
        // Implement your logic here
        std::cout << "[Controller_INFO] Connecting to wifi..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
        std::cout << "[Controller_INFO] Wifi connected!" << std::endl;
    }
};

class User
{
public:
    User(Controller &controller)
        : controller(controller)
    {
    }

    void getLastNSensorData(int n)
    {
        std::vector<string> sensorData = controller.getLastNSensorData(n);

        for (const string &data : sensorData)
        {
            std::cout << "[User_INFO] " << data << std::endl;
        }
    }

private:
    Controller &controller;
};

int main()
{
    Controller controller;
    MoistureSensor sensor(1, 4096); // Set frequency to 60 minutes and storage size to 4KB
    User user(controller);

    // std::thread sensorThread(&MoistureSensor::readSensorData, &sensor);
    // std::thread controllerThread(&Controller::receiveDataFromWifi, &controller);

    // sensorThread.join();
    // controllerThread.join();

    user.getLastNSensorData(5);

    return 0;
}
