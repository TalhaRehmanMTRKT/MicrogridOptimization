#include "simulation.h"
#include <numeric>
#include <fstream>



using namespace std;
Simulation::Simulation(int numEvs, double mta,
	double stdta,
	double mtd,
	double stdtd,
	double mDN,
	double stdDN,
	double mDD,
	double stdDD
) {
	meanDailyDistance = std::log(mDD * mDD / std::sqrt(mDD * mDD + stdDD * stdDD));
	stdDevDailyDistance = std::sqrt(std::log(1 + stdDD * stdDD / (mDD * mDD)));

	_meanDailyDistance = mDD;
	_stdDevDailyDistance = stdDD;
	_meanDistanceNeigbour = mDN;
	_stdDevdistanceNeigbour = stdDN;

	meanDistanceNeigbour = std::log(mDN * mDN / std::sqrt(mDN * mDN + stdDN * stdDN));
	stdDevdistanceNeigbour = std::sqrt(std::log(1 + stdDN * stdDN / (mDN * mDN)));

	this->numEvs = numEvs;
	this->meanArrival = mta;
	this->stdDevArrival = stdta;
	this->meanDeparture = mtd;
	this->stdDevDeparture = stdtd;

	// Set the seed for reproducible results
}

void Simulation::simulateArrivalAndDepartureTimes()
{
	std::default_random_engine generator;

	// Simulate EV arrival and departure times for all days and all simulations
	evArrivalTimes.resize(numDays, std::vector<std::vector<int>>(numSimulations, std::vector<int>(numEvs)));
	evDepartureTimes.resize(numDays, std::vector<std::vector<int>>(numSimulations, std::vector<int>(numEvs)));

	for (int day = 0; day < numDays; ++day) {
		for (int sim = 0; sim < numSimulations; ++sim) {
			std::normal_distribution<double> arrivalDistribution(meanArrival, stdDevArrival);
			std::normal_distribution<double> departureDistribution(meanDeparture, stdDevDeparture);

			for (int ev = 0; ev < numEvs; ++ev) {

				// Ensure departure time is greater than arrival time
				do {
					evArrivalTimes[day][sim][ev] = static_cast<int>(std::min(std::max(arrivalDistribution(generator), 0.0), 23.0));
					evDepartureTimes[day][sim][ev] = static_cast<int>(std::min(std::max(departureDistribution(generator), 0.0), 23.0));
				} while (evDepartureTimes[day][sim][ev] <= evArrivalTimes[day][sim][ev]);
			}

		}
	}
}
void Simulation::storeEVData(std::default_random_engine& generator) {
	// Resize the 2D arrays
	evArrival.resize(numDays, std::vector<int>(numEvs));
	evDeparture.resize(numDays, std::vector<int>(numEvs));

	// Choose 10 random EVs for each day
	for (int day = 0; day < numDays; ++day) {
		std::vector<int> allEVs(numEvs);
		std::iota(allEVs.begin(), allEVs.end(), 0);
		std::shuffle(allEVs.begin(), allEVs.end(), generator);

		// Store data for the selected EVs
		for (int i = 0; i < numEvs; ++i) {
			int evIdx = allEVs[i];
			evArrival[day][i] = evArrivalTimes[day][evIdx][i];
			evDeparture[day][i] = evDepartureTimes[day][evIdx][i];
		}
	}
	evArrivalTimes.clear();
	evDepartureTimes.clear();
}

// Implementation of simulateDailyDrivingDistances
void Simulation::simulateDailyDrivingDistances() {
	std::default_random_engine generator;

	// Simulate daily driving distances for all days, all simulations, and all vehicles
	simulatedDailyDistances.resize(numDays, std::vector<std::vector<double>>(numSimulations, std::vector<double>(numEvs)));

	for (int day = 0; day < numDays; ++day) {
		std::lognormal_distribution<double> distanceDistribution(meanDailyDistance, stdDevDailyDistance);

		for (int sim = 0; sim < numSimulations; ++sim) {
			for (int vehicle = 0; vehicle < numEvs; ++vehicle) {
				do {
					simulatedDailyDistances[day][sim][vehicle] = distanceDistribution(generator);
				} while (simulatedDailyDistances[day][sim][vehicle] > evRatedMileage[vehicle]);
			}
		}
	}

}
void Simulation::calculateSOCini() {
	SOCini.resize(numDays, std::vector<double>(numEvs));

	for (int day = 0; day < numDays; ++day) {
		for (int ev = 0; ev < numEvs; ++ev) {
			double calculatedSOC = socmax - v * dailydistances[day][ev] / evRatedMileage[ev];

			// Check if the calculated SOC is below socmin, and assign socmin if needed
			SOCini[day][ev] = (calculatedSOC < socmin) ? socmin : calculatedSOC;
		}
	}
}

