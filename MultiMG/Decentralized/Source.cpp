#include <ilcplex/ilocplex.h>
#include<chrono>
#include <iostream>
#include <fstream>
#include <map>
#include"EMS.h"
ILOSTLBEGIN


typedef IloArray<IloNumVarArray> NumVar2D;
typedef IloArray<NumVar2D> NumVar3D;


void printIntArray(int* arr, int size) {
    for (int i = 0; i < size; i++) {
        std::cout << arr[i] << " ";
    }
    std::cout << std::endl;
}


int
main(int, char**)
{

    auto start = chrono::high_resolution_clock::now();
    IloEnv env;
    IloModel model(env);


#pragma region Microgrid Input Data

// Some Constants
 

int T = 24; //One day
int numEvs = 5; // Total Nmber of Electric vehicles

map<std::string, int> int_constant = {
        {"Cdg1", 135},
        {"Cdg2", 140},
        {"Chob", 80},
        {"Cchp1", 150},
        {"Cchp2", 145},
        {"Pbmax", 150},
        {"Hssmax", 50},
};


map<std::string, float> float_constant = {
        {"socini", 0.2},
        {"effin", 0.95},
        {"Eveffin", 0.90},
        {"k1", 1.2},
        {"k2", 0.8},
        {"Heffin", 0.95}
};


// Arrival and Departure time of Evs 
int* ta = new int[numEvs] {2,4,6,9,9};
int* td = new int[numEvs] {6, 7, 9, 12, 16};
float* evsocini = new float[numEvs] {0.30,0.20,0.10,0.70,0.40};
float* evcap = new float[numEvs] {38.3, 47.5, 28.9, 56, 52};
    
// Electric and Heat Demand 
int* Pload_arr = new int[T] {169, 175, 179, 171, 181, 190, 270, 264, 273, 281, 300, 320, 280, 260, 250, 200, 180, 190, 240, 280, 325, 350, 300, 250};  //Electicity demand w.r.t tim
int* Hload_arr = new int[T] {130, 125, 120, 120, 125, 135, 150, 160, 175, 190, 195, 200, 195, 195, 180, 170, 185, 190, 195, 200, 195, 190, 180, 175};  //Heat Demand
int* Cload_arr = new int[T] {100, 100, 80, 100, 120, 135, 150, 135, 125, 130, 140, 150, 150, 130, 120, 110, 90, 80, 135, 150, 135, 140, 110, 125};  //Heat Demand


// Electric, Heat and Cooling Demand  // add the randomly populating 
int** Pload = new int* [3];  //Electicity demand w
int** Hload = new int* [3];  //Heat Demand
int** Cload = new int* [3];  //Cooling Demand

float* scaling = new float[3] {1,0.5,2};

for (int i = 0; i < 3; i++)
{   
    Pload[i] = new int[T];
    Hload[i] = new int[T];
    Cload[i] = new int[T];

    for (int t = 0; t < T; t++)
    {
        Pload[i][t] = static_cast<int>(Pload_arr[t] * scaling[i]);
        Hload[i][t] = static_cast<int>(Hload_arr[t] * scaling[i]);
        Cload[i][t]  = static_cast<int>(Cload_arr[t] * 1);
    }
    
}





// Trading Prices Electric and Heat
int* CGbuy = new int[T] { 138, 139, 143, 149, 150, 152, 155, 158, 160, 154, 153, 153, 152, 150, 149, 149, 154, 156, 163, 164, 164, 160, 150, 148};  //buying price of electricity
int* CGsell = new int[T] { 128, 129, 133, 139, 140, 142, 145, 148, 150, 144, 143, 143, 142, 140, 139, 139, 144, 146, 153, 154, 154, 150, 140, 138}; //selling price of electricity
int* CHbuy = new int[T] {77, 77, 77, 77, 77, 77, 77, 77, 83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 78, 78, 78, 78, 78, 78}; // buying price of heat
int* CHsell = new int[T] {75, 75, 75, 75, 75, 75, 75, 75, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 78, 78, 78, 78, 78, 78 }; // selling price of heat
// Scale the prices by a factor of 1.8
const double scalingFactor = 1.8;
for (int i = 0; i < T; ++i) {
    CHbuy[i] = static_cast<int>(CHbuy[i] * scalingFactor);
    CHsell[i] = static_cast<int>(CHsell[i] * scalingFactor);
}



// Renewable Infeeds
float* Rdg1 = new float[T] {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 10, 15, 20, 23, 28, 33, 35, 34, 31, 28, 10, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 }; //Power generated by PV w.r.t time
//float* Rdg2 = new float[T] {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 10, 15, 20, 23, 28, 33, 35, 34, 31, 28, 10, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 }; //Power generated by PV w.r.t time

#pragma endregion


EMS mg1("mg1" , T, numEvs, int_constant, float_constant, ta, td, evsocini, evcap, Pload[0], Hload[0], Cload[0], CGbuy, CGsell, CHbuy, CHsell, Rdg1);
EMS mg2("mg2" , T, numEvs, int_constant, float_constant, ta, td, evsocini, evcap, Pload[1], Hload[1], Cload[1], CGbuy, CGsell, CHbuy, CHsell, Rdg1);
EMS mg3("mg3" , T, numEvs, int_constant, float_constant, ta, td, evsocini, evcap, Pload[2], Hload[2], Cload[2], CGbuy, CGsell, CHbuy, CHsell, Rdg1);
resi mg1Result = mg1.solve();
resi mg2Result = mg2.solve();
resi mg3Result = mg3.solve();



int NumMg = 3;


int** pshort = new int* [NumMg] {mg1Result.pmgshort, mg2Result.pmgshort, mg3Result.pmgshort};
int** psur = new int* [NumMg] {mg1Result.pmgsur, mg2Result.pmgsur, mg3Result.pmgsur};
int** hshort = new int* [NumMg] {mg1Result.Hmgshort, mg2Result.Hmgshort, mg3Result.Hmgshort};
int** hsur = new int* [NumMg] {mg1Result.Hmgsur, mg2Result.Hmgsur, mg3Result.Hmgsur};


int* ptotalshort = new int[T];
int* ptotalsur = new int[T];
int* htotalshort = new int[T];
int* htotalsur = new int[T];

for (int t = 0; t < T; t++) {
    ptotalshort[t] = 0; // Initialize to 0 for each time step
    ptotalsur[t] = 0; // Initialize to 0 for each time step
    htotalshort[t] = 0; // Initialize to 0 for each time step
    htotalsur[t] = 0; // Initialize to 0 for each time step

    for (int mg = 0; mg < NumMg; mg++) {
        ptotalshort[t] += pshort[mg][t];
        ptotalsur[t] += psur[mg][t];
        htotalshort[t] += hshort[mg][t];
        htotalsur[t] += hsur[mg][t];
    }
}


#pragma region CEMS



IloEnv envcems;
IloModel cmodel(envcems);


#pragma region CEMS decision variables


NumVar2D powergridbuy(envcems, NumMg);
NumVar2D powergridsell(envcems, NumMg);
NumVar2D heatgridbuy(envcems, NumMg);
NumVar2D heatgridsell(envcems, NumMg);


NumVar3D powersend(envcems, NumMg);
NumVar3D powersrec(envcems, NumMg);
NumVar3D heatsend(envcems, NumMg);
NumVar3D heatrec(envcems, NumMg);


for (int mg = 0; mg < NumMg; mg++)
{
    powergridbuy[mg] = IloNumVarArray(envcems, T, 0, IloInfinity);
    powergridsell[mg] = IloNumVarArray(envcems, T, 0, IloInfinity);
    heatgridbuy[mg] = IloNumVarArray(envcems, T, 0, IloInfinity);
    heatgridsell[mg] = IloNumVarArray(envcems, T, 0, IloInfinity);


    powersend[mg] = NumVar2D(envcems, NumMg);
    powersrec[mg] = NumVar2D(envcems, NumMg);
    heatsend[mg] = NumVar2D(envcems, NumMg);
    heatrec[mg] = NumVar2D(envcems, NumMg);


    for (int l = 0; l < NumMg; l++)
    {
        powersend[mg][l] = IloNumVarArray(envcems, T, 0, IloInfinity);
        powersrec[mg][l] = IloNumVarArray(envcems, T, 0, IloInfinity);
        heatsend[mg][l] = IloNumVarArray(envcems, T, 0, IloInfinity);
        heatrec[mg][l] = IloNumVarArray(envcems, T, 0, IloInfinity);

    }

}

#pragma endregion


#pragma region Objective Function

IloExpr cemsobjective(envcems);

for (int mg = 0; mg < NumMg; mg++)
{
    for (int t = 0; t < T; t++)
    {
        cemsobjective += CGbuy[t] * powergridbuy[mg][t] - CGsell[t] * powergridsell[mg][t] - CHsell[t] * heatgridsell[mg][t] + CHbuy[t] * heatgridbuy[mg][t];
    }
}

cmodel.add(IloMinimize(envcems, cemsobjective));

#pragma endregion


#pragma region constraints

IloExpr exp0(envcems);
IloExpr exp1(envcems);
IloExpr exp2(envcems);
IloExpr exp3(envcems);


for (int t = 0; t < T; t++)
{
    for (int k = 0; k < NumMg; k++)
    {
        for (int l = 0; l < NumMg; l++)
        {

            if (k != l) {
                exp0 += powersend[k][l][t];
                exp1 += powersrec[k][l][t];
                exp2 += heatsend[k][l][t];
                exp3 += heatrec[k][l][t];


            }
            else
            {
                    cmodel.add(powersend[k][l][t] == 0);
                    cmodel.add(powersrec[k][l][t] == 0);
                    cmodel.add(heatsend[k][l][t] == 0);
                    cmodel.add(heatrec[k][l][t] == 0);

            }
        }
    }
    cmodel.add(exp0 == exp1);
    cmodel.add(exp2 == exp3);
}





for (int t = 0; t < T; t++)
{

    for (int k = 0; k < NumMg; k++)
    {

        IloExpr mgEsend(envcems);
        IloExpr mgErec(envcems);
        IloExpr mgHsend(envcems);
        IloExpr mgHrec(envcems);

        for (int l = 0; l < NumMg; l++)
        {
            mgEsend += powersend[k][l][t];
            mgErec += powersrec[k][l][t];
            mgHsend += heatsend[k][l][t];
            mgHrec += heatrec[k][l][t];
        }
        cmodel.add(mgEsend + powergridsell[k][t] == psur[k][t]);
        cmodel.add(mgHsend + heatgridsell[k][t] == hsur[k][t]);

        cmodel.add(mgErec + powergridbuy[k][t] == pshort[k][t]);

        cmodel.add(mgHrec + heatgridbuy[k][t] == hshort[k][t]);



    }



    


}




#pragma endregion




#pragma endregion
IloCplex cplex(envcems);
cplex.extract(cmodel);
cplex.exportModel("ModelLP.lp");
cplex.setOut(envcems.getNullStream());
cplex.solve();
double obj = cplex.getObjValue();
cout << "Solution status: " << cplex.getStatus() << endl;
cout << "Minimized Objective Funtion of " << obj << endl;





// Create and open the CSV file for writing
ofstream outputFile("global_output.csv");



outputFile << "Mg1send/rec,Mg1short/sur,mg1buy/sell,Mg2send/rec,Mg2short/sur,mg2buy/sell,Mg3send/rec,Mg3short/sur,mg3buy/sell" << endl;


for (int t = 0; t < T; t++)
{


    for (int mg = 0; mg < NumMg; mg++)
    {
        double ps = 0; double pr = 0;

        for (int l = 0; l < NumMg; l++)
        {
            ps += cplex.getValue(powersend[mg][l][t]);
            pr += cplex.getValue(powersrec[mg][l][t]);

        }

        outputFile << -ps + pr << ","<<+psur[mg][t] - pshort[mg][t]<<"," << cplex.getValue(powergridbuy[mg][t]) - cplex.getValue(powergridsell[mg][t]) << ",";

    }

    outputFile << endl;


}




return 1;

}