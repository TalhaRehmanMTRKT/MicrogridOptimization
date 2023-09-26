#include"EMS.h"





void EMS::MgOptimizationModel() {

#pragma region Objective Function
    IloExpr objective(env_);


    for (int t = 0; t < T_; t++)
    {
        objective += intConstants_["Cdg1"] * Pdg1[t] + intConstants_["Cdg2"] * Pdg2[t] + CGbuy_[t] * PGbuy[t] - CGsell_[t] * PGsell[t] + CHbuy_[t] * HGbuy[t] - CHsell_[t] * HGsell[t] + intConstants_["Chob"] * Hhob[t] + intConstants_["Cchp1"] * Pchp1[t] + intConstants_["Cchp2"] * Pchp2[t];
    }


    // Objective: minimize cost
    model_.add(IloMinimize(env_, objective));

#pragma endregion



#pragma region Constraints


    // Constraint: meet demand

    for (int t = 0; t < T_; t++)
    {

        model_.add(0 <= Pdg1[t]);
        model_.add(Pdg1[t] <= 100);

        model_.add(0 <= Pdg2[t]);
        model_.add(Pdg2[t] <= 80);

        model_.add(0 <= Hhob[t]);
        model_.add(Hhob[t] <= 80);

        model_.add(30 <= Pchp1[t]);
        model_.add(Pchp1[t] <= 60);

        model_.add(50 <= Pchp2[t]);
        model_.add(Pchp2[t] <= 100);

        model_.add(Hchp1[t] == (floatConstants_["k1"]) * Pchp1[t]);
        model_.add(Hchp2[t] == (floatConstants_["k2"]) * Pchp2[t]);

        model_.add(0 <= HSSsoc[t]);
        model_.add(HSSsoc[t] <= 1);

        for (int n = 0; n < numEvs_; n++)
        {

            // soc bounds
            model_.add(0 <= evsoc[n][t]);
            model_.add(evsoc[n][t] <= 1);

            // Ev battery Constraints
            if (t == ta_[n])
            {
                model_.add(evsoc[n][t] == evsocini_[n] + (floatConstants_["Eveffin"] * Pevchg[n][t] - Pevdischg[n][t] / floatConstants_["Eveffin"]) / evcap_[n]);
                model_.add(0 <= Pevchg[n][t]);
                model_.add(0 <= Pevdischg[n][t]);
                model_.add(Pevchg[n][t] <= (evcap_[n] * (1 - evsocini_[n]) / floatConstants_["Eveffin"]));
                model_.add(Pevdischg[n][t] <= (evcap_[n] * evsocini_[n] * floatConstants_["Eveffin"]));
            }
            else if (t > ta_[n] && t <= td_[n])

            {
                model_.add(evsoc[n][t] == evsoc[n][t - 1] + ((floatConstants_["Eveffin"] * Pevchg[n][t] - (Pevdischg[n][t] / floatConstants_["Eveffin"])) / evcap_[n]));
                model_.add(0 <= Pevchg[n][t]);
                model_.add(0 <= Pevdischg[n][t]);
                model_.add(Pevchg[n][t] <= (evcap_[n] * (1 - evsoc[n][t - 1])) / floatConstants_["Eveffin"]);
                model_.add(Pevdischg[n][t] <= evcap_[n] * evsoc[n][t - 1] * floatConstants_["Eveffin"]);


                if (t == td_[n]) { model_.add(evsoc[n][t] >= 0.5); }

            }
            else
            {

                model_.add(Pevchg[n][t] == 0);
                model_.add(Pevdischg[n][t] == 0);

            }

        }


        // Adding the HSS and BSS Constraints constraints
        model_.add(0 <= statoc[t]);
        model_.add(statoc[t] <= 1);


        if (t == 0)
        {
            model_.add(statoc[t] == floatConstants_["socini"] + ((floatConstants_["effin"] * Bchg[t] - (Bdischg[t] / floatConstants_["effin"])) / intConstants_["Pbmax"]));
            model_.add(0 <= Bchg[t]);
            model_.add(0 <= Bdischg[t]);
            model_.add(Bchg[t] <= (intConstants_["Pbmax"] * (1 - floatConstants_["socini"]) / floatConstants_["effin"]));
            model_.add(Bdischg[t] <= (intConstants_["Pbmax"] * floatConstants_["socini"] * floatConstants_["effin"]));


            model_.add(HSSsoc[t] == 0.5 + ((floatConstants_["Heffin"] * Hchg[t] - (Hdischg[t] / floatConstants_["Heffin"])) / intConstants_["Hssmax"]));
            model_.add(0 <= Hchg[t]);
            model_.add(0 <= Hdischg[t]);
            //model_add(0 == Hdischg[t] * Hchg[t]);
            model_.add(Hchg[t] <= (intConstants_["Hssmax"] * (1 - 0.5) / floatConstants_["Heffin"]));
            model_.add(Hdischg[t] <= (intConstants_["Hssmax"] * 0.5 * floatConstants_["Heffin"]));

        }
        else
        {
            model_.add(statoc[t] == statoc[t - 1] + ((floatConstants_["effin"] * Bchg[t] - (Bdischg[t] / floatConstants_["effin"])) / intConstants_["Pbmax"]));
            model_.add(0 <= Bchg[t]);
            model_.add(0 <= Bdischg[t]);
            model_.add(Bchg[t] <= (intConstants_["Pbmax"] * (1 - statoc[t - 1])) / floatConstants_["effin"]);
            model_.add(Bdischg[t] <= intConstants_["Pbmax"] * statoc[t - 1] * floatConstants_["effin"]);


            model_.add(HSSsoc[t] == HSSsoc[t - 1] + ((floatConstants_["Heffin"] * Hchg[t] - (Hdischg[t] / floatConstants_["Heffin"])) / intConstants_["Hssmax"]));
            model_.add(0 <= Hchg[t]);
            model_.add(0 <= Hdischg[t]);
            //model_add(0 == Hdischg[t] * Hchg[t]);
            model_.add(Hchg[t] <= (intConstants_["Hssmax"] * (1 - HSSsoc[t - 1]) / floatConstants_["Heffin"]));
            model_.add(Hdischg[t] <= (intConstants_["Hssmax"] * HSSsoc[t - 1] * floatConstants_["Heffin"]));
        }


        // Preparing electric vehicles expression for the power balance equation
        IloExpr exp0(env_);
        IloExpr exp1(env_);
        for (int n = 0; n < numEvs_; n++)
        {
            exp0 += Pevchg[n][t];
            exp1 += Pevdischg[n][t];
        }

        // Adding the Electric power balance constraint

        model_.add(Pdg1[t] - Pec[t] + Pdg2[t] + Rdg1_[t] + Pchp1[t] + Pchp2[t] + Bdischg[t] + exp1 - exp0 - Bchg[t] + PGbuy[t] - PGsell[t] == Pload_[t]);

        model_.add(Hhob[t] - Hac[t] + Hchp1[t] + Hchp2[t] + HGbuy[t] - HGsell[t] - Hchg[t] + Hdischg[t] == Hload_[t]);

        model_.add(0.85 * Hac[t] + 0.95 * Pec[t] == Cload_[t]);

    }
#pragma endregion


}


