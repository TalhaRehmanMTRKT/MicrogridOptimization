
#include "IncludeDataFunction.h"


double calculateCRF(double int_r, int n) {
	double numerator = int_r * pow(1 + int_r, n);
	double denominator = pow(1 + int_r, n) - 1;
	return numerator / denominator;
}



double centralized() {



#pragma region Data
	int T = 24;
	int numMgs = 2;
	int seasons = 2;

	float gridbuy[24] = { 138, 139, 143, 149, 150, 152, 155, 158, 160, 154, 153, 153, 152, 150, 149, 149, 154, 156, 163, 164, 164, 160, 150, 148 };
	float gridsell[24] = { 128, 129, 133, 139, 140, 142, 145, 148, 150, 144, 143, 143, 142, 140, 139, 139, 144, 146, 153, 154, 154, 150, 140, 138 };


	float*** rdg = new float** [numMgs];
	rdg[0] = new float* [seasons];
	rdg[1] = new float* [seasons];

	rdg[0][0] = new float[T] { 0, 0, 0, 0, 0, 0, 0, 6, 9, 10, 13, 18, 23, 25, 24, 21, 18, 8, 0, 0, 0, 0, 0 };//renewable at MG;
	rdg[0][1] = new float[T] {  0, 0, 0, 0, 0, 0, 0, 6, 9, 10, 13, 18, 23, 25, 24, 21, 18, 8, 0, 0, 0, 0, 0 };//renewable at MG;

	rdg[1][0] = new float[T] { 0, 0, 0, 0, 0, 0, 0, 6, 9, 10, 13, 18, 23, 25, 24, 21, 18, 8, 0, 0, 0, 0, 0 };//renewable at MG;
	rdg[1][1] = new float[T] {3149, 2905, 2494, 2045, 1646, 1397, 1192, 1258, 1552, 2112, 2528, 2882, 3221, 2816, 1951, 1430, 931, 344, 316, 471, 732, 959, 953, 1086}
	;//renewable at MG;


	float*** Eload = new float** [numMgs];
	Eload[0] = new float* [seasons];
	Eload[1] = new float* [seasons];

	Eload[0][0] = new float[T] { 192, 187, 189, 188, 200, 224, 247, 305, 535, 673, 670, 651, 320, 343, 585, 603, 600, 557, 424, 356, 317, 299, 247, 216 };
	Eload[0][1] = new float[T] { 369, 345, 329, 351, 381, 372, 470, 454, 363, 371, 373, 416, 361, 362, 357, 351, 357, 391, 464, 467, 428, 417, 414, 400};

	Eload[1][0] = new float[T] { 369, 345, 329, 351, 381, 372, 470, 454, 363, 371, 373, 416, 361, 362, 357, 351, 357, 391, 464, 467, 428, 417, 414, 400};
	Eload[1][1] = new float[T] { 192, 187, 189, 188, 200, 224, 247, 305, 535, 673, 670, 651, 320, 343, 585, 603, 600, 557, 424, 356, 317, 299, 247, 216 };


	float chgeff = 0.95;
	float diseff = 0.95;

	int M = 100000;

	// Capacity Constants
	int EcapMax = 2000;

	int cinv = 100;
	int com_fixed = 5;

	// Variable Cost
	int c_var = 0.0001;

	double crf = calculateCRF(0.08, 5);


	double investcost = (cinv + com_fixed) * crf;


#pragma endregion


	IloEnv env;
	IloModel model(env);

#pragma region Decision Variables
	NumVar3D Pgridbuy(env, numMgs);

	NumVar3D Pgridsell(env, numMgs);

	NumVar3D pdg(env, numMgs);


	NumVar3D mgsend(env, numMgs);
	NumVar3D mgrec(env, numMgs);


	NumVar3D Pchg(env, numMgs);
	NumVar3D Pdischg(env, numMgs);
	NumVar3D Psoc(env, numMgs);


	NumVar2D cap_share(env, numMgs);



	NumVar3D bin(env, numMgs);



	for (int i = 0; i < numMgs; i++)
	{

		Pgridbuy[i] = NumVar2D(env, seasons);
		Pgridsell[i] = NumVar2D(env, seasons);

		pdg[i] = NumVar2D(env, seasons);

		Pchg[i] = NumVar2D(env, seasons);
		Pdischg[i] = NumVar2D(env, seasons);
		Psoc[i] = NumVar2D(env, seasons);

		bin[i] = NumVar2D(env, seasons);

		mgsend[i] = NumVar2D(env, numMgs);
		mgrec[i] = NumVar2D(env, numMgs);

		cap_share[i] = IloNumVarArray(env, seasons, 0, IloInfinity);


		for (int j = 0; j < seasons; j++)
		{


			Pgridbuy[i][j] = IloNumVarArray(env, T, 0, IloInfinity);
			Pgridsell[i][j] = IloNumVarArray(env, T, 0, IloInfinity);

			Pchg[i][j] = IloNumVarArray(env, T, 0, IloInfinity);
			Pdischg[i][j] = IloNumVarArray(env, T, 0, IloInfinity);
			Psoc[i][j] = IloNumVarArray(env, T, 0, IloInfinity);

			bin[i][j] = IloNumVarArray(env, T, 0, 1, ILOBOOL);

			mgsend[i][j] = IloNumVarArray(env, T, 0, IloInfinity);
			mgrec[i][j] = IloNumVarArray(env, T, 0, IloInfinity);

			pdg[i][j] = IloNumVarArray(env, T, 0, IloInfinity);

		}

	}

#pragma endregion


#pragma region objective

	IloExpr objective(env);



	for (int d = 0; d < seasons; d++)
	{
		for (int i = 0; i < numMgs; i++)
		{
			for (int t = 0; t < T; t++)

			{

				objective += Pgridbuy[i][d][t] * gridbuy[t] - Pgridsell[i][d][t] * gridsell[t];

			}

			objective += (365 / seasons) * cap_share[i][d] * investcost;

		}
	}



	model.add(IloMinimize(env, objective));


#pragma endregion



#pragma region Constrints


	for (int d = 0; d < seasons; d++)
	{

		IloExpr ce(env);

		for (int i = 0; i < numMgs; i++)
		{


			for (int t = 0; t < T; t++)
			{
				// Electrical Power balance constraint
				model.add(Pgridbuy[i][d][t] + rdg[i][d][t] + Pdischg[i][d][t] == Pgridsell[i][d][t] + Eload[i][d][t] + Pchg[i][d][t]);



				if (t == 0)
				{
					model.add(Psoc[i][d][t] == Psoc[i][d][T - 1] + Pchg[i][d][t] * chgeff - Pdischg[i][d][t] / diseff);
				}
				else
				{
					model.add(Psoc[i][d][t] == Psoc[i][d][t - 1] + Pchg[i][d][t] * chgeff - Pdischg[i][d][t] / diseff);

				}


				model.add(Psoc[i][d][t] <= cap_share[i][d]);

				model.add(Psoc[i][d][t] >= 0);



			}

			//model.add(cap_share[i][d]==0);

			ce += cap_share[i][d];

		}

		model.add(ce == EcapMax);

	}




#pragma endregion



#pragma region Solve

	IloCplex cplex(model);


	cplex.solve();


	cout << "Objective Value: " << cplex.getObjValue() << endl;

	for (int i = 0; i < numMgs; i++)
	{
		cout << "MG: " << i << endl;
		cout << "Ecapacity: " << cplex.getValue(cap_share[i][0]) << "\t" << cplex.getValue(cap_share[i][1]) << endl << endl;
	}




	for (int i = 0; i < numMgs; i++)
	{
		for (size_t t = 0; t < 24; t++)
		{
			cout << cplex.getValue(Pchg[i][0][t]) << "\t" << cplex.getValue(Pdischg[i][0][t]) << endl;
		}

		cout << endl << endl;
	}


#pragma endregion

	return cplex.getObjValue();
}