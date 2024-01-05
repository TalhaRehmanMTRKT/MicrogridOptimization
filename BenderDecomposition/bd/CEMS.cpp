#include "IncludeDataFunction.h"



double CEMS(int& NumMgs,SMESSRes& second, prices& price) {


	IloEnv env;
	IloModel model(env);

	int T = 24;



	NumVar2D Egridbuy(env, NumMgs);
	NumVar2D Egridsell(env, NumMgs);
	NumVar2D Tgridbuy(env, NumMgs);
	NumVar2D Tgridsell(env, NumMgs);
	NumVar2D Hgridbuy(env, NumMgs);


	NumVar3D Esend(env, NumMgs);
	NumVar3D Tsend(env, NumMgs);
	NumVar3D Hsend(env, NumMgs);

	NumVar3D Erecv(env, NumMgs);
	NumVar3D Trecv(env, NumMgs);
	NumVar3D Hrecv(env, NumMgs);


	for (int i = 0; i < NumMgs; i++)
	{

		Egridbuy[i] = IloNumVarArray(env, T, 0, IloInfinity);
		Egridsell[i] = IloNumVarArray(env, T, 0, IloInfinity);
		
		Tgridbuy[i] = IloNumVarArray(env, T, 0, IloInfinity);
		Tgridsell[i] = IloNumVarArray(env, T, 0, IloInfinity);
		
		Hgridbuy[i] = IloNumVarArray(env, T, 0, IloInfinity);


		Esend[i] = NumVar2D(env, NumMgs);
		Tsend[i] = NumVar2D(env, NumMgs);
		Hsend[i] = NumVar2D(env, NumMgs);

		Erecv[i] = NumVar2D(env, NumMgs);
		Trecv[i] = NumVar2D(env, NumMgs);
		Hrecv[i] = NumVar2D(env, NumMgs);

		for (int l = 0; l < NumMgs; l++)
		{

			Esend[i][l] = IloNumVarArray(env, T, 0, IloInfinity);
			Tsend[i][l] = IloNumVarArray(env, T, 0, IloInfinity);
			Hsend[i][l] = IloNumVarArray(env, T, 0, IloInfinity);

			Erecv[i][l] = IloNumVarArray(env, T, 0, IloInfinity);
			Trecv[i][l] = IloNumVarArray(env, T, 0, IloInfinity);
			Hrecv[i][l] = IloNumVarArray(env, T, 0, IloInfinity);

		}


	}


	int M= 1000000;
	IloExpr objective(env);

	for (int mg = 0; mg < NumMgs; mg++)
	{
		for (int t = 0; t < T; t++)
		{
			objective += price.EPriceBuy[t] * Egridbuy[mg][t] - price.EPriceSell[t] * Egridsell[mg][t] + price.TPriceBuy[t] * Tgridbuy[mg][t] - price.TPriceSell[t] * Tgridsell[mg][t] + price.HydrogenPrice * Hgridbuy[mg][t];
		}

	}


	model.add(IloMinimize(env, objective));

	IloExpr exp0(env);
	IloExpr exp1(env);
	IloExpr exp2(env);
	IloExpr exp3(env);


	for (int t = 0; t < T; t++)
	{
		for (int k = 0; k < NumMgs; k++)
		{
			for (int l = 0; l < NumMgs; l++)
			{

				if (k != l) {
					exp0 += Esend[k][l][t];
					exp1 += Erecv[k][l][t];
				}
				else
				{
					model.add(Esend[k][l][t] == 0);
					model.add(Erecv[k][l][t] == 0);
				}
			}
		}
		model.add(exp0 == exp1);
		model.add(exp2 == exp3);
	}



	for (int t = 0; t < T; t++)
	{

		for (int k = 0; k < NumMgs; k++)
		{

			IloExpr mgEsend(env);
			IloExpr mgErec(env);

			for (int l = 0; l < NumMgs; l++)
			{
				mgEsend += Esend[k][l][t];
				mgErec += Erecv[k][l][t];
			}
			model.add(mgEsend + Egridsell[k][t] == second.Esur[k][t]);
			model.add(mgErec + Egridbuy[k][t] == second.Eshort[k][t]);
			model.add(Tgridbuy[k][t] == second.Tshort[k][t]);
			model.add(Tgridsell[k][t] == second.Tsur[k][t]);
			model.add(Hgridbuy[k][t] == second.Hshort[k][t]);

		}

	}

	IloCplex cplex(model);
	cplex.setOut(env.getNullStream());

	cplex.solve();

	std::cout << "\t The CMES is: ";
	std::cout << cplex.getStatus() << endl;

	double obj = cplex.getObjValue();
	cout<<obj << endl;




	std::string globalRes = "D:\\MasterWork\\Codes\\MicrogridOptimization\\BenderDecomposition\\Global";



	// Construct the full file path
	std::string filePath = globalRes + "\\_cems.csv";



	// Open the file for writing
	std::ofstream outputFile(filePath);

	// Write the data to the output file stream

	for (int mg = 0; mg < NumMgs; mg++)
	{
		outputFile << "MG" << mg << endl;

		outputFile << "Egridbuy,EgridSell,Eshort,Esur,Esend,Erec,Tgridbuy,TgridSell,Tshort,Tsur,HgridGuy,Hshort" << endl;

		for (int i = 0; i < T; i++)
		{
			double Es = 0;
			double Er = 0;
			for (int k = 0; k < NumMgs; k++)
			{
				Es += cplex.getValue(Esend[mg][k][i]);
				Er += cplex.getValue(Erecv[mg][k][i]);
			}


			outputFile<< cplex.getValue(Egridbuy[mg][i]) << "," 
				<< cplex.getValue(Egridsell[mg][i]) << "," 
				<< second.Eshort[mg][i] << ","
				<< second.Esur[mg][i] << ","
				<< Es << ","
				<< Er << ","
				<< cplex.getValue(Tgridbuy[mg][i]) << ","
				<< cplex.getValue(Tgridsell[mg][i]) << ","
				<< second.Tshort[mg][i] << ","
				<< second.Tsur[mg][i] << ","
				<< cplex.getValue(Hgridbuy[mg][i]) << ","
				<< second.Hshort[mg][i] << endl;
		}

		outputFile << endl;
		outputFile << endl;


	}




	return obj;
}