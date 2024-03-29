#include <iostream>
#include <fstream>
#include <ctime>
#include <thread>
#include <chrono>
#include <vector>
#include <deque>
#include <dirent.h>
#include <cstdlib> 
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

// TODO: doesn't read the last line?
std::string readLastLine(const std::string &filename)
{
    std::ifstream file(filename);

    if (!file.is_open())
    {
        std::cerr << "Error: Unable to open file " << filename << std::endl;
        return ""; // Return an empty string to indicate an error
    }

    file.seekg(0, std::ios::end); // Move to the end of the file

    // Check if the file is empty
    if (file.tellg() <= 0)
    {
        return ""; // Return an empty string if the file is empty
    }

    std::string lastLine;
    char ch;

    // Move backward until a newline character or the beginning of the file is found
    do
    {
        file.seekg(-2, std::ios::cur); // Move two characters backward

        if (file.tellg() <= 0)
        {
            // Reached the beginning of the file
            file.seekg(0, std::ios::beg); // Move to the beginning of the file
            break;
        }

        file.get(ch); // Read the current character

        // If the current character is not a newline, add it to the lastLine string
        if (ch != '\n')
        {
            lastLine = ch + lastLine;
        }

    } while (ch != '\n');

    return lastLine;
}

struct WifiChannel
{
    int channel;
    bool active;
    bool probe_response;
    bool authenticated;
    int associated;
};

// TODO : based on the frame received, decide what to do
class WirelessCommunication
{
public:
    WirelessCommunication()
    {
        active_channels = {{1, false, false, false, -1}, {6, false, false, false, -1}, {11, false, false, false, -1}};
        received_data = false;
    }

    string getFrame()
    {
        return data;
    }

    bool receiveFrame()
    {
        if (received_data == true)
        {
            received_data = false;
            return true;
        }
        return false;
    }

    void sendFrame(double moistureLevel, string currentTime)
    {
        data = to_string(moistureLevel) + ", " + currentTime;
        // sleep for 10 seconds
        this_thread::sleep_for(chrono::seconds(2));
        received_data = true;
    }

    bool send_association_response(int channel_number)
    {
        for (int i = 0; i < active_channels.size(); i++)
        {
            if (active_channels[i]
                        .authenticated == true &&
                active_channels[i].channel == channel_number)
            {
                active_channels[i].associated = 1;
                string association_response = "[WirelessCommunication_INFO] Sending association response on channel : " + to_string(active_channels[i].channel);
                cout << association_response << endl;
                return 1;
            }
        }
        return 0;
    }

    bool send_probe_response(int channel_number)
    {
        for (int i = 0; i < active_channels.size(); i++)
        {
            if (active_channels[i]
                        .active == true &&
                active_channels[i].channel == channel_number)
            {
                active_channels[i].probe_response = true;
                string probe_response = "[WirelessCommunication_INFO] Sending probe response on channel : " + to_string(active_channels[i].channel);
                cout << probe_response << endl;
                return true;
            }
        }
        return false;
    }

    // TODO: model when it is not associated
    bool waiting_for_association(int channel_number)
    {
        for (int i = 0; i < active_channels.size(); i++)
        {
            if (active_channels[i]
                        .authenticated == true &&
                active_channels[i].channel == channel_number && active_channels[i].associated == 1)
            {
                return true;
            }
        }
        return false;
    }

    bool waiting_for_authentication(int channel_number)
    {
        for (int i = 0; i < active_channels.size(); i++)
        {
            if (active_channels[i]
                        .authenticated == true &&
                active_channels[i].channel == channel_number)
            {
                return true;
            }
        }
        return false;
    }

    void send_back_authentication_result(int channel_number)
    {
        for (int i = 0; i < active_channels.size(); i++)
        {
            if (active_channels[i]
                        .active == true &&
                active_channels[i].channel == channel_number)
            {
                string authentication_result = "[WirelessCommunication_INFO] Sending authentication result on channel : " + to_string(active_channels[i].channel);
                cout << authentication_result << endl;
                active_channels[i].authenticated = true;
            }
        }
    }

    void authenticate_request(int channel_number)
    {

        for (int i = 0; i < active_channels.size(); i++)
        {
            if (active_channels[i]
                        .active == true &&
                active_channels[i].channel == channel_number)
            {
                string authenticate_request = "[WirelessCommunication_INFO] Authenticating request on channel : " + to_string(active_channels[i].channel);
                cout << authenticate_request << endl;
                send_back_authentication_result(channel_number);
            }
        }
    }

    void association_request(int channel_number)
    {
        for (int i = 0; i < active_channels.size(); i++)
        {
            if (active_channels[i]
                        .authenticated == true &&
                active_channels[i].channel == channel_number)
            {
                string association_request = "[WirelessCommunication_INFO] Association request on channel : " + to_string(active_channels[i].channel);
                cout << association_request << endl;
                send_association_response(channel_number);
            }
        }
    }

    void send_probe_request(int i)
    {
        string probe_request = "[WirelessCommunication_INFO] Sending probe request on channel and waiting for reply: " + to_string(active_channels[i].channel);
        cout << probe_request << endl;
    }

    void active_scanning()
    {
        // get size of active_channels

        while (true)
        {
            for (int i = 0; i < active_channels.size(); i++)
            {
                if (active_channels[i].active == false)
                {
                    active_channels[i].active = true;
                    send_probe_request(i);
                    this_thread::sleep_for(chrono::seconds(100));
                    active_channels[i].active = false;
                }
            }
        }
    }

private:
    vector<WifiChannel> active_channels;
    bool received_data;
    string data;
};

