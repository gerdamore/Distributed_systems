#include <iostream>
#include <fstream>
#include <ctime>
#include <thread>
#include <chrono>
#include <vector>
#include <deque>
#include <cstdlib> // For rand()

using namespace std;

string data_requested;
int request = 0;
string data_requested_nlast_Data;
int request_nlast_Data = 0;

struct SensorData
{
    int moistureLevel;
    time_t currentTime;
};
SensorData wifiData;

class MoistureSensor
{
private:
    int frequency;
    int storageSize;
    int storageUsed;
    int depth;

public:
    MoistureSensor() : frequency(0), storageSize(0), storageUsed(0), depth(0)
    {
    }

    MoistureSensor(int freq, int maxSize, int d)
    {
        frequency = freq;
        storageSize = maxSize;
        storageUsed = 0;
        depth = d;
    }

    void setFrequency(int freq)
    {
        frequency = freq;
    }

    int getDepth()
    {
        return depth;
    }

    SensorData readSensor()
    {
        SensorData data;
        data.moistureLevel = rand() % 101;
        data.currentTime = time(nullptr);

        return data;
    }

    void readSensorData()
    {
        // Simulate reading sensor data
        // Read sensor data at set frequency
        while (true)
        {

            int moistureLevel = rand() % 101; // Randomize the moisture level between 0 and 100
            time_t currentTime = time(nullptr);

            if (isWifiConnected())
            {
                sendSensorDataToWifi(moistureLevel, currentTime);
            }
            else
            {
                storeSensorDataInStorage(moistureLevel, currentTime);
            }

            // Sleep for the set frequency in minutes
            this_thread::sleep_for(chrono::minutes(frequency));
        }
    }

    void readSensorDataOnceAndSend()
    {
        int moistureLevel = rand() % 101; // Randomize the moisture level between 0 and 100
        time_t currentTime = time(nullptr);

        if (isWifiConnected())
        {
            sendSensorDataToWifi(moistureLevel, currentTime);
        }
        else
        {
            cout << "[MoistureSensor_ERROR] Wifi is not connected!" << endl;
        }
    }

    void calibrate()
    {
        cout << "[MoistureSensor_INFO] Starting calibration..." << endl;
        // Perform calibration logic here
        cout << "[MoistureSensor_INFO] Calibration completed!" << endl;
    }

private:
    bool isWifiConnected()
    {
        // Check if wifi is connected
        // Implement your logic here
        return true; // Assuming wifi is connected for demonstration purposes
    }

    void sendSensorDataToWifi(int moistureLevel, time_t currentTime)
    {
        // Send sensor data to wifi
        // Implement your logic here
        cout << "[MoistureSensor_INFO] Sending sensor data to wifi: Moisture Level = " << moistureLevel << ", Time = " << ctime(&currentTime);
        wifiData.currentTime = currentTime;
        wifiData.moistureLevel = moistureLevel;
    }

    void storeSensorDataInStorage(int moistureLevel, time_t currentTime)
    {
        // Store sensor data in internal storage (file)
        // Implement your logic here
        ofstream storageFile("sensor_data.txt", ios::app);
        if (storageFile.is_open())
        {
            int dataSize = sizeof(moistureLevel) + sizeof(currentTime);
            if (storageUsed + dataSize <= MoistureSensor::storageSize)
            {
                storageFile << "[MoistureSensor_INFO] Moisture Level = " << moistureLevel << ", Time = " << ctime(&currentTime) << endl;
                storageFile.close();
                storageUsed += dataSize;
            }
            else
            {
                cout << "[MoistureSensor_ERROR] Storage capacity exceeded!" << endl;
            }
        }
        else
        {
            cout << "[MoistureSensor_ERROR] Failed to open storage file!" << endl;
        }
    }
};

class Controller
{
public:
    Controller() {}

    Controller(const vector<MoistureSensor> &sensors) : sensors(sensors)
    {
        connectToWifi();
    }

    void set_Sensor(const vector<MoistureSensor> &sensors)
    {
        this->sensors = sensors;
    }

    void receiveDataFromWifi()
    {
        ofstream outputFile("received_data.txt", ios::app);
        if (outputFile.is_open())
        {
            while (true)
            {
                time_t currentTime = time(nullptr);
                for (MoistureSensor &sensor : sensors)
                {
                    SensorData wifiData = sensor.readSensor();
                    string data = "Moisture Level = " + to_string(wifiData.moistureLevel) + ", Time = " + ctime(&wifiData.currentTime);
                    outputFile << data << endl;
                    cout << "[Controller_INFO] Received data: " << data;
                }
                this_thread::sleep_for(chrono::minutes(1));
            }
            outputFile.close();
        }
        else
        {
            cout << "[Controller_ERROR] Failed to open output file!" << endl;
        }
    }

    void getLastNSensorData(int n)
    {
        while (true)
        {
            if (request_nlast_Data == 1)
            {
                ifstream inputFile("received_data.txt");
                string lastNSensorData;

                if (inputFile.is_open())
                {

                    string line;
                    deque<string> lines;

                    while (getline(inputFile, line))
                    {
                        lines.push_back(line);
                        if (lines.size() > n)
                        {
                            lines.pop_front();
                        }
                    }

                    for (const string &line : lines)
                    {
                        lastNSensorData += line + "\n";
                    }

                    inputFile.close();
                }
                else
                {
                    cout << "[Controller_ERROR] Failed to open input file!" << endl;
                }

                data_requested_nlast_Data = lastNSensorData;
                this_thread::sleep_for(chrono::seconds(7));
            }
            // wait for 50 seconds
        }
    }

    void readSensor()
    {
        while (true)
        {
            if (request == 1)
            {
                string sensorDataWithDepth;
                for (MoistureSensor &sensor : sensors)
                {
                    SensorData sensorData = sensor.readSensor();
                    string data = "Moisture Level = " + to_string(sensorData.moistureLevel) + ", Time = " + ctime(&sensorData.currentTime);
                    int depth = sensor.getDepth();
                    sensorDataWithDepth += "Sensor Depth = " + to_string(depth) + ", " + data + "\n";
                }
                data_requested = sensorDataWithDepth;
                this_thread::sleep_for(chrono::seconds(7));
            }
            // wait for 50 seconds
        }
    }

private:
    vector<MoistureSensor> sensors;
    void connectToWifi()
    {
        // Connect to wifi
        // Implement your logic here
        cout << "[Controller_INFO] Connecting to wifi..." << endl;
        this_thread::sleep_for(chrono::seconds(2));
        cout << "[Controller_INFO] Wifi connected!" << endl;
    }
};

class User
{
public:
    User()
    {
    }

    void getLastNSensorData(int n)
    {
        cout << "[User_INFO] Request sensor data " << endl;
        request_nlast_Data = 1;
        // wait for 30 seconds

        // string data = controller.readSensor();
        this_thread::sleep_for(chrono::seconds(15));
        cout << "[User_INFO] Sensor Data: " << data_requested_nlast_Data << endl;

        request_nlast_Data = 0;
    }

    void requestSensorData()
    {

        cout << "[User_INFO] Request sensor data " << endl;
        request = 1;
        // wait for 30 seconds

        // string data = controller.readSensor();
        this_thread::sleep_for(chrono::seconds(5));
        cout << "[User_INFO] Sensor Data: " << data_requested << endl;

        request = 0;
    }

private:
};

class SoilLayer
{
public:
    double depth;
    double thickness;
    double fieldCapacity;
    double availableWaterHoldingCapacity;

    SoilLayer() : depth(0), thickness(0), fieldCapacity(0), availableWaterHoldingCapacity(0)
    {
    }

    SoilLayer(double depth, double thickness, double fieldCapacity, double availableWaterHoldingCapacity)
        : depth(depth), thickness(thickness), fieldCapacity(fieldCapacity), availableWaterHoldingCapacity(availableWaterHoldingCapacity)
    {
    }

    double calculateWaterDepthAtAWC()
    {
        return availableWaterHoldingCapacity * (thickness / 10.0) / 10.0; // Convert result from mm to cm
    }

    double calculateDepthAtMAD()
    {
        return calculateWaterDepthAtAWC() * 0.5; // Result in cm
    }

    double calculateSoilWaterDepletion(double volumetricWaterContent)
    {
        double waterDepletion = (fieldCapacity * thickness) - (volumetricWaterContent / 100 * thickness);
        return waterDepletion;
    }
};

class IrrigationSystem
{
public:
    // Set frequency to 60 minutes and storage size to 4KB

    IrrigationSystem()
    {

        this->soilLayer_30 = SoilLayer(30.0, 30.0, 0.25, 50.0);
        this->soilLayer_60 = SoilLayer(60.0, 30.0, 0.21, 45.0);
        this->soilLayer_90 = SoilLayer(90.0, 30.0, 0.19, 37.0);
        this->sensor = MoistureSensor(1, 4096, 30);
        this->controller = Controller({sensor});
        this->user = User();

        sensor.calibrate();
        thread sensorThread(&MoistureSensor::readSensorData, &sensor);
        sensorThread.detach();
        thread controllerThread(&Controller::receiveDataFromWifi, &controller);
        controllerThread.detach();
        thread controllerThread_readsensor(&Controller::readSensor, &controller);
        controllerThread_readsensor.detach();
        thread controllerThread_readsensor_nlast(&Controller::getLastNSensorData, &controller, 5);
        controllerThread_readsensor_nlast.detach();
    }

    void checkSensorLayer()
    {
        for (const auto &layer : {soilLayer_30, soilLayer_60, soilLayer_90})
        {
            if (layer.depth == sensor.getDepth())
            {
                // Calculate soil water depletion
                // print soil water depletion and soil layer depth
                cout << "Soil water depletion at depth " << layer.depth << ": " << soilLayer_30.calculateSoilWaterDepletion(sensor.readSensor().moistureLevel) << " cm" << endl;
                return;
            }
        }
        cout << "No matching soil layer found for the sensor." << endl;
    }

    void
    start()
    {
        string command;
        while (getline(cin, command))
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
                cout << "Enter the value of n: ";
                cin >> n;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                user.getLastNSensorData(n);
            }
            else if (command == "3")
            {
                checkSensorLayer();
            }
            else
            {
                cout << "Invalid command!" << endl;
            }
        }
    }

private:
    Controller controller;
    User user;
    MoistureSensor sensor;
    SoilLayer soilLayer_30;
    SoilLayer soilLayer_60;
    SoilLayer soilLayer_90;
};

int main()
{

    IrrigationSystem irrigationSystem;
    irrigationSystem.start();

    return 0;
}