resi EMS::solve() {

    MgOptimizationModel();


    IloCplex cplex(env_);
    cplex.extract(model_);
    cplex.exportModel("ModelLP.lp");
    cplex.setOut(env_.getNullStream());
    cplex.solve();
    obj = cplex.getObjValue();
    cout << "Solution status: " << cplex.getStatus() << endl;
    cout << "Minimized Objective Funtion of "<< name_ << " is :  " << obj << endl;
    

    int* pmgsur = new int[T_];
    int* pmgshort = new int[T_];
    int* Hmgsur = new int[T_];
    int* Hmgshort = new int[T_];

    for (int t = 0; t < T_; t++)
    {
        pmgsur[t] = cplex.getValue(PGsell[t]);
        pmgshort[t] = cplex.getValue(PGbuy[t]);

        Hmgsur[t] = cplex.getValue(HGsell[t]);
        Hmgshort[t] = cplex.getValue(HGbuy[t]);
    }


    resi results;
    results.pmgsur = pmgsur;
    results.pmgshort = pmgshort;
    results.Hmgsur = Hmgsur;
    results.Hmgshort = Hmgshort;

    IloNum eps = cplex.getParam(
        IloCplex::Param::MIP::Tolerances::Integrality);

    saveResultsToFile(cplex);


    return results;

}


void EMS::saveResultsToFile(
    IloCplex& cplex) {

    // Create and open the CSV file for writing
    ofstream outputFile(name_ + "_output.csv");

    if (outputFile.is_open()) {
        // Write the header row
        outputFile << "Time,Pload+Pec,Hload+Hac,Cload,CGbuy,CGsell,CHsell,CHbuy,Rdg1,PGshort/sur,statoc,Bchg/dischg,Pdg1,Pdg2,Pchp1,Pchp2,Hhob,Hchp1,Hchp2,HGshort/sur,Hchg/dischg";
        for (int mg = 0; mg < numEvs_; mg++) {
            outputFile << ",Pevchg/dischg" << mg + 1;
        }

        outputFile << std::endl;

        // Write the data rows
        for (int i = 0; i < T_; i++) {
            outputFile << i + 1 << "," << Pload_[i] + cplex.getValue(Pec[i]) << "," << Hload_[i] + cplex.getValue(Hac[i]) << "," << Cload_[i] << "," << CGbuy_[i] << ","
                << CGsell_[i] << "," << CHsell_[i] << ","
                << CHbuy_[i] << "," << Rdg1_[i] << ","
                << cplex.getValue(PGbuy[i]) - cplex.getValue(PGsell[i]) << ","
                << cplex.getValue(statoc[i]) << "," << -cplex.getValue(Bchg[i]) + cplex.getValue(Bdischg[i]) << "," << cplex.getValue(Pdg1[i]) << ","
                << cplex.getValue(Pdg2[i]) << ","
                << cplex.getValue(Pchp1[i]) << "," << cplex.getValue(Pchp2[i]) << ","
                << cplex.getValue(Hhob[i]) << "," << cplex.getValue(Hchp1[i]) << ","
                << cplex.getValue(Hchp2[i]) << "," << cplex.getValue(HGbuy[i]) - cplex.getValue(HGsell[i]) << "," << -cplex.getValue(Hchg[i]) + cplex.getValue(Hdischg[i]);

            for (int mg = 0; mg < numEvs_; mg++) {
                outputFile << "," << -cplex.getValue(Pevchg[mg][i]) + cplex.getValue(Pevdischg[mg][i]);
            }

            outputFile << std::endl;
        }

        // Close the CSV file
        outputFile.close();
        std::cout << "Data saved to " << name_ << std::endl;
    }
    else {
        std::cerr << "Failed to open the " << name_ << " file for writing." << std::endl;
    }
}



