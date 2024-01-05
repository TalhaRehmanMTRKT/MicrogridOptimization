#ifndef SIMULATION_H
#define SIMULATION_H

#include <iostream>
#include <vector>
#include <random>

class Simulation {
public:
    Simulation(int numEvs, double mta,
        double stdta,
        double mtd,
        double stdtd,
        double mDN,
        double stdDN,
        double mDD,
        double stdDD
    );
    void loadEVData(const std::string& capacityFile, const std::string& mileageFile);
    void simulateArrivalAndDepartureTimes();
    void storeEVData(std::default_random_engine& generator);
    void simulateDailyDrivingDistances();
    void storeDailyDistanceData(std::default_random_engine& generator);
    void storeDailyNeighborDistances(std::default_random_engine& generator);
    void calculateSOCini();
    void saveParameters(const std::string& filePath);
    void printEVDataForDay(int day);
    void simulateDailyNeighborDistances();
    void calculateResLoad();
    std::vector<float> getMilages() const;
    std::vector<float> getCapacities() const;
    std::vector<int> getDayArrivalTime(int day) const;
    std::vector<int> getDayDepartureTime(int day) const;
    std::vector<double> getDaySOCIni(int day) const;
    std::vector<double> getDayResLoad(int day) const;


private:
    static const int numDays = 1; // You can adjust the number of days as needed
    static const int numSimulations = 1000;
    float socmax = 0.9;
    const float socmin = 0.1;  // Replace with your desired socmin value
    int v = 1;
    int numEvs;
    double meanArrival;
    double stdDevArrival;
    double meanDeparture;
    double stdDevDeparture;
    double meanDistanceNeigbour;
    double stdDevdistanceNeigbour;
    double meanDailyDistance;
    double stdDevDailyDistance;
    double _meanDistanceNeigbour;
    double _stdDevdistanceNeigbour;
    double _meanDailyDistance;
    double _stdDevDailyDistance;


    std::vector<std::vector<std::vector<int>>> evArrivalTimes; // 3D array: [numDays][numSimulations][numEvs]
    std::vector<std::vector<std::vector<int>>> evDepartureTimes;
    std::vector<std::vector<int>> evArrival; // 3D array: [numDays][numSimulations][numEvs]
    std::vector<std::vector<int>> evDeparture;
    std::vector<float> evBatteryCapacity;
    std::vector<float> evRatedMileage;
    std::vector<std::vector<std::vector<double>>> simulatedDailyDistances;
    std::vector<std::vector<std::vector<double>>> simulatedDailyNeighborDistances;
    std::vector<std::vector<double>> dailydistances;
    std::vector<std::vector<double>> dailyNeighborDistances;
    std::vector<std::vector<double>> SOCini;
    std::vector<std::vector<double>> SOCreq;
    std::vector<std::vector<double>> loadres;


};



#endif // SIMULATION_H