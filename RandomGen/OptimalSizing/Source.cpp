#include <ilcplex/ilocplex.h>

using namespace std;
ILOSTLBEGIN

int main() {

    IloEnv env;
    IloModel Model(env);

    try {



#pragma region Lower Level (Optimal Operation done by CEMS)


        int totaltime = 24;//One day
        float effin = 0.95;//battery effciency


        IloNumVar Ca(env, 0, IloInfinity, ILOINT,"Ca"); // Allocated Capacity
        IloNumVar Cb(env, 0, IloInfinity, ILOINT, "Cb"); // Allocated Capacity
        IloNumVar Cc(env, 0, IloInfinity, ILOINT, "Cc"); // Allocated Capacity
        IloNumVar Cu(env, 0, IloInfinity, ILOINT, "Cu"); // Unused Capacity

        float gridbuy[24] = { 64,60,56,54,59,63,66,78,86,91,88,73,56,54,54,56,72,77,98,96,90,88,87,81 };
        float gridsell[24] = { 52,48,44,42,47,51,54,66,74,79,76,61,44,42,42,44,60,65,86,84,78,76,75,69 };
        float mg1sur[24] = { 81, 105, 0, 0, 0, 78, 0, 2, 96, 89, 90, 52, 0, 0, 0, 0, 111, 67, 0, 0, 22, 33, 36, 50 };
        float mg1short[24] = { 0, 0, 0, 0, 0, 0, 20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 14, 17, 0, 0, 0, 0 };
       
        float mg2sur[24] = { 168, 173, 171, 172, 160, 136, 353, 295, 65, 0, 0, 0, 55, 36, 0, 0, 12, 47, 176, 244, 283, 301, 353, 384 };
        float mg2short[24] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 68, 62, 41, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        
        float mg3sur[24] = { 0, 0, 5, 8, 0, 0, 0, 139, 90, 61, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 36, 96 };
        float mg3short[24] = { 0, 45, 0, 0, 5, 0, 0, 0, 0, 0, 0, 4, 235, 258, 303, 314, 65, 75, 124, 104, 93, 23, 0, 0 };


        IloNumVarArray Gridpowerbuy1(env, totaltime, 0, IloInfinity);//Grid power bought
        IloNumVarArray Gridpowersell1(env, totaltime, 0, IloInfinity);//Grid power sold
        IloNumVarArray statoc(env, totaltime, 0, IloInfinity);//battery SOC
        IloNumVarArray besschg1(env, totaltime, 0, 100);//battery charging
        IloNumVarArray bessdis1(env, totaltime, 0, 100);//battery discharging
        IloNumVarArray bina1(env, totaltime, 0, 1, ILOBOOL);//battery discharging
        IloNumVarArray bina2(env, totaltime, 0, 1, ILOBOOL);//battery discharging

        IloNumVarArray Gridpowerbuy2(env, totaltime, 0, IloInfinity);//Grid power bought
        IloNumVarArray Gridpowersell2(env, totaltime, 0, IloInfinity);//Grid power sold
        IloNumVarArray statoc2(env, totaltime, 0, IloInfinity);//battery SOC
        IloNumVarArray besschg2(env, totaltime, 0, 100);//battery charging
        IloNumVarArray bessdis2(env, totaltime, 0, 100);//battery discharging
        IloNumVarArray binb1(env, totaltime, 0, 1, ILOBOOL);//battery discharging
        IloNumVarArray binb2(env, totaltime, 0, 1, ILOBOOL);//battery discharging


        IloNumVarArray Gridpowerbuy3(env, totaltime, 0, IloInfinity);//Grid power bought
        IloNumVarArray Gridpowersell3(env, totaltime, 0, IloInfinity);//Grid power sold
        IloNumVarArray statoc3(env, totaltime, 0, IloInfinity);//battery SOC
        IloNumVarArray besschg3(env, totaltime, 0, 100);//battery charging
        IloNumVarArray bessdis3(env, totaltime, 0, 100);//battery discharging
        IloNumVarArray binc1(env, totaltime, 0, 1, ILOBOOL);//battery discharging
        IloNumVarArray binc2(env, totaltime, 0, 1, ILOBOOL);//battery discharging


        IloNumVarArray mg1send(env, totaltime, 0, IloInfinity);//Grid power bought
        IloNumVarArray mg2send(env, totaltime, 0, IloInfinity);//Grid power sold
        IloNumVarArray mg3send(env, totaltime, 0, IloInfinity);//Grid power sold
        IloNumVarArray mg1rec(env, totaltime, 0, IloInfinity);//Grid power bought
        IloNumVarArray mg2rec(env, totaltime, 0, IloInfinity);//Grid power sold
        IloNumVarArray mg3rec(env, totaltime, 0, IloInfinity);//Grid power sold

        IloBoolVarArray bin1(env, totaltime);
        IloBoolVarArray bin2(env, totaltime);
        IloBoolVarArray bin3(env, totaltime);


        IloExpr lowerObjectiveExpr(env);

        int M = 1000000;
        for (int i = 0; i < totaltime; i++)
        {
            lowerObjectiveExpr += gridbuy[i] * (Gridpowerbuy1[i] + Gridpowerbuy2[i] + Gridpowerbuy3[i]) - gridsell[i] * (Gridpowersell1[i]+Gridpowersell2[i] + Gridpowersell3[i]);
        }
        // Objective: minimize operational cost
        Model.add(IloMinimize(env, lowerObjectiveExpr));


        // Constraint: meet demand
       Model.add(statoc[0] == statoc[totaltime-1]);
        Model.add(statoc2[0] == statoc2[totaltime - 1]);
        Model.add(statoc3[0] == statoc3[totaltime - 1]);

         Model.add(Ca - IloMax(statoc) == 0);
         Model.add(Cb - IloMax(statoc2) == 0);
         Model.add(Cc - IloMax(statoc3) == 0);

        Model.add(Ca + Cc +  Cb + Cu== 300);
        //int ca = 500;
        for (int i = 0; i < totaltime; i++)
        {

            Model.add(0 <= statoc[i]);
            Model.add(Ca >= statoc[i]);

            Model.add(0 <= statoc2[i]);
            Model.add(Cb >= statoc2[i]);


            Model.add(0 <= statoc3[i]);
            Model.add(Cc >= statoc3[i]);

            if (i==0){
                Model.add(statoc[i] == statoc[totaltime - 1] + ((effin * (besschg1[i]) - ((bessdis1[i]) / effin))));
                Model.add(statoc2[i] == statoc[totaltime - 1] + ((effin * (besschg2[i]) - ((bessdis2[i]) / effin))));
                Model.add(statoc3[i] == statoc[totaltime - 1] + ((effin * (besschg3[i]) - ((bessdis3[i]) / effin))));
            }
            else {
                Model.add(statoc[i] == statoc[i - 1] + ((effin * (besschg1[i]) - ((bessdis1[i]) / effin))));
                Model.add(statoc2[i] == statoc2[i - 1] + ((effin * (besschg2[i]) - ((bessdis2[i]) / effin))));
                Model.add(statoc3[i] == statoc3[i - 1] + ((effin * (besschg3[i]) - ((bessdis3[i]) / effin))));
            }


            Model.add(0 <= besschg1[i] + besschg2[i] + besschg3[i]);
            Model.add(besschg1[i] + besschg2[i] + besschg3[i] <= 100 * bina1[i]);

            Model.add(0 <= bessdis1[i] + bessdis2[i] + bessdis3[i]);
            Model.add(bessdis1[i] + bessdis2[i] + bessdis3[i] <= 100 * bina1[i]);

            Model.add(0 <= bina1[i] + bina2[i]);
            Model.add(1 >= bina1[i] + bina2[i]);



           // Model.add(0 <= binb1[i] + binb2[i]);
            //Model.add(1 >= binb1[i] + binb2[i]);


           // Model.add(0 <= binc1[i] + binc2[i]);
           // Model.add(1 >= binc1[i] + binc2[i]);


            Model.add(mg1send[i] + mg2send[i] + mg3send[i] == mg1rec[i] + mg2rec[i] + mg3rec[i]);
            Model.add(mg1send[i] <= M * bin1[i]); // if mg1send[i] > 0 then bin1[i] will be 1
            Model.add(bessdis1[i] <= (1 - bin1[i]) * M); // if bin1[i] = 1 then bessdis1[i] must be 0
            Model.add(Gridpowerbuy1[i] <= (1 - bin1[i]) * M); // if bin1[i] = 1 then Gridpowerbuy1[i] must be 0

            // Constraints for mg2send[i]
            Model.add(mg2send[i] <= M * bin2[i]); // if mg2send[i] > 0 then bin2[i] will be 1
            Model.add(bessdis2[i] <= (1 - bin2[i]) * M); // if bin2[i] = 1 then bessdis2[i] must be 0
            Model.add(Gridpowerbuy2[i] <= (1 - bin2[i]) * M); // if bin2[i] = 1 then Gridpowerbuy2[i] must be 0

            Model.add(mg3send[i] <= M * bin3[i]); // if mg2send[i] > 0 then bin2[i] will be 1
            Model.add(bessdis3[i] <= (1 - bin3[i]) * M); // if bin2[i] = 1 then bessdis2[i] must be 0
            Model.add(Gridpowerbuy3[i] <= (1 - bin3[i]) * M); // if bin2[i] = 1 then Gridpowerbuy2[i] must be 0

            Model.add(Gridpowerbuy1[i] + mg1rec[i] + bessdis1[i] == mg1short[i]);
            Model.add(Gridpowersell1[i] + mg1send[i] + besschg1[i] == mg1sur[i]);
            Model.add(Gridpowerbuy2[i] + mg2rec[i] + bessdis2[i] == mg2short[i]);
            Model.add(Gridpowersell2[i] + mg2send[i] + besschg2[i] == mg2sur[i]);
            Model.add(Gridpowerbuy3[i] + mg3rec[i] + bessdis3[i] == mg3short[i]);
            Model.add(Gridpowersell3[i] + mg3send[i] + besschg3[i] == mg3sur[i]);


        }




#pragma endregion


        IloCplex cplex(Model);

        cplex.solve();


        //std::ofstream outfile("output.csv");
        std::ofstream outfile("Allocation.csv");


        cout << "Solution status: " << cplex.getStatus() << endl;
        cout << "Allocation,Ca," << cplex.getValue(Ca) << ",Cb," << cplex.getValue(Cb) << ",Cc," << cplex.getValue(Cc) << ",Cu," << cplex.getValue(Cu)<<endl;
        cout << "Minimized Objective Funtion " << cplex.getObjValue() << endl;
        if (outfile.is_open())
        {
            outfile << "Solution status: " << cplex.getStatus() << "\n";
            outfile << "Allocation,Ca," << cplex.getValue(Ca) << ",Cb," << cplex.getValue(Cb) << ",Cc," << cplex.getValue(Cc) << ",Cu," << cplex.getValue(Cu) << endl;
            outfile << "Minimized Objective Function: " << cplex.getObjValue() << "\n\n";

            // Column headers for the CSV
            outfile << "Gridpowerbuy1,mg1send,mg1rec,bessdis1,besschg1,Gridpowersell1,Gridpowerbuy2,mg2send,mg2rec,bessdis2,besschg2,Gridpowersell2,Gridpowerbuy3,mg3send,mg3rec,bessdis3,besschg3,Gridpowersell3,statoc1,statoc2,statoc3" << "\n";

            for (int i = 0; i < totaltime; i++)
            {
                outfile << cplex.getValue(Gridpowerbuy1[i]) << ",";
                outfile << cplex.getValue(mg1send[i]) << ",";
                outfile << cplex.getValue(mg1rec[i]) << ",";
                outfile << cplex.getValue(bessdis1[i]) << ",";
                outfile << cplex.getValue(besschg1[i]) << ",";
                outfile << cplex.getValue(Gridpowersell1[i]) << ",";
                outfile << cplex.getValue(Gridpowerbuy2[i]) << ",";
                outfile << cplex.getValue(mg2send[i]) << ",";
                outfile << cplex.getValue(mg2rec[i]) << ",";
                outfile << cplex.getValue(bessdis2[i]) << ",";
                outfile << cplex.getValue(besschg2[i]) << ",";
                outfile << cplex.getValue(Gridpowersell2[i]) << ",";
                outfile << cplex.getValue(Gridpowerbuy3[i]) << ",";
                outfile << cplex.getValue(mg3send[i]) << ",";
                outfile << cplex.getValue(mg3rec[i]) << ",";
                outfile << cplex.getValue(bessdis3[i]) << ",";
                outfile << cplex.getValue(besschg3[i]) << ",";
                outfile << cplex.getValue(Gridpowersell3[i]) << ",";
                outfile << cplex.getValue(statoc[i]) << ",";
                outfile << cplex.getValue(statoc2[i]) << ",";
                outfile << cplex.getValue(statoc3[i]) << "\n";
            }

            outfile.close();
        }
        else
        {
            std::cerr << "Unable to open file for writing!" << std::endl;
        }
    }
    catch (IloException& ex) {
        cerr << "Error: " << ex << endl;
    }






    env.end();
    return 0;
}