// Implementation of storeDailyDistanceData
void Simulation::storeDailyDistanceData(std::default_random_engine& generator) {
	// Resize the 2D array
	dailydistances.resize(numDays, std::vector<double>(numEvs));

	// Choose 10 random vehicles for each day
	for (int day = 0; day < numDays; ++day) {
		std::vector<int> allVehicles(numEvs);
		std::iota(allVehicles.begin(), allVehicles.end(), 0);
		std::shuffle(allVehicles.begin(), allVehicles.end(), generator);

		// Store data for the selected vehicles
		for (int i = 0; i < numEvs; ++i) {
			int vehicleIdx = allVehicles[i];
			dailydistances[day][i] = simulatedDailyDistances[day][vehicleIdx][i];
		}
	}
	simulatedDailyDistances.clear();
}





// Implementation of simulateDailyNeighborDistances
void Simulation::simulateDailyNeighborDistances() {
	std::default_random_engine generator;

	// Simulate daily neighbor distances for all days, all simulations, and all vehicles
	simulatedDailyNeighborDistances.resize(numDays, std::vector<std::vector<double>>(numSimulations, std::vector<double>(numEvs)));

	for (int day = 0; day < numDays; ++day) {
		std::lognormal_distribution<double> distanceDistribution(meanDistanceNeigbour, stdDevdistanceNeigbour);

		for (int sim = 0; sim < numSimulations; ++sim) {
			double commonNeighborDistance = distanceDistribution(generator);

			for (int vehicle = 0; vehicle < numEvs; ++vehicle) {
				// Ensure neighbor distance is less than the rated mileage
				simulatedDailyNeighborDistances[day][sim][vehicle] = commonNeighborDistance;
			}
		}
	}
}


void Simulation::storeDailyNeighborDistances(std::default_random_engine& generator) {
	// Resize the 2D array
	dailyNeighborDistances.resize(numDays, std::vector<double>(numEvs));

	// Choose 10 random vehicles for each day
	for (int day = 0; day < numDays; ++day) {
		std::vector<int> allDays(numDays);
		std::iota(allDays.begin(), allDays.end(), 0);
		std::shuffle(allDays.begin(), allDays.end(), generator);
		// Store data for the selected vehicles
		int dayIdx = allDays[day];
		for (int i = 0; i < numEvs; ++i) {
			dailyNeighborDistances[day][i] = simulatedDailyNeighborDistances[day][dayIdx][i];
		}
	}
	simulatedDailyNeighborDistances.clear();
}


void Simulation::calculateResLoad() {
	SOCreq.resize(numDays, std::vector<double>(numEvs));
	loadres.resize(numDays, std::vector<double>(numEvs));
	for (int day = 0; day < numDays; ++day) {
		for (int ev = 0; ev < numEvs; ++ev) {
			double socreq = 0.1 + dailyNeighborDistances[day][ev] / evRatedMileage[ev];
			SOCreq[day][ev] = socreq;
			loadres[day][ev] = socreq * evBatteryCapacity[ev];
		}
	}
}



void Simulation::loadEVData(const std::string& capacityFile, const std::string& mileageFile) {
	std::ifstream capacityStream(capacityFile);
	std::ifstream mileageStream(mileageFile);

	if (!capacityStream.is_open() || !mileageStream.is_open()) {
		std::cerr << "Error: Unable to open one or both of the input files.\n";
		return;
	}

	// Load EV battery capacity and rated mileage
	evBatteryCapacity.resize(numEvs);
	evRatedMileage.resize(numEvs);

	for (int ev = 0; ev < numEvs; ++ev) {
		capacityStream >> evBatteryCapacity[ev];
		mileageStream >> evRatedMileage[ev];
	}

	capacityStream.close();
	mileageStream.close();
}

