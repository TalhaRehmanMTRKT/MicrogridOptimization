#include "IncludeDataFunction.h"
#include "simulation.h"


Simulation  runSimulation(int numEvs, double meanta, double stdta, double meantd, double stdtd,
	double meanDN, double stdDN, double meanDD, double stdDD,
	const std::string& DataFolder,
	const std::string& resultsFolder) {

	// Assuming std::default_random_engine generator(25); is not needed here and can be created locally.

	std::default_random_engine generator(25);

	std::string mileageFile = DataFolder + "\\Milage.txt";
	std::string capfile = DataFolder + "\\EvCap.txt";
	Simulation simulation(numEvs, meanta, stdta, meantd, stdtd, meanDN, stdDN, meanDD, stdDD);
	simulation.loadEVData(capfile, mileageFile);
	cout << "\t---- Simulating Arrival and Departure ----" << std::endl;
	simulation.simulateArrivalAndDepartureTimes();
	simulation.storeEVData(generator);
	cout << "\t---- Simulating DailyDriving ----" << std::endl;
	simulation.simulateDailyDrivingDistances();
	simulation.storeDailyDistanceData(generator);
	cout << "\t---- Simulating DailyNeighbor ----" << std::endl;
	simulation.simulateDailyNeighborDistances();
	simulation.storeDailyNeighborDistances(generator);
	cout << "\t---- Calculating SOCini ----" << std::endl;
	simulation.calculateSOCini();
	cout << "\t---- Calculating ResLoad ----" << std::endl;
	simulation.calculateResLoad();
	simulation.saveParameters(resultsFolder);
	std::cout << "\t---- Simulation Success ----" << std::endl;
	//simulation.printEVDataForDay(0);
	return simulation;
}


MGCRes MGC(int& id,int& NumEVs, vector<double>TPriceBuy, vector<double>TPriceSell, vector<double>EPriceBuy, vector<double>EPriceSell, vector<double>& RDG, vector<double>& Eload, vector<double>& Tload, vector<double>& Cload, vector<double>& Hload)

