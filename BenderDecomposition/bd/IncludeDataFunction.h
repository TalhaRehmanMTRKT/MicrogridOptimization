#pragma once
#pragma once
#include <iostream>;  // for  cout cin
#include<random>; // for random number generation
#include <fstream>; // to read and write from/on files
#include<chrono>; // for high resolution timing (elapsed time in microseconds and so on)
#include"ilcplex/ilocplex.h";
#include <string>;


using namespace std;
typedef IloArray<IloNumVarArray>  NumVar2D;
typedef IloArray<NumVar2D>  NumVar3D;

//std::string SmresFold = "D:\\MasterWork\\Codes\\MicrogridOptimization\\BenderDecomposition\\SMESS";



struct MGCRes
{
	vector<double> Esh;
	vector<double> Esur;
	vector<double> Tsh;
	vector<double> Hsh;
	vector<double> Tsurr;
	float effEB;
	float effEL;

};


struct SMESSRes
{
	vector<double> Ecapacity;
	vector<double> Tcapacity;
	vector<double> Hcapacity;
	vector<vector<double>> Eshort;
	vector<vector<double>> Tshort;
	vector<vector<double>> Hshort;
	vector<vector<double>> Esur;
	vector<vector<double>> Tsur;
	vector<vector<double>> Hsur;
};

struct prices
{
	vector<double> TPriceBuy;
	vector<double> TPriceSell;
	vector<double> EPriceBuy;
	vector<double> EPriceSell;
	double HydrogenPrice;
};


double centralized();

double CEMS(int& NumMgs, SMESSRes& second, prices& price);
MGCRes MGC(int& id, int& NumEVs, vector<double>TPriceBuy, vector<double>TPriceSell, vector<double>EPriceBuy, vector<double>EPriceSell, vector<double>& RDG, vector<double>& Eload, vector<double>& Tload, vector<double>& Cload, vector<double>& Hload);



void printMGCRes(const MGCRes& mgcRes);
SMESSRes smess(int& total, int& numMgs, std::vector<MGCRes>& mgVector, prices& price);

double singleMG();