#include "DataStore.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

// Initialize static members
vector<vector<float>> DataStore::ambient_temperature_2d;
vector<vector<float>> DataStore::wind_speed_2d;
vector<vector<float>> DataStore::solar_irradiance_2d;
vector<vector<float>> DataStore::Price_2d;
vector<vector<int>> DataStore::Load_2d;
const double DataStore::k = -3.7e-3;  // Temperature coefficient
const double DataStore::Iref = 1000.0;  // Reference solar irradiance in W/m^2
const double DataStore::Tref = 25.0;   // Reference temperature in degrees Celsius

void DataStore::Populate_Parameters(const std::string& csvpath) {
    ifstream file(csvpath);

    if (!file.is_open()) {
        cerr << "Error opening file: " << csvpath << endl;
        return;
    }

    string line;
    getline(file, line); // Read and discard the header

    int day = 0;
    vector<float> ambient_temperature_day, wind_speed_day, solar_irradiance_day, Price_day;
    vector<int> Load_day;

    while (getline(file, line)) {
        istringstream row(line);
        string token;

        // Read the columns
        vector<string> columns;
        while (getline(row, token, ',')) {
            columns.push_back(token);
        }

        // Assuming the columns are in the correct order, adjust accordingly if needed
        float ws50m = stof(columns[5]);
        float allsky_sfc_sw_dwn = stof(columns[6]);
        float t2m = stof(columns[7]);
        float price = stof(columns[10]);
        int load = stoi(columns[9]);

        // Store the values in the corresponding vectors
        wind_speed_day.push_back(ws50m);
        solar_irradiance_day.push_back(allsky_sfc_sw_dwn);
        ambient_temperature_day.push_back(t2m);
        Load_day.push_back(load);
        Price_day.push_back(price);

        // Check if 24 hours data is collected, then move to the next day
        if (Load_day.size() == 24) {
            ambient_temperature_2d.push_back(ambient_temperature_day);
            wind_speed_2d.push_back(wind_speed_day);
            solar_irradiance_2d.push_back(solar_irradiance_day);
            Load_2d.push_back(Load_day);
            Price_2d.push_back(Price_day);

            // Reset vectors for the next day
            ambient_temperature_day.clear();
            wind_speed_day.clear();
            solar_irradiance_day.clear();
            Load_day.clear();
            Price_day.clear();

            day++;
        }
    }

    file.close();
}