{

	int T = 24;



	vector<double> PriceEgridbuy = EPriceBuy;
	vector<double> PriceEgridsell = EPriceSell;


	vector<double> PriceTgridbuy = TPriceBuy;
	vector<double> PriceTgridsell = TPriceSell;


	vector<double> loadE = Eload;
	vector<double> loadT = Tload;
	vector<double> loadC = Cload;
	vector<double> loadH = Hload;

	int EVs = NumEVs;


	vector<double>Renewable = RDG;
	const double meanta = 8.5;
	const double stdta = 3.3;
	const double meantd = 17.5;
	const double stdtd = 3.24;
	int numEvs = 20;
	const double meanDD = 34.1;
	const double stdDD = 14.4;
	const double meanDN = 17.59;
	const double stdDN = 5.93;

	std::string datafolder = "D:\\MasterWork\\Codes\\MicrogridOptimization\\BenderDecomposition\\bd";
	std::string main_saving_path = "D:\\MasterWork\\Codes\\MicrogridOptimization\\BenderDecomposition\\bd";


	Simulation sim1 = runSimulation(EVs, meanta, stdta, meantd, stdtd, meanDN, stdDN, meanDD, stdDD, datafolder, main_saving_path);


	IloEnv env;
	IloModel model(env);

#pragma region Decision Variables

	IloNumVarArray Eshort(env, T, 0, IloInfinity);
	IloNumVarArray Esur(env, T, 0, IloInfinity);

	IloNumVarArray Tshort(env, T, 0, IloInfinity);
	IloNumVarArray Tsur(env, T, 0, IloInfinity);

	IloNumVarArray Hshort(env, T, 0, IloInfinity);




	IloNumVarArray pEC(env, T, 0, IloInfinity);
	IloNumVarArray pEB(env, T, 0, IloInfinity);
	IloNumVarArray pEL(env, T, 0, IloInfinity);

	IloNumVarArray hEL(env, T, 0, IloInfinity);
	
	
	IloNumVarArray tEB(env, T, 0, IloInfinity);
	IloNumVarArray tAC(env, T, 0, IloInfinity);

	IloNumVarArray cAC(env, T, 0, IloInfinity);
	IloNumVarArray cEC(env, T, 0, IloInfinity);

	IloNumVarArray Bin(env, T, 0,1, ILOBOOL);



	float effEB = 0.9;
	float effEC = 2.5;
	float effEL = 0.75;

	float effAC = 1.3;
	int M = 1000000;





	IloExpr objective(env);


	for (int t = 0; t < T; t++)
	{
		objective += EPriceBuy[t] * Eshort[t]   - EPriceSell[t] * Esur[t]   +   TPriceBuy[t] * Tshort[t] - TPriceSell[t] * Tsur[t] + 100 * Hshort[t];

	}

	model.add(IloMinimize(env, objective));

	for (int t = 0; t < T; t++)
	{
		model.add(Eshort[t] + Renewable[t]  == Eload[t] + pEC[t] + pEB[t] + pEL[t]  + Esur[t]);
		model.add(Tshort[t] + tEB[t] == Tload[t] + tAC[t] + Tsur[t]);
		model.add( cAC[t] + cEC[t]  == Cload[t]);
		model.add( Hshort[t] + hEL[t] == Hload[t]);

		model.add(tEB[t] == effEB * pEB[t]);
		model.add(cEC[t] == effEC * pEC[t]);
		model.add(cAC[t] == effAC * tAC[t]);
		model.add(hEL[t] == effEL * pEL[t]);



		model.add(pEL[t] <= M * Bin[t]);
		model.add(pEB[t] <= M * Bin[t]);
		model.add(Eshort[t] <= M * (1 - Bin[t]));
		model.add(Hshort[t]>=0);
	}

	IloCplex cplex(model);
	cplex.setOut(env.getNullStream());

	cplex.solve();

	std::cout << "\t The MGC is: ";
	std::cout << cplex.getStatus() << endl;



	MGCRes results;

	for (int t = 0; t < T; t++)
	{
		results.Esh.push_back(cplex.getValue(Eshort[t]));
		double value = cplex.getValue(Hshort[t]);
		results.Hsh.push_back(std::max(value, 0.0));
		results.Esur.push_back(cplex.getValue(Esur[t]));
		results.Tsh.push_back(cplex.getValue(Tshort[t]));
		results.Tsurr.push_back(cplex.getValue(Tsur[t]));

	}

	results.effEB = effEB;
	results.effEL = effEL;



	std::string LocalRes = "D:\\MasterWork\\Codes\\MicrogridOptimization\\BenderDecomposition\\Local";


	// Construct the full file path
	std::string filePath = LocalRes + "\\" + std::to_string(id) + "_mg.csv";

	// Open the file for writing
	std::ofstream outputFile(filePath);

	if (outputFile.is_open()) {
		// Write the header
		outputFile << "Time,Esh,Esur,Tsh,Tsurr,Hsh,Hel,Hload,cAC,CEC,pEB,pEC,pEL,RDG,Eload,tEB,tAC,tLoad\n";

		// Determine the size of the vectors
		std::size_t dataSize = results.Esh.size();

		// Write the data to the file
		for (std::size_t i = 0; i < dataSize; ++i) {

			outputFile << i << "," << results.Esh[i] << "," << results.Esur[i] << "," << results.Tsh[i] << "," << results.Tsurr[i] 
				<< "," << results.Hsh[i] << "," << cplex.getValue(hEL[i]) << "," << Hload[i]
				<< "," << cplex.getValue(cAC[i]) << "," << cplex.getValue(cEC[i]) 
				<< "," << cplex.getValue(pEB[i]) << "," << cplex.getValue(pEC[i]) << "," << cplex.getValue(pEL[i])
				<< "," << Renewable[i] << "," << Eload[i] 
			<< "," << cplex.getValue(tEB[i]) << "," << cplex.getValue(tAC[i]) << "," << Tload[i] << "\n";
		}

		// Close the file
		outputFile.close();

		std::cout << "Results stored successfully in: " << filePath << std::endl;
	}
	else {
		std::cerr << "Error: Unable to open file for writing." << std::endl;
	}



















	
	env.end();


	return results;
}

void printMGCRes(const MGCRes& mgcRes)
{
	// Print Esh
	std::cout << "Esh: ";
	for (const auto& value : mgcRes.Esh)
	{
		std::cout << value << " ";
	}
	std::cout << std::endl;

	// Print Esur
	std::cout << "Esur: ";
	for (const auto& value : mgcRes.Esur)
	{
		std::cout << value << " ";
	}
	std::cout << std::endl;

	// Print Tsh
	std::cout << "Tsh: ";
	for (const auto& value : mgcRes.Tsh)
	{
		std::cout << value << " ";
	}
	std::cout << std::endl;

	// Print Hsh
	std::cout << "Hsh: ";
	for (const auto& value : mgcRes.Hsh)
	{
		std::cout << value << " ";
	}
	std::cout << std::endl;

	// Print Tsurr
	std::cout << "Tsurr: ";
	for (const auto& value : mgcRes.Tsurr)
	{
		std::cout << value << " ";
	}
	std::cout << std::endl;
}