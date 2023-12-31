#include <ilcplex/ilocplex.h>
#include<chrono>
#include <iostream>
#include <fstream>
ILOSTLBEGIN


int main(int, char**)
{

    auto start = chrono::high_resolution_clock::now();
    IloEnv env;
    IloModel model(env);


#pragma region Microgrid Input Data

// Some Constants 
int T = 24; //One day
int Cdg1 = 135; //Cost for generating per kW power from dg
int Cdg2 = 140; //Cost for generating per kW power from dg
int Chob = 80;
int Cchp1 = 150;
int Cchp2 = 145;
float socini = 0.2; //Initially charged battery
int Pbmax = 100; //battery maximum capacity
float effin = 0.95; //battery effciency
float k1 = 1.2; //heat to electric ratio chp1
float k2 = 0.8; //heat to electric ratio chp2
float Heffin = 0.95; //battery effciency
int Hssmax = 50; //battery maximum capacity

// Electric and Heat Demand 
int* Pload = new int[T] {169, 175, 179, 171, 181, 190, 270, 264, 273, 281, 300, 320, 280, 260, 250, 200, 180, 190, 240, 280, 325, 350, 300, 250};  //Electicity demand w.r.t tim
int* Hload = new int[T] {130, 125, 120, 120, 125, 135, 150, 160, 175, 190, 195, 200, 195, 195, 180, 170, 185, 190, 195, 200, 195, 190, 180, 175};  //Heat Demand


// Trading Prices Electric and Heat
int* CGbuy = new int[T] { 138, 139, 143, 149, 150, 152, 155, 158, 160, 154, 153, 153, 152, 150, 149, 149, 154, 156, 163, 164, 164, 160, 150, 148};  //buying price of electricity
int* CGsell = new int[T] { 128, 129, 133, 139, 140, 142, 145, 148, 150, 144, 143, 143, 142, 140, 139, 139, 144, 146, 153, 154, 154, 150, 140, 138}; //selling price of electricity
int* CHbuy = new int[T] {77, 77, 77, 77, 77, 77, 77, 77, 83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 78, 78, 78, 78, 78, 78}; // buying price of heat
int* CHsell = new int[T] {75, 75, 75, 75, 75, 75, 75, 75, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 78, 78, 78, 78, 78, 78 }; // selling price of heat

// Renewable Infeeds
float* Rdg1 = new float[T] {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 10, 15, 20, 23, 28, 33, 35, 34, 31, 28, 10, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 }; //Power generated by PV w.r.t time
//float* Rdg2 = new float[T] {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 10, 15, 20, 23, 28, 33, 35, 34, 31, 28, 10, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 }; //Power generated by PV w.r.t time

#pragma endregion


#pragma region Decision Variables

IloNumVarArray PGbuy(env, T, 0, IloInfinity);//Grid power bought
IloNumVarArray PGsell(env, T, 0, IloInfinity);//Grid power sold

IloNumVarArray HGbuy(env, T, 0, IloInfinity, ILOINT);//Grid Heat bought
IloNumVarArray HGsell(env, T, 0, IloInfinity, ILOINT);//Grid Heat sold


IloNumVarArray statoc(env, T, 0, 1); //battery storage capacity
IloNumVarArray Bchg(env, T, 0, 50); //battery charging
IloNumVarArray Bdischg(env, T, 0, 50); //battery discharging

IloNumVarArray HSSsoc(env, T, 0, 1); //battery storage capacity
IloNumVarArray Hchg(env, T, 0, 50,ILOINT); //battery charging
IloNumVarArray Hdischg(env, T, 0, 50,ILOINT); //battery discharging

IloNumVarArray Pdg1(env, T, 0, 100,ILOINT); //DGA power generation range
IloNumVarArray Pdg2(env, T, 0, 80, ILOINT); //DGA power generation range
IloNumVarArray Pchp1(env, T, 30, 60,ILOINT); //CHP1 power generation range
IloNumVarArray Pchp2(env, T, 50, 100, ILOINT); //CHP2 power generation range


IloNumVarArray Hhob(env, T, 0, 80, ILOINT); //HOB heat power generation range
IloNumVarArray Hchp1(env, T, 0, IloInfinity); //CHP1 heat power generation range
IloNumVarArray Hchp2(env, T, 0, IloInfinity); //CHP2 heat generation range

#pragma endregion


#pragma region Objective Function

IloExpr objective(env, T);

 
    for (int t = 0; t < T; t++)
    {
        objective +=  Cdg1 * Pdg1[t] + Cdg2 * Pdg2[t] + CGbuy[t] * PGbuy[t] - CGsell[t] * PGsell[t]  + CHbuy[t] * HGbuy[t] - CHsell[t] * HGsell[t] + Chob * Hhob[t] 
            + Cchp1 * Pchp1[t] + Cchp2 * Pchp2[t];
    }


    // Objective: minimize cost
    model.add(IloMinimize(env, objective));

#pragma endregion



#pragma region Constraints


    // Constraint: meet demand

    for (int t = 0; t < T; t++)
    {

        model.add(0 <= Pdg1[t]);
        model.add(Pdg1[t] <= 100);

        model.add(0 <= Pdg2[t]);
        model.add(Pdg2[t] <= 80);

        model.add(0 <= Hhob[t]);
        model.add(Hhob[t] <= 80);

        model.add(30 <= Pchp1[t]);
        model.add(Pchp1[t] <= 60);

        model.add(50 <= Pchp2[t]);
        model.add(Pchp2[t] <= 100);

        model.add(Hchp1[t] == (k1) * Pchp1[t]);
        model.add(Hchp2[t] == (k2) * Pchp2[t]);



        // Adding the Battery Constraints constraints
        model.add(0 <= statoc[t]);
        model.add(statoc[t] <= 1);
        model.add(0 <= HSSsoc[t]);
        model.add(HSSsoc[t] <= 1);


        if (t == 0)
        {
            model.add(statoc[t] == socini + ((effin * Bchg[t] - (Bdischg[t] / effin)) / Pbmax));
            model.add(0 <= Bchg[t]);
            model.add(0 <= Bdischg[t]);
            model.add(Bchg[t] <= (Pbmax * (1 - socini) / effin));
            model.add(Bdischg[t] <= (Pbmax * socini * effin));


            model.add(HSSsoc[t] == 0.5 + ((Heffin * Hchg[t] - (Hdischg[t] / Heffin)) / Hssmax));
            model.add(0 <= Hchg[t]);
            model.add(0 <= Hdischg[t]);
            //model.add(0 == Hdischg[t] * Hchg[t]);
            model.add(Hchg[t] <= (Hssmax * (1 - 0.5) / Heffin));
            model.add(Hdischg[t] <= (Hssmax * 0.5 * Heffin));

        }
        else
        {
            model.add(statoc[t] == statoc[t - 1] + ((effin * Bchg[t] - (Bdischg[t] / effin)) / Pbmax));
            model.add(0 <= Bchg[t]);
            model.add(0 <= Bdischg[t]);
            model.add(Bchg[t] <= (Pbmax * (1 - statoc[t - 1])) / effin);
            model.add(Bdischg[t] <= Pbmax * statoc[t - 1] * effin);

            model.add(HSSsoc[t] == HSSsoc[t - 1] + ((Heffin * Hchg[t] - (Hdischg[t] / Heffin)) / Hssmax));
            model.add(0 <= Hchg[t]);
            model.add(0 <= Hdischg[t]);
            //model.add(0 == Hdischg[t] * Hchg[t]);
            model.add(Hchg[t] <= (Hssmax * (1 - HSSsoc[t - 1]) / Heffin));
            model.add(Hdischg[t] <= (Hssmax * HSSsoc[t - 1] * Heffin));

        }

        // Adding the Electric power balance constraint
        model.add(Pdg1[t] + Pdg2[t] + Rdg1[t]+ Pchp1[t] + Pchp2[t] + Bdischg[t] - Bchg[t] + PGbuy[t] - PGsell[t] == Pload[t]);

        model.add(Hhob[t] + Hchp1[t] + Hchp2[t] + HGbuy[t] - HGsell[t] - Hchg[t] + Hdischg[t] == Hload[t]);


    }

#pragma endregion


#pragma region Results
    IloCplex cplex(env);
    cplex.extract(model);
    cplex.exportModel("ModelLP.lp");
    cplex.setOut(env.getNullStream());
    if (!cplex.solve()) {
        env.error() << "Failed" << endl;
        throw(-1);
    }
    double obj = cplex.getObjValue();
    auto end = chrono::high_resolution_clock::now();
    auto Elapsed = chrono::duration_cast<chrono::milliseconds>(end - start);
    cout << "\n\n\t The elapsed time is: \t" << Elapsed.count() << endl;
    cout << "Solution status: " << cplex.getStatus() << endl;
    cout << "Minimized Objective Funtion : " << obj << endl;
    IloNum eps = cplex.getParam(
        IloCplex::Param::MIP::Tolerances::Integrality);
#pragma endregion


#pragma region StoringOutput

    // Create and open the CSV file for writing
    std::ofstream outputFile("output.csv");

    if (outputFile.is_open()) {
        // Write the header row
        outputFile << "Time,Pload,Hload,CGbuy,CGsell,CHsell,CHbuy,Rdg1,PGbuy,PGsell,statoc,Bchg,Bdischg,Pdg1,Pdg2,Pchp1,Pchp2,Hhob,Hchp1,Hchp2,HGbuy,HGsell,Hchg,Hdisch" << std::endl;

        // Write the data rows
        for (int i = 0; i < T; i++) {
            outputFile << i + 1 << "," << Pload[i] <<","<<Hload[i] << "," << CGbuy[i] << ","
                << CGsell[i] << "," << CHsell[i] << ","
                << CHbuy[i] << "," << Rdg1[i] << ","
                << cplex.getValue(PGbuy[i]) << "," << -cplex.getValue(PGsell[i]) << ","
                << cplex.getValue(statoc[i]) << "," << -cplex.getValue(Bchg[i]) << ","
                << cplex.getValue(Bdischg[i]) << "," << cplex.getValue(Pdg1[i]) << ","
                << cplex.getValue(Pdg2[i]) << ","
                << cplex.getValue(Pchp1[i]) << "," << cplex.getValue(Pchp2[i]) << ","
                << cplex.getValue(Hhob[i]) << "," << cplex.getValue(Hchp1[i]) << ","
                << cplex.getValue(Hchp2[i]) << "," << cplex.getValue(HGbuy[i]) << ","
                << -cplex.getValue(HGsell[i]) << "," << cplex.getValue(Hchg[i]) << "," << cplex.getValue(Hdischg[i]);

            outputFile << std::endl;

        }


        // Close the CSV file
        outputFile.close();
        std::cout << "Data saved to output.csv" << std::endl;
    }
    else {
        std::cerr << "Failed to open the output.csv file for writing." << std::endl;
    }



#pragma endregion

    env.end();
    system("pause");
    return 0;
}