class MoistureSensor
{
public:
    MoistureSensor() : frequency(0), storageUsed(0), depth(0), line_number(2)
    {
    }

    MoistureSensor(int freq, int d, string f, WirelessCommunication *w)
    {
        line_number = 0;
        storageUsed = 0;
        frequency = freq;
        depth = d;
        filename = f;
        communication = w;
    }

    void init_communication()
    {
        cout << "[MoistureSensor_INFO: " << depth << "] Initializing communication on channel 1" << endl;
        while ((*communication).send_probe_response(1) == false)
            ;
        (*communication).authenticate_request(1);
        while ((*communication).waiting_for_authentication(1) == false)
            ;
        (*communication).association_request(1);
        while ((*communication).waiting_for_association(1) == false)
            ;
        cout << "[MoistureSensor_INFO: " << depth << "] Initializing communication FINISHED on channel 1" << endl;
        // TODO: unsuccessfull connection to wireless communication
    }

    void setFilename(string filename)
    {
        this->filename = filename;
    }

    int getDepth()
    {
        return depth;
    }

    void readSensorData()
    {
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
            this->line_number++;
            this_thread::sleep_for(chrono::minutes(frequency));
        }
    }

    void readSensorDataOnceAndSend()
    {
        SensorData sensor_data = readSensor();
        // get a close read of the sensor data add [0,0.1] to the moisture level
        sensor_data.moistureLevel += (rand() % 100) / 1000.0;
        if (isWifiConnected())
        {
            sendSensorDataToWifi(sensor_data.moistureLevel, sensor_data.currentTime);
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
        return true; // Assuming wifi is connected for demonstration purposes
    }

    void sendSensorDataToWifi(double moistureLevel, string currentTime)
    {

        (*communication).sendFrame(moistureLevel, currentTime);
    }

    void storeSensorDataInStorage(double moistureLevel, string currentTime)
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

    int frequency;
    int storageUsed;
    int depth;
    int line_number;
    string filename;
    WirelessCommunication *communication;
};

class User_INFO
{
public:
    User_INFO(string data_requested)
    {
        this->data_requested = data_requested;
    }
    User_INFO()
    {
    }
    string data_requested;
};

class Controller
{
public:
    Controller() {}

    Controller(const vector<MoistureSensor> &sensors, WirelessCommunication *w, User_INFO *i) : sensors(sensors)
    {
        communication = w;
        data_user = i;
    }

    void
    getLastNSensorData(int n)
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

                data_user->data_requested = lastNSensorData;
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
                for (MoistureSensor &sensor : sensors)
                {
                    // TODO: send here a Wifi request
                    sensor.readSensorDataOnceAndSend();
                    data_user->data_requested = readLastLine("received_data.txt");
                }
                this_thread::sleep_for(chrono::seconds(7));
            }
        }
    }

    void receiveDataFromWifi()
    {
        while (true)
        {
            while (!(*communication).receiveFrame())
                ;
            string data = (*communication).getFrame();

            ofstream outputFile("received_data.txt", ios::app);
            if (outputFile.is_open())
            {
                string moisture_data = "[Controller_INFO] Moisture Level = " + data;
                outputFile << data << endl;
                cout << moisture_data << endl;
                outputFile.close();
            }
            else
            {
                cout << "[Controller_ERROR] Failed to open output file!" << endl;
            }
        }
    }

private:
    vector<MoistureSensor> sensors;
    WirelessCommunication *communication;
    User_INFO *data_user;
};

class User
{
public:
    User() {}
    User(User_INFO *i) : data_user(i)
    {
    }

    void getLastNSensorData(int n)
    {
        cout << "[User_INFO] Request sensor data " << endl;
        // TODO: communicate through something else
        request_nlast_Data = 1;

        this_thread::sleep_for(chrono::seconds(15));
        cout << "[User_INFO] Sensor Data: " << data_user->data_requested << endl;

        request_nlast_Data = 0;
    }

    void requestSensorData()
    {

        cout << "[User_INFO] Request sensor data " << endl;
        request = 1;
        this_thread::sleep_for(chrono::seconds(5));
        cout << "[User_INFO] Sensor Data: " << data_user->data_requested << endl;

        request = 0;
    }

private:
    User_INFO *data_user;
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

        this->data_requested = User_INFO();
        this->communication = WirelessCommunication();
        this->soilLayer_30 = SoilLayer(30.0, 30.0, 0.25, 50.0);
        this->soilLayer_60 = SoilLayer(60.0, 30.0, 0.21, 45.0);
        this->soilLayer_90 = SoilLayer(90.0, 30.0, 0.19, 37.0);
        this->sensor = MoistureSensor(1, 30, file, &communication);
        this->controller = Controller({sensor}, &communication, &data_requested);
        this->user = User(&data_requested);

        thread communicationThread(&WirelessCommunication::active_scanning, &communication);
        this_thread::sleep_for(chrono::seconds(5));
        sensor.init_communication();

        communicationThread.detach();
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
                // User request irrigation
                cout << "[IrrigationSystem_INFO] Irrigation requested!" << endl;
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
    WirelessCommunication communication;
    User_INFO data_requested;
};

int main()
{

    IrrigationSystem irrigationSystem;
    irrigationSystem.start();

    return 0;
}
