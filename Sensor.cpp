#include <iostream>
#include <fstream>
#include <ctime>
#include <thread>
#include <chrono>
#include <vector>
#include <deque>
#include <cstdlib> // For std::rand()

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

            int moistureLevel = std::rand() % 101; // Randomize the moisture level between 0 and 100
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

    void readSensorDataOnceAndSend()
    {
        int moistureLevel = std::rand() % 101; // Randomize the moisture level between 0 and 100
        std::time_t currentTime = std::time(nullptr);

        if (isWifiConnected())
        {
            sendSensorDataToWifi(moistureLevel, currentTime);
        }
        else
        {
            std::cout << "[MoistureSensor_ERROR] Wifi is not connected!" << std::endl;
        }
    }

    void calibrate()
    {
        std::cout << "[MoistureSensor_INFO] Starting calibration..." << std::endl;
        // Perform calibration logic here
        std::cout << "[MoistureSensor_INFO] Calibration completed!" << std::endl;
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
            int dataSize = sizeof(moistureLevel) + sizeof(currentTime);
            if (storageUsed + dataSize <= MoistureSensor::storageSize)
            {
                storageFile << "[MoistureSensor_INFO] Moisture Level = " << moistureLevel << ", Time = " << std::ctime(&currentTime) << std::endl;
                storageFile.close();
                storageUsed += dataSize;
            }
            else
            {
                std::cout << "[MoistureSensor_ERROR] Storage capacity exceeded!" << std::endl;
            }
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
    Controller(MoistureSensor &sensor) : moistureSensor(sensor)
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
                std::string data = "Moisture Level = " + std::to_string(wifiData.moistureLevel) + ", Time = " + std::ctime(&wifiData.currentTime);
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

    SensorData readSensor()
    {
        moistureSensor.readSensorDataOnceAndSend();
        return wifiData;
    }

private:
    MoistureSensor moistureSensor;
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

    void requestSensorData()
    {
        std::cout << "[User_INFO] Request sensor data " << std::endl;
        SensorData wifiData = controller.readSensor();
        std::string data = "Moisture Level = " + std::to_string(wifiData.moistureLevel) + ", Time = " + std::ctime(&wifiData.currentTime);
        std::cout << "[User_INFO] Sensor Data: " << data << std::endl;
    }

private:
    Controller &controller;
};

class IrrigationSystem
{
public:
    // Set frequency to 60 minutes and storage size to 4KB

    IrrigationSystem() : sensor(60, 4096), controller(sensor), user(controller)
    {
        sensor.calibrate();
        std::thread sensorThread(&MoistureSensor::readSensorData, &sensor);
        sensorThread.detach();
        std::thread controllerThread(&Controller::receiveDataFromWifi, &controller);
        controllerThread.detach();
    }

    void start()
    {
        std::string command;
        while (std::getline(std::cin, command))
        {
            if (command == "1")
            {
                // User request sensor data
                user.requestSensorData();
            }
            else if (command == "2")
            {
                // User request last n sensor data
                int n;
                std::cout << "Enter the value of n: ";
                std::cin >> n;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                user.getLastNSensorData(n);
            }
            else
            {
                std::cout << "Invalid command!" << std::endl;
            }
        }
    }

private:
    Controller controller;
    User user;
    MoistureSensor sensor;
};

int main()
{

    IrrigationSystem irrigationSystem;
    irrigationSystem.start();

    return 0;
}
