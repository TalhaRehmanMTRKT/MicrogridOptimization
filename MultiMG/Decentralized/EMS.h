#include <ilcplex/ilocplex.h>
#include<chrono>
#include <iostream>
#include <fstream>
#include <map>

using namespace std;

typedef IloArray<IloNumVarArray> NumVar2D;


class EMS {

public:
    void MgOptimizationModel();
    void solve();
    double obj;

    IloNumVarArray PGbuy;
    IloNumVarArray PGsell;
    IloNumVarArray HGbuy;
    IloNumVarArray HGsell;
  

    EMS(int T, int numEvs, const std::map<std::string, int>& intConstants, const std::map<std::string, float>& floatConstants, int* ta, int* td, float* evsocini, float* evcap, int* Pload, int* Hload, int* Cload, int* CGbuy, int* CGsell, int* CHbuy, int* CHsell, float* Rdg1)
        : T_(T), numEvs_(numEvs), intConstants_(intConstants), floatConstants_(floatConstants), ta_(ta), td_(td), evsocini_(evsocini), evcap_(evcap), Pload_(Pload), Hload_(Hload), Cload_(Cload), CGbuy_(CGbuy), CGsell_(CGsell), CHbuy_(CHbuy), CHsell_(CHsell), Rdg1_(Rdg1) {

        env_ = IloEnv();
        model_ = IloModel(env_);

        InitializeVariables();
    }

    ~EMS() {
        env_.end();
    }


private:
    IloEnv env_;
    IloModel model_;
    int T_;
    int numEvs_;
    std::map<std::string, int> intConstants_;
    std::map<std::string, float> floatConstants_;
    int* ta_;
    int* td_;
    float* evsocini_;
    float* evcap_;
    int* Pload_;
    int* Hload_;
    int* Cload_;
    int* CGbuy_;
    int* CGsell_;
    int* CHbuy_;
    int* CHsell_;
    float* Rdg1_;

    IloNumVarArray statoc;
    IloNumVarArray Bchg;
    IloNumVarArray Bdischg;
    IloNumVarArray HSSsoc;
    IloNumVarArray Hchg;
    IloNumVarArray Hdischg;
    IloNumVarArray Pdg1;
    IloNumVarArray Pdg2;
    IloNumVarArray Pchp1;
    IloNumVarArray Pchp2;
    IloNumVarArray Hhob;
    IloNumVarArray Hchp1;
    IloNumVarArray Hchp2;
    IloNumVarArray Hac;
    IloNumVarArray Pec;

    // Electric Vehicles Decision Variables
    NumVar2D Pevchg;
    NumVar2D Pevdischg;
    NumVar2D evsoc;

    void InitializeVariables() {
        // Initialize decision variables
        PGbuy = IloNumVarArray(env_, T_, 0, IloInfinity);//Grid power bought
        PGsell = IloNumVarArray(env_, T_, 0, IloInfinity);//Grid power sold
        HGbuy = IloNumVarArray(env_, T_, 0, IloInfinity, ILOINT);//Grid Heat bought
        HGsell = IloNumVarArray(env_, T_, 0, IloInfinity, ILOINT);//Grid Heat sold
        statoc = IloNumVarArray(env_, T_, 0, 1); //battery storage capacity
        Bchg = IloNumVarArray(env_, T_, 0, 100); //battery charging
        Bdischg = IloNumVarArray(env_, T_, 0, 100); //battery discharging
        HSSsoc = IloNumVarArray(env_, T_, 0, 1); //battery storage capacity
        Hchg = IloNumVarArray(env_, T_, 0, 50, ILOINT); //battery charging
        Hdischg = IloNumVarArray(env_, T_, 0, 50, ILOINT); //battery discharging
        Pdg1 = IloNumVarArray(env_, T_, 0, 100, ILOINT); //DGA power generation range
        Pdg2 = IloNumVarArray(env_, T_, 0, 80, ILOINT); //DGA power generation range
        Pchp1 = IloNumVarArray(env_, T_, 30, 60, ILOINT); //CHP1 power generation range
        Pchp2 = IloNumVarArray(env_, T_, 50, 100, ILOINT); //CHP2 power generation range
        Hhob = IloNumVarArray(env_, T_, 0, 80, ILOINT); //HOB heat power generation range
        Hchp1 = IloNumVarArray(env_, T_, 0, IloInfinity); //CHP1 heat power generation range
        Hchp2 = IloNumVarArray(env_, T_, 0, IloInfinity); //CHP2 heat generation range
        Hac = IloNumVarArray(env_, T_, 0, IloInfinity); // AC
        Pec = IloNumVarArray(env_, T_, 0, IloInfinity); // Electric Chiller

        // Electric Vehicles Decision Variables
        Pevchg = NumVar2D(env_, numEvs_);
        Pevdischg = NumVar2D(env_, numEvs_);
        evsoc = NumVar2D(env_, numEvs_);

        for (int n = 0; n < numEvs_; n++) {
            Pevchg[n] = IloNumVarArray(env_, T_, 0, evcap_[n], ILOFLOAT);
            Pevdischg[n] = IloNumVarArray(env_, T_, 0, evcap_[n], ILOFLOAT);
            evsoc[n] = IloNumVarArray(env_, T_, 0, 1, ILOFLOAT);
        }
    }

};