#include <ilcplex/ilocplex.h>
#include <iostream>
#include <fstream>
#include "DataStore.h"

ILOSTLBEGIN

using namespace std;

typedef IloArray<IloNumVarArray> NumVar2D;
typedef IloArray<NumVar2D> NumVar3D;

double calculateCRF(double int_r, int n) {
    double numerator = int_r * pow(1 + int_r, n);
    double denominator = pow(1 + int_r, n) - 1;
    return numerator / denominator;
}

double calculateFactorI(double int_r, int ry_i, int l) {
    double sum = 0.0;

    for (int i = ry_i; i <= l; i = i + ry_i) {
        sum += 1.0 / pow(1 + int_r, i);
    }

    return sum;
}

int main() {
    IloEnv env;
    IloModel model(env);
    int M = 10000000000;
    int T = 8760;
    int D = 365;
    int H = 24;

    int Cinv_pv = 2000;
    int Com_pv = 33;
    int lt_pv = 25;

    int Cinv_wt =3200;
    int Com_wt = 100;
    int lt_wt = 25;

    int Cinv_bss = 100;
    int Com_bss = 5;
    int lt_bss = 5;

    int AbunPenalty = 10;
    int carbonPenalty = 40;
    float em_h = 0.0065;

    float cutin = 2.5;
    float cutout = 20;
    float windrated = 9.5;

    std::string csvFilePath = "D:\\MasterWork\\Codes\\MicrogridOptimization\\RandomGen\\OptimalSizing\\Datset.csv";
    DataStore::Populate_Parameters(csvFilePath);
    cout << "Success";
    NumVar2D solarout(env, D);
    NumVar2D windout(env, D);
    NumVar2D Electricgridbuy(env, D);
    NumVar2D Electricgridsell(env, D);
    NumVar2D ElectricAbun(env, D);
    NumVar2D Esschg(env, D);
    NumVar2D Essdischg(env, D);
    IloNumVar Esschg_max(env, D);
    IloNumVar Essdischg_max(env, D);
    NumVar2D Ess(env, D);
    NumVar2D bin(env, D);

    IloNumVar capwind(env);
    IloNumVar capsolar(env);
    IloNumVar capbatt(env);

    NumVar2D ool(env,D);
    for (int d = 0; d < D; d++)
    {
        solarout[d] = IloNumVarArray(env, H, 0, IloInfinity);
        windout[d] = IloNumVarArray(env, H, 0, IloInfinity);
        Electricgridbuy[d] = IloNumVarArray(env, H, 0, IloInfinity);
        Electricgridsell[d] = IloNumVarArray(env, H, 0, IloInfinity);
        ElectricAbun[d] = IloNumVarArray(env, H, 0, IloInfinity);
        Esschg[d] = IloNumVarArray(env, H, 0, IloInfinity);
        Essdischg[d] = IloNumVarArray(env, H, 0, IloInfinity);
        Ess[d] = IloNumVarArray(env, H, 0, IloInfinity);
        bin[d] = IloNumVarArray(env, H, 0, 1, ILOBOOL);
        ool[d] = IloNumVarArray(env, H, 0, 1, ILOBOOL);
    }



    for (int d = 0; d < 365; d++) {
        for (int h = 0; h < H; h++) {
            if (DataStore::wind_speed_2d[d][h] < cutin || DataStore::wind_speed_2d[d][h] > cutout) {
                model.add(windout[d][h] == 0);
            }
            else {
                if (DataStore::wind_speed_2d[d][h] >= windrated && DataStore::wind_speed_2d[d][h] <= cutout) {

                    model.add(windout[d][h] == capwind);
                }
                else {
                    double a = (pow(DataStore::wind_speed_2d[d][h], 2) - pow(cutin, 2) )/ (pow(windrated, 2) - pow(cutin, 2));
                    model.add(windout[d][h] == a * capwind);
                }
            }
        }
    }

    for (int d = 0; d < 365; d++) {
        for (int h = 0; h < H; h++) {
            float act_t_amb = DataStore::ambient_temperature_2d[d][h] + 0.0256 * DataStore::solar_irradiance_2d[d][h];
            float irr_factor = DataStore::solar_irradiance_2d[d][h] / DataStore::Iref;
            model.add(solarout[d][h] == capsolar * irr_factor * (1 + DataStore::k * (act_t_amb - DataStore::Tref)));
        }
    }



    IloExpr objective(env);

    for (int d = 0; d < 365; d++) {
        for (int h = 0; h < H; h++) {
            objective += DataStore::Price_2d[d][h] * Electricgridbuy[d][h] - 0.8 * DataStore::Price_2d[d][h] * Electricgridsell[d][h] + carbonPenalty * em_h * Electricgridbuy[d][h] + AbunPenalty * ElectricAbun[d][h] 
                ;
        }
    }

    objective += capsolar * Cinv_pv * calculateCRF(0.08, 25) + capsolar * Com_pv;
    objective += capwind * Cinv_wt * calculateCRF(0.08, 25) + capwind * Com_wt;
   // objective += capbatt * Cinv_bss * calculateCRF(0.08, 25) + Com_bss * capbatt *5;
   // objective += capbatt * Cinv_bss * calculateFactorI(0.08, 5, 25) * calculateCRF(0.08, 25);

    for (int d = 0; d < 365; d++) {
        for (int h = 0; h < H; h++) {
            // With Battery
             model.add(ElectricAbun[d][h] + solarout[d][h] + windout[d][h] + Electricgridbuy[d][h] - Electricgridsell[d][h] == DataStore::Load_2d[d][h]);
            //
            // With battery

             model.add(Ess[d][h] == 0);
             model.add(Esschg[d][h] == 0);
             model.add(Essdischg[d][h] == 0);
             model.add(capbatt == 0);
            //model.add(-Esschg[d][h] + Essdischg[d][h] + ElectricAbun[d][h] + solarout[d][h] + windout[d][h] + Electricgridbuy[d][h] - Electricgridsell[d][h] == DataStore::Load_2d[d][h]);
             /*
            if (h == 0) {
                // Set the Ess at the start of the day equal to its value at the end of the previous day
                // Assuming Ess is some kind of array or container
                model.add(Ess[d][h] == Ess[d][H - 1]);
            }
            else {
       
                model.add(Ess[d][h] == Ess[d][h - 1] + Esschg[d][h] * 0.95 - Essdischg[d][h] / 0.95);
            }
            model.add(Ess[d][h] <= 0.9 * capbatt);
            model.add(Ess[d][h] >= 0.1 * capbatt);

            model.add(Esschg[d][h] <= bin[d][h] * M);
            model.add(Esschg[d][h] <= Esschg_max);

            model.add(Essdischg[d][h] <= (1 - bin[d][h]) * M);
            model.add(Essdischg[d][h] <= Essdischg_max);

            model.add(Esschg[d][h] >= 0);
            model.add(Essdischg[d][h] >= 0);*/
        }
    }
    IloExpr emmission(env);
    for (int d = 0; d < 365; d++) {
        for (int h = 0; h < H; h++) {
            {
                emmission += em_h * Electricgridbuy[d][h];
            }
        }
    }
    model.add(IloMinimize(env, objective));
    IloExpr LCOE(env);
    IloExpr totalgeneration(env);
    IloCplex cplex(env);
    cplex.extract(model);
    cplex.exportModel("ModelLP.lp");
    cplex.solve();
    double obj = cplex.getObjValue();
    cout << "Solution status: " << cplex.getStatus() << endl;

    IloExpr Abund(env);
    for (int d = 0; d < 365; d++) {
        for (int h = 0; h < H; h++) {
            {
                Abund += ElectricAbun[d][h];
            }
        }
    }
    IloExpr demand(env);

    for (int d = 0; d < 365; d++) {
        for (int h = 0; h < H; h++) {
            {
                demand += DataStore::Load_2d[d][h]; // Assuming Load_2d is a 2D vector
            }
        }
    }

    for (int d = 0; d < 365; d++) {
        for (int h = 0; h < H; h++) {
            {
                totalgeneration += cplex.getValue(solarout[d][h]) + cplex.getValue(windout[d][h]);
            }
        }
    }


    LCOE = cplex.getValue(objective) / totalgeneration;
    cout << "Annual System Cost " << obj << endl;
    cout << "CapSOlar : " << cplex.getValue(capsolar) << "\t" << "CapWind : " << cplex.getValue(capwind) << "\t" << "Capatt : " << cplex.getValue(capbatt) << endl;
    cout << "LCOE" << "\t" << LCOE << endl;
    cout << "Total Emission" << "\t" << cplex.getValue(emmission) << endl;
    cout << "Total Abun" << "\t" << cplex.getValue(Abund) << endl;
    cout << "Total Demand" << "\t" << cplex.getValue(demand) << endl;
    cout << "Total Gen" << "\t" << cplex.getValue(totalgeneration) << endl;

    ofstream outputFile("without_ess_init.csv");

    // Write header line
    outputFile << "SolarOut,WindOut,Ess,ElectricGridBuy,ElectricGridSell,EssChg,EssDischg,Load,ElectricAbun" << endl;

    // Write data for each time period
    for (int t = 0; t < 24; t++) {
        cout << cplex.getValue(windout[0][t]);
        outputFile << cplex.getValue(solarout[0][t]) << ',' << cplex.getValue(windout[0][t]) << ',' << cplex.getValue(Ess[0][t]) << ',' << cplex.getValue(Electricgridbuy[0][t]) << ',' << cplex.getValue(Electricgridsell[0][t]) << ',' <<
            cplex.getValue(Esschg[0][t]) << ',' << cplex.getValue(Essdischg[0][t]) << ',' << DataStore::Load_2d[0][t] << ',' << cplex.getValue(ElectricAbun[0][t]) << endl;
    }

    // Close the output file
    outputFile.close();


    return 0;
}