void Simulation::printEVDataForDay(int day) {
	if (day < 0 || day >= numDays) {
		std::cerr << "Invalid day number. Please provide a valid day.\n";
		return;
	}

	int n = 5; // Number of EVs to display
	cout << "\t---- Selected 5 Evs Data Being Displayed ------" << endl;
	std::cout << "Day: " << day << "\n";
	std::cout << "EV arrivals(h): ";
	for (int ev = 0; ev < n; ++ev) {
		std::cout << evArrival[day][ev];
		if (ev < numEvs - 1) {
			std::cout << ", ";
		}
	}
	std::cout << "\n";
	std::cout << "EV departures(h): ";
	for (int ev = 0; ev < n; ++ev) {
		std::cout << evDeparture[day][ev];
		if (ev < n - 1) {
			std::cout << ", ";
		}
	}
	std::cout << "\n";
	std::cout << "EV Daily Distance(Km): ";
	for (int ev = 0; ev < n; ++ev) {
		std::cout << dailydistances[day][ev];
		if (ev < n - 1) {
			std::cout << ", ";
		}
	}
	std::cout << "\n";
	std::cout << "SOC ini(%): ";
	for (int ev = 0; ev < n; ++ev) {
		std::cout << SOCini[day][ev];
		if (ev < n - 1) {
			std::cout << ", ";
		}
	}
	std::cout << "\n";
	std::cout << "Daily Neigbour(Km): ";
	for (int ev = 0; ev < n; ++ev) {
		std::cout << dailyNeighborDistances[day][ev];
		if (ev < n - 1) {
			std::cout << ", ";
		}
	}
	std::cout << "\n";
	std::cout << "Load Resilience(KWh): ";
	for (int ev = 0; ev < n; ++ev) {
		std::cout << loadres[day][ev];
		if (ev < n - 1) {
			std::cout << ", ";
		}
	}
	cout << "\n---------------------------------------\n" << endl;

}

// Function to save private member variables to files
void Simulation::saveParameters(const std::string& filePath) {
	// Save private variables to txt file
	std::string simfile = filePath + "\\SimulationParameters.txt";
	cout << simfile << endl;

	std::ofstream txtFile(simfile);
	if (!txtFile.is_open()) {
		std::cerr << "Error opening txt file!" << std::endl;
		return; // or throw an exception
	}
	txtFile << "numDays: " << numDays << "\n";
	txtFile << "numSimulations: " << numSimulations << "\n";
	txtFile << "socmax: " << socmax << "\n";
	txtFile << "socmin: " << socmin << "\n";
	txtFile << "v: " << v << "\n";
	txtFile << "numEvs: " << numEvs << "\n";
	txtFile << "meanArrival: " << meanArrival << "\n";
	txtFile << "stdDevArrival: " << stdDevArrival << "\n";
	txtFile << "meanDeparture: " << meanDeparture << "\n";
	txtFile << "stdDevDeparture: " << stdDevDeparture << "\n";
	txtFile << "meanDailyDistance: log/orig " << meanDailyDistance << " , " << _meanDailyDistance << "\n";
	txtFile << "stdDevDailyDistance: log/orig " << stdDevDailyDistance << " , " << _stdDevDailyDistance << "\n";
	txtFile << "meanDistanceNeigbour: log/orig " << meanDistanceNeigbour << " , " << _meanDistanceNeigbour << "\n";
	txtFile << "stdDevdistanceNeigbour: log/orig " << stdDevdistanceNeigbour << " , " << _stdDevdistanceNeigbour << "\n";
	txtFile << "EVs specifications:\n";
	txtFile << "EVs Bat Capacity: ";
	for (size_t i = 0; i < evBatteryCapacity.size(); ++i) {
		txtFile << evBatteryCapacity[i] << ", ";
	}
	txtFile << "\n";
	txtFile << "EVs Range : ";
	for (size_t i = 0; i < evRatedMileage.size(); ++i) {
		txtFile << evRatedMileage[i] << ", ";
	}
	txtFile.close();

	// Save private variables to csv file
	std::ofstream csvFile(filePath + "\\SimulationData.csv");
	csvFile << "D, EVnumber, Arrival, Departure, Socini, Dailydistance, DailyNeigbour, socreq, loadres\n";
	for (size_t i = 0; i < numDays; ++i) {
		for (size_t j = 0; j < numEvs; ++j) {
			csvFile << i << ", " << j << ", " << evArrival[i][j] << ", " << evDeparture[i][j] << ", "
				<< SOCini[i][j] << ", " << dailydistances[i][j] << "," << dailyNeighborDistances[i][j] << "," << SOCreq[i][j] << "," << loadres[i][j] << "\n";
		}
	}
	cout << "Saved" << endl;
	csvFile.close();
}
std::vector<float> Simulation::getMilages() const {
	return evRatedMileage;
}
std::vector<float> Simulation::getCapacities() const {
	return evBatteryCapacity;
}

std::vector<int> Simulation::getDayArrivalTime(int day) const
{
	return evArrival[day];
}
std::vector<int> Simulation::getDayDepartureTime(int day) const
{
	return evDeparture[day];
}
std::vector<double> Simulation::getDaySOCIni(int day) const {
	return SOCini[day];

}
std::vector<double> Simulation::getDayResLoad(int day) const {
	return loadres[day];
}


