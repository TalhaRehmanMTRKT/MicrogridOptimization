#include "IncludeDataFunction.h"



SMESSRes smess(int& total, int& numMgs, std::vector<MGCRes>& mgVector, prices& price) {


	SMESSRes resultSmess;

	IloEnv env;
	IloModel model(env);

	int T = 24;



#pragma region Decision Variables


	IloNumVarArray Ecapacity(env, numMgs, 0, IloInfinity);
	IloNumVarArray Tcapacity(env, numMgs, 0, IloInfinity);
	IloNumVarArray Hcapacity(env, numMgs, 0, IloInfinity);


	NumVar2D Eshort_real(env, numMgs);
	NumVar2D Esur_real(env, numMgs);
	NumVar2D Pfc(env, numMgs);

	NumVar2D Tshort_real(env, numMgs);
	NumVar2D Tsur(env, numMgs);
	NumVar2D Tsur_real(env, numMgs);
	NumVar2D Tfc(env, numMgs);

	NumVar2D Hshort_real(env, numMgs);
	NumVar2D Hsur_real(env, numMgs);
	NumVar2D Hsur(env, numMgs);


	NumVar2D Echg(env, numMgs);
	NumVar2D Tchg(env, numMgs);
	NumVar2D Hchg(env, numMgs);

	NumVar2D Edischg(env, numMgs);
	NumVar2D Tdischg(env, numMgs);
	NumVar2D Hdischg(env, numMgs);

	NumVar2D HFC(env, numMgs);

	NumVar2D HSOC(env, numMgs);
	NumVar2D TSOC(env, numMgs);
	NumVar2D ESOC(env, numMgs);

	NumVar2D binfc(env, numMgs);




	for (int mg = 0; mg < numMgs; mg++)
	{

		Eshort_real[mg] = IloNumVarArray(env, T, 0, IloInfinity);
		Esur_real[mg] = IloNumVarArray(env, T, 0, IloInfinity);
		Pfc[mg] = IloNumVarArray(env, T, 0, IloInfinity);

		Tshort_real[mg] = IloNumVarArray(env, T, 0, IloInfinity);
		Tsur_real[mg] = IloNumVarArray(env, T, 0, IloInfinity);
		Tsur[mg] = IloNumVarArray(env, T, 0, IloInfinity);
		Tfc[mg] = IloNumVarArray(env, T, 0, IloInfinity);

		Hshort_real[mg] = IloNumVarArray(env, T, 0, IloInfinity);
		Hsur_real[mg] = IloNumVarArray(env, T, 0, IloInfinity);
		Hsur[mg] = IloNumVarArray(env, T, 0, IloInfinity);

		Echg[mg] = IloNumVarArray(env, T, 0, IloInfinity);
		Tchg[mg] = IloNumVarArray(env, T, 0, IloInfinity);
		Hchg[mg] = IloNumVarArray(env, T, 0, IloInfinity);

		Edischg[mg] = IloNumVarArray(env, T, 0, IloInfinity);
		Tdischg[mg] = IloNumVarArray(env, T, 0, IloInfinity);
		Hdischg[mg] = IloNumVarArray(env, T, 0, IloInfinity);

		HFC[mg] = IloNumVarArray(env, T, 0, IloInfinity);

		HSOC[mg] = IloNumVarArray(env, T, 0, IloInfinity);
		TSOC[mg] = IloNumVarArray(env, T, 0, IloInfinity);
		ESOC[mg] = IloNumVarArray(env, T, 0, IloInfinity);

		binfc[mg] = IloNumVarArray(env, T, 0, 1, ILOBOOL);

	}



#pragma endregion


#pragma region Objective Function

	IloExpr objective(env);

	for (int mg = 0; mg < numMgs; mg++)
	{

		for (int t = 0; t < T; t++)
		{
			objective += price.EPriceBuy[t] * Eshort_real[mg][t] - price.EPriceSell[t] * Esur_real[mg][t] + price.TPriceBuy[t] * Tshort_real[mg][t] - price.TPriceSell[t] * Tsur_real[mg][t] + price.HydrogenPrice * Hshort_real[mg][t];

		}

	}


	model.add(IloMinimize(env, objective));

#pragma endregion


#pragma region Constraints


	float fc_eff = 0.6;
	float fc_eff_heat = 0.5;
	int M = 1000000;

	IloExpr c1(env);
	IloExpr c2(env);
	IloExpr c3(env);


	for (int mg = 0; mg < numMgs; mg++)
	{


		c1 += Ecapacity[mg];
		c2 += Tcapacity[mg];
		c3 += Hcapacity[mg];

		//model.add(Hcapacity[1]==400);



		//model.add(Ecapacity[0] == 400);

		for (int t = 0; t < T; t++)
		{
			model.add(Hsur[mg][t] <= mgVector[mg].Esur[t] * mgVector[mg].effEL);
			model.add(Tsur[mg][t] <= mgVector[mg].Esur[t] * mgVector[mg].effEB);
			model.add(Hsur[mg][t] >= 0);
			model.add(Tsur[mg][t] >= 0);


			model.add(Eshort_real[mg][t] + mgVector[mg].Esur[t] + Pfc[mg][t] + Edischg[mg][t] == Esur_real[mg][t] + Echg[mg][t] + mgVector[mg].Esh[t] + Hsur[mg][t] / mgVector[mg].effEL + Tsur[mg][t] / mgVector[mg].effEB);

			model.add(Esur_real[mg][t] >= 0);

			model.add(Pfc[mg][t] == fc_eff * HFC[mg][t]);
			model.add(Tfc[mg][t] == fc_eff_heat * Pfc[mg][t]);

			model.add(Pfc[mg][t] <= M * binfc[mg][t]);
			model.add(Hsur[mg][t] <= M * (1-binfc[mg][t]));



			model.add(Hshort_real[mg][t] + Hsur[mg][t] + Hdischg[mg][t] == mgVector[mg].Hsh[t] + Hsur_real[mg][t] + Hchg[mg][t] + HFC[mg][t]);

			model.add(Hsur_real[mg][t] >= 0);

			model.add(Tshort_real[mg][t] + Tsur[mg][t] + Tdischg[mg][t] + Tfc[mg][t] == mgVector[mg].Tsh[t] + Tsur_real[mg][t] + Tchg[mg][t]);


			if (t == 0)
			{
				model.add(HSOC[mg][t] == HSOC[mg][T - 1] + Hchg[mg][t] - Hdischg[mg][t]);

				model.add(TSOC[mg][t] == TSOC[mg][T - 1] + Tchg[mg][t] * 0.97 - Tdischg[mg][t] / 0.97);

				model.add(ESOC[mg][t] == ESOC[mg][T - 1] + Echg[mg][t] * 0.95 - Edischg[mg][t] / 0.95);

			}
			else
			{
				model.add(HSOC[mg][t] == HSOC[mg][t - 1] + Hchg[mg][t] - Hdischg[mg][t]);
				model.add(TSOC[mg][t] == TSOC[mg][t - 1] + Tchg[mg][t] * 0.97 - Tdischg[mg][t] / 0.97);
				model.add(ESOC[mg][t] == ESOC[mg][t - 1] + Echg[mg][t] * 0.95 - Edischg[mg][t] / 0.95);

			}

			model.add(HSOC[mg][t] <= Hcapacity[mg]);
			model.add(TSOC[mg][t] <= Tcapacity[mg]);
			model.add(ESOC[mg][t] <= Ecapacity[mg]);


		}
	}

	model.add(c1 == total);
	model.add(c2 == total);
	model.add(c3 == total);



	IloCplex cplex(model);
	cplex.setOut(env.getNullStream());

	cplex.solve();

	std::cout << "\t The SMESS is: ";
	std::cout << cplex.getStatus() << endl;

	cout << "Objective Value: " << cplex.getObjValue() << endl;

	resultSmess.Ecapacity.resize(numMgs);

	resultSmess.Tcapacity.resize(numMgs);
	resultSmess.Hcapacity.resize(numMgs);


	for (int mg = 0; mg < numMgs; mg++)
	{
		resultSmess.Ecapacity[mg] = cplex.getValue(Ecapacity[mg]);
		resultSmess.Tcapacity[mg] = cplex.getValue(Tcapacity[mg]);
		resultSmess.Hcapacity[mg] = cplex.getValue(Hcapacity[mg]);

		resultSmess.Eshort.resize(numMgs, std::vector<double>(T));
		resultSmess.Tshort.resize(numMgs, std::vector<double>(T));
		resultSmess.Hshort.resize(numMgs, std::vector<double>(T));

		resultSmess.Esur.resize(numMgs, std::vector<double>(T));
		resultSmess.Tsur.resize(numMgs, std::vector<double>(T));
		resultSmess.Hsur.resize(numMgs, std::vector<double>(T));


		for (int i = 0; i < T; i++)
		{

			resultSmess.Eshort[mg][i] = cplex.getValue(Eshort_real[mg][i]);
			resultSmess.Tshort[mg][i] = cplex.getValue(Tshort_real[mg][i]);
			resultSmess.Hshort[mg][i] = cplex.getValue(Hshort_real[mg][i]);

			resultSmess.Esur[mg][i] = cplex.getValue(Esur_real[mg][i]);
			resultSmess.Tsur[mg][i] = cplex.getValue(Tsur_real[mg][i]);
			resultSmess.Hsur[mg][i] = cplex.getValue(Hsur_real[mg][i]);

		}
	}



	std::string SRes = "D:\\MasterWork\\Codes\\MicrogridOptimization\\BenderDecomposition\\SMESS";

	for (int mg = 0; mg < numMgs; mg++)
	{
		// Construct the full file path
		std::string filePath = SRes + "\\" + std::to_string(mg) + "_mgSMESS.csv";

		// Open the file for writing
		std::ofstream outputFile(filePath);

		if (outputFile.is_open()) {
			// Write the header

			// Determine the size of the vectors
			int dataSize = T;

			outputFile << "Capacity," << resultSmess.Ecapacity[mg] << "," << resultSmess.Tcapacity[mg] << "," << resultSmess.Hcapacity[mg] << "\n";
			outputFile << "Time,Eshort,Eshort_real,Esur,Esur_real,Pfc,ptB,pEL,Echg,Edisch,Esoc\n";

			for (int i = 0; i < dataSize; i++)
			{
				outputFile << i << "," << mgVector[mg].Esh[i] << "," 
					<< cplex.getValue(Eshort_real[mg][i]) << "," 
					<< mgVector[mg].Esur[i] << "," 
					<< cplex.getValue(Esur_real[mg][i]) 
					<< "," << cplex.getValue(Pfc[mg][i]) << ","
					<< cplex.getValue(Tsur[mg][i])/mgVector[mg].effEB << ","
					<< cplex.getValue(Hsur[mg][i])/mgVector[mg].effEL << ","
					<< cplex.getValue(Echg[mg][i]) << ","
					<< cplex.getValue(Edischg[mg][i]) << ","
					<< cplex.getValue(ESOC[mg][i]) << "\n";
			}

			outputFile << endl << endl;



			outputFile << "Time,Tshort_real,Tshort,Tsur_real,Tsur,Tfc,Tchg,Tdischg,Tsoc\n";

			for (int i = 0; i < dataSize; i++)
			{
				outputFile << i << "," << cplex.getValue(Tshort_real[mg][i]) << "," << mgVector[mg].Tsh[i] << "," << cplex.getValue(Tsur_real[mg][i]) << "," << cplex.getValue(Tsur[mg][i]) << "," << cplex.getValue(Tfc[mg][i]) << "," << cplex.getValue(Tchg[mg][i]) << "," << cplex.getValue(Tdischg[mg][i]) << "," << cplex.getValue(TSOC[mg][i]) << "\n";
			}

			outputFile << endl <<endl;

			outputFile << "Time,Hshort,Hshort_real,Hsur_real,Hsur,Hchg,Hdischg,HFC,Hsoc\n";

			for (int i = 0; i < dataSize; i++)
			{
				outputFile << i << "," <<mgVector[mg].Hsh[i]<<"," << cplex.getValue(Hshort_real[mg][i]) << "," << cplex.getValue(Hsur_real[mg][i]) << "," << cplex.getValue(Hsur[mg][i]) << "," << cplex.getValue(Hchg[mg][i]) << "," << cplex.getValue(Hdischg[mg][i]) << "," << cplex.getValue(HFC[mg][i]) << "," << cplex.getValue(HSOC[mg][i]) << "\n";
			}


			std::cout << "Results stored successfully in: " << filePath << std::endl;
		}
		else {
			std::cerr << "Error: Unable to open file for writing." << std::endl;
		}

	}





	env.end();

#pragma region end



	return resultSmess;
}