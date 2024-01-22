#include <iostream>
#include <fstream>
#include <ctime>
#include <thread>
#include <chrono>
#include <vector>
#include <deque>
#include <dirent.h>
#include <cstdlib> // For rand()
#include <cstring>
#include <string.h>
#include <sstream>

using namespace std;

string data_requested;
int request = 0;
string data_requested_nlast_Data;
int request_nlast_Data = 0;

struct SensorData
{
    double moistureLevel;
    string currentTime;
};
SensorData wifiData;

#define STORAGE_SIZE_MAX 4096

std::string get_Word(const std::string &input, int n)
{
    std::istringstream iss(input);
    std::string word;

    for (int i = 0; i <= n; i++)
        iss >> word;

    return word;
}

class MoistureSensor
{
private:
    int frequency;
    int storageUsed;
    int depth;
    int line_number;
    string filename;

public:
    MoistureSensor() : frequency(0), storageUsed(0), depth(0), line_number(2)
    {
    }

    MoistureSensor(int freq, int d, string f)
    {
        line_number = 0;
        storageUsed = 0;
        frequency = freq;
        depth = d;
        filename = f;
    }

    void setFilename(string filename)
    {
        this->filename = filename;
    }

    int getDepth()
    {
        return depth;
    }

    SensorData get_data_from_line(string line)
    {
        SensorData sensorData{0, ""};
        sensorData.moistureLevel = stod(get_Word(line, depth / 30 + 2));
        sensorData.currentTime = get_Word(line, 1);
        return sensorData;
    }

    SensorData readSensor()
    {
        ifstream inputFile(this->filename);

        SensorData sensorData{0, ""};
        if (inputFile.is_open())
        {
            string line;
            int line_number = 0;
            while (getline(inputFile, line))
            {

                if (line_number == this->line_number)
                {
                    sensorData = get_data_from_line(line);
                    this->line_number++;
                    break;
                }
                line_number++;
            }
            inputFile.close();
        }
        else
        {
            cout << "[MoistureSensor_ERROR] Failed to open input file!" << endl;
        }
        return sensorData;
    }

    void readSensorData()
    {
        // Simulate reading sensor data
        // Read sensor data at set frequency
        while (true)
        {
            SensorData wifiData = readSensor();
            if (isWifiConnected())
            {
                sendSensorDataToWifi(wifiData.moistureLevel, wifiData.currentTime);
            }
            else
            {
                storeSensorDataInStorage(wifiData.moistureLevel, wifiData.currentTime);
            }

            // Sleep for the set frequency in minutes
            this_thread::sleep_for(chrono::minutes(frequency));
        }
    }

    void readSensorDataOnceAndSend()
    {
        SensorData wifiData = readSensor();
        if (isWifiConnected())
        {
            sendSensorDataToWifi(wifiData.moistureLevel, wifiData.currentTime);
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

    void sendSensorDataToWifi(int moistureLevel, string currentTime)
    {
        // Send sensor data to wifi
        // Implement your logic here
        cout << "[MoistureSensor_INFO] Sending sensor data to wifi: Moisture Level = " << to_string(moistureLevel) << ", Time = " << (currentTime) << endl;
        wifiData.currentTime = currentTime;
        wifiData.moistureLevel = moistureLevel;
    }

    void storeSensorDataInStorage(int moistureLevel, string currentTime)
    {
        // Store sensor data in internal storage (file)
        // Implement your logic here
        ofstream storageFile("sensor_data.txt", ios::app);
        if (storageFile.is_open())
        {
            int dataSize = sizeof(moistureLevel) + sizeof(currentTime);
            if (storageUsed + dataSize <= STORAGE_SIZE_MAX)
            {
                storageFile << "[MoistureSensor_INFO] Moisture Level = " << moistureLevel << ", Time = " << currentTime << endl;
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
                for (MoistureSensor &sensor : sensors)
                {
                    SensorData wifiData = sensor.readSensor();
                    string data = "[Controller_INFO] Moisture Level = " + to_string(wifiData.moistureLevel) + ", Time = " + wifiData.currentTime;
                    outputFile << data << endl;
                    cout << data << endl;
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
                    string data = "[Controller_INFO] Moisture Level = " + to_string(sensorData.moistureLevel) + ", Time = " + sensorData.currentTime;
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

    string select_rand_folder()
    {
        srand(time(0)); // Seed the random number generator
        vector<string> folders;
        string folderPath = "./Distributed_systems/sensor_data_CAF"; // Replace with the actual folder path

        // Read the contents of the folder
        DIR *dir;
        struct dirent *ent;

        if ((dir = opendir(folderPath.c_str())) != NULL)

        {
            while ((ent = readdir(dir)) != NULL)
            {
                if (ent->d_type == DT_DIR && strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0)
                {
                    folders.push_back(ent->d_name);
                }
            }
            closedir(dir);
        }
        else
        {
            cout << "[IrrigationSystem_ERROR] Failed to open folder: " << folderPath << endl;
            return "";
        }

        if (folders.empty())
        {
            cout << "[IrrigationSystem_ERROR] No folders found in: " << folderPath << endl;
            return "";
        }

        int randomIndex = rand() % folders.size();
        string selectedFolder = folders[randomIndex];
        cout << "[IrrigationSystem_INFO] Selected folder: " << selectedFolder << endl;
        return folderPath + "/" + selectedFolder;
    }

    string select_rand_file(string folderPath)
    {
        srand(time(0)); // Seed the random number generator
        vector<string> files;
        // Open a random file in the selected folder
        string folderPathWithFile = folderPath;
        // Read the contents of the folder
        DIR *dir;
        struct dirent *ent;

        if ((dir = opendir(folderPathWithFile.c_str())) != NULL)
        {
            while ((ent = readdir(dir)) != NULL)
            {
                if (ent->d_type == DT_REG)
                {
                    files.push_back(ent->d_name);
                }
            }
            closedir(dir);
        }
        else
        {
            cout << "[IrrigationSystem_ERROR] Failed to open folder: " << folderPathWithFile << endl;
            return "";
        }

        if (files.empty())
        {
            cout << "[IrrigationSystem_ERROR] No files found in: " << folderPathWithFile << endl;
            return "";
        }

        int randomIndex = rand() % files.size();
        string selectedFile = files[randomIndex];
        cout << "[IrrigationSystem_INFO] Selected file: " << selectedFile << endl;
        return folderPath + "/" + selectedFile;
    }

    IrrigationSystem()
    {
        string folder = select_rand_folder();
        string file = select_rand_file(folder);
        this->soilLayer_30 = SoilLayer(30.0, 30.0, 0.25, 50.0);
        this->soilLayer_60 = SoilLayer(60.0, 30.0, 0.21, 45.0);
        this->soilLayer_90 = SoilLayer(90.0, 30.0, 0.19, 37.0);
        this->sensor = MoistureSensor(1, 30, file);
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
                cout << "[IrrigationSystem_INFO] Soil water depletion at depth " << layer.depth << ": " << soilLayer_30.calculateSoilWaterDepletion(sensor.readSensor().moistureLevel) << " cm" << endl;
                return;
            }
        }
        cout << "[IrrigationSystem_INFO] No matching soil layer found for the sensor." << endl;
    }

    void start()
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
                cout << "[IrrigationSystem_ERROR] Invalid command!" << endl;
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
