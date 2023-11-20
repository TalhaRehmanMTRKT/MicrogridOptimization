#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <ilcplex/ilocplex.h>

class DataStore {
public:
    static std::vector<std::vector<float>> ambient_temperature_2d;
    static std::vector<std::vector<float>> wind_speed_2d;
    static std::vector<std::vector<float>> solar_irradiance_2d;
    static std::vector<std::vector<float>> Price_2d;
    static std::vector<std::vector<int>> Load_2d;
    static void Populate_Parameters(const std::string& csvpath);

    // Constants
    static const double k;     // Temperature coefficient
    static const double Iref;  // Reference solar irradiance in W/m^2
    static const double Tref;  // Reference temperature in degrees Celsius

};