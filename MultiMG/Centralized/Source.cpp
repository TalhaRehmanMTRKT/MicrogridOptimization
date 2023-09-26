#include <ilcplex/ilocplex.h>
#include<chrono>
#include <iostream>
#include <fstream>
#include <map>
ILOSTLBEGIN


typedef IloArray<IloNumVarArray> NumVar2D;
typedef IloArray<NumVar2D> NumVar3D;


int
main(int, char**)
{

#pragma region Input Data and constants

    int T = 24; //One day
    int NumEvs = 5; // Total Nmber of Electric vehicles
    int NumMg = 3; // Total Nmber of Microgrids
    int NumDg = 3; // Total Nmber of Diesel Generators
    int NumCHP = 3; // Total Nmber of Combine heat and power generation units
    int NumHOB = 1; // Total Number of Heat only boilers


    int** Cchp = new int* [NumMg];// add the randomly populating
    int** Cdg = new int* [NumMg];// add the randomly populating
    int** Chob = new int* [NumMg];// add the randomly populating
    float** k_chp = new float* [NumMg]; // add the randomly populating

    Cchp[0] = new int[NumCHP] {150, 145}; Cchp[1] = new int[NumCHP] { 150, 145 }; Cchp[2] = new int[NumCHP] { 150, 145 };
    Cdg[0] = new int[NumDg] {135, 140}; Cdg[1] = new int[NumDg] { 135, 140 }; Cdg[2] = new int[NumDg] { 135, 140 };
    Chob[0] = new int[NumHOB] {80}; Chob[1] = new int[NumHOB] {80}; Chob[2] = new int[NumHOB] {80};
    k_chp[0] = new float[NumCHP] {1.2, 0.8}; k_chp[1] = new float[NumCHP] {1.2, 0.8}; k_chp[2] = new float[NumCHP] {1.2, 0.8};


    // Arrival and Departure time of Evs 
    int** ta = new int* [NumMg];  // add the randomly populating
    int** td = new int* [NumMg];  // add the randomly populating
    float** evsocini = new float* [NumMg];  // add the randomly populating
    float* socini = new float[NumMg] {0.2, 0.2, 0.2};  // add the randomly populating
    float** evcap = new float* [NumMg]; // add the randomly populating 
    float** eveffin = new float* [NumMg]; // add the randomly populating 
    eveffin[0] = new float[NumEvs] { 0.90, 0.90, 0.90, 0.90, 0.90};
    eveffin[1] = new float[NumEvs] { 0.90, 0.90, 0.90, 0.90, 0.90};
    eveffin[2] = new float[NumEvs] { 0.90, 0.90, 0.90, 0.90, 0.90};


    ta[0] = new int[NumEvs] {2, 4, 6, 9, 9};
    ta[1] = new int[NumEvs] {2, 4, 6, 9, 9};
    ta[2] = new int[NumEvs] {2, 4, 6, 9, 9};

    td[0] = new int[NumEvs] {6, 7, 9, 12, 16};
    td[1] = new int[NumEvs] {6, 7, 9, 12, 16};
    td[2] = new int[NumEvs] {6, 7, 9, 12, 16};

    evsocini[0] = new float[NumEvs] {0.30, 0.20, 0.10, 0.70, 0.40};
    evsocini[1] = new float[NumEvs] {0.30, 0.20, 0.10, 0.70, 0.40};
    evsocini[2] = new float[NumEvs] {0.30, 0.20, 0.10, 0.70, 0.40};

    evcap[0] = new float[NumEvs] {38.3, 47.5, 28.9, 56, 52};
    evcap[1] = new float[NumEvs] {38.3, 47.5, 28.9, 56, 52};
    evcap[2] = new float[NumEvs] {38.3, 47.5, 28.9, 56, 52};

    
    float* Heffin = new float[NumMg] {0.95, 0.95, 0.95}; // add the randomly populating
    float* effin = new float [NumMg] {0.95, 0.95, 0.95}; // add the randomly populating



    int* Hsscap = new int[NumMg] {50,50,50}; // add the randomly populating 
    int* Bsscap = new int[NumMg] {150, 150, 150}; // add the randomly populating 

    int** dgcap = new int* [NumMg]; // add the randomly populating 
    int** chpcap = new int* [NumMg]; // add the randomly populating 
    int** hobcap = new int* [NumMg]; // add the randomly populating

    dgcap[0] = new int[NumDg] {100, 80}; dgcap[1] = new int[NumDg] { 100, 80 }; dgcap[2] = new int[NumDg] {100, 80};
    chpcap[0] = new int[NumCHP] {60, 100}; chpcap[1] = new int[NumCHP] { 60, 100}; chpcap[2] = new int[NumCHP] { 60, 100};
    hobcap[0] = new int[NumHOB] {80}; hobcap[1] = new int[NumHOB] {80}; hobcap[2] = new int[NumHOB] {80};


    int** dgmin = new int* [NumMg]; // add the randomly populating 
    int** chpmin = new int* [NumMg]; // add the randomly populating 
    int** hobmin = new int* [NumMg]; // add the randomly populating 

    dgmin[0] = new int[NumDg] {0, 0}; dgmin[1] = new int[NumDg] { 0, 0 }; dgmin[2] = new int[NumDg] {0, 0};
    chpmin[0] = new int[NumCHP] {30, 50}; chpmin[1] = new int[NumCHP] { 30, 50}; chpmin[2] = new int[NumCHP] { 30, 50};
    hobmin[0] = new int[NumHOB] {0}; hobmin[1] = new int[NumHOB] {0}; hobmin[2] = new int[NumHOB] {0};

    // Electric, Heat and Cooling Demand  // add the randomly populating 
    // Electric and Heat Demand 
    int* Pload_arr = new int[T] {169, 175, 179, 171, 181, 190, 270, 264, 273, 281, 300, 320, 280, 260, 250, 200, 180, 190, 240, 280, 325, 350, 300, 250};  //Electicity demand w.r.t tim
    int* Hload_arr = new int[T] {130, 125, 120, 120, 125, 135, 150, 160, 175, 190, 195, 200, 195, 195, 180, 170, 185, 190, 195, 200, 195, 190, 180, 175};  //Heat Demand
    int* Cload_arr = new int[T] {100, 100, 80, 100, 120, 135, 150, 135, 125, 130, 140, 150, 150, 130, 120, 110, 90, 80, 135, 150, 135, 140, 110, 125};  //Heat Demand


    // Electric, Heat and Cooling Demand  // add the randomly populating 
    int** Pload = new int* [3];  //Electicity demand w
    int** Hload = new int* [3];  //Heat Demand
    int** Cload = new int* [3];  //Cooling Demand

    float* scaling = new float[3] {1, 0.5, 2};

    for (int i = 0; i < 3; i++)
    {
        Pload[i] = new int[T];
        Hload[i] = new int[T];
        Cload[i] = new int[T];

        for (int t = 0; t < T; t++)
        {
            Pload[i][t] = static_cast<int>(Pload_arr[t] * scaling[i]);
            Hload[i][t] = static_cast<int>(Hload_arr[t] * scaling[i]);
            Cload[i][t] = static_cast<int>(Cload_arr[t] * 1);
        }

    }


    // Renewable Infeeds  // Make the Scenario(Random) Generator for this
    float** Rdg = new float* [NumMg]; //Power generated by PV w.r.t time

    Rdg[0] = new float[T] {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 10, 15, 20, 23, 28, 33, 35, 34, 31, 28, 10, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 }; //Power generated by PV w.r.t time
    Rdg[1] = new float[T] {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 10, 15, 20, 23, 28, 33, 35, 34, 31, 28, 10, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 }; //Power generated by PV w.r.t time
    Rdg[2] = new float[T] {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 10, 15, 20, 23, 28, 33, 35, 34, 31, 28, 10, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 }; //Power generated by PV w.r.t time


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

#pragma endregion


    IloEnv env;
    IloModel model(env);


#pragma region Decision Variables

    /// Electric Network Elements ///
//Gensets
    NumVar3D powerdg(env, NumMg);
    NumVar3D powerchp(env, NumMg);

    //Electric Battery
    NumVar2D BssSOC(env, NumMg);
    NumVar2D Bsschg(env, NumMg);
    NumVar2D Bssdischg(env, NumMg);



    //Electric Vehicles
    NumVar3D evSOC(env, NumMg);
    NumVar3D evchg(env, NumMg);
    NumVar3D evdischg(env, NumMg);


    //Electric Surplus and Shortage
    NumVar2D psur(env, NumMg);
    NumVar2D pshort(env, NumMg);


    /// Heat Network Elements ///

// Thermal Gensets
    NumVar3D heatchp(env, NumMg);
    NumVar3D heathob(env, NumMg);

    // Thermal storage system
    NumVar2D HssSOC(env, NumMg);
    NumVar2D Hsschg(env, NumMg);
    NumVar2D Hssdischg(env, NumMg);



    //Electric Surplus and Shortage
    NumVar2D Hsur(env, NumMg);
    NumVar2D Hshort(env, NumMg);

    /// Cooling Network Elements ///

// Cooling energy Gensets
    NumVar2D powerEC(env, NumMg);
    NumVar2D heatAC(env, NumMg);


    /// Grid Elements ///

//Electric power buy and sell
    NumVar2D powergridbuy(env, NumMg);
    NumVar2D powergridsell(env, NumMg);

    //Thermal power buy and sell
    NumVar2D heatgridbuy(env, NumMg);
    NumVar2D heatgridsell(env, NumMg);


    /// Internal microgrid trading Elements ///

    NumVar3D powersend(env, NumMg);
    NumVar3D powersrec(env, NumMg);
    NumVar3D heatsend(env, NumMg);
    NumVar3D heatrec(env, NumMg);


    /// Populating the Decision Varaibales 2D and 3D arrays ///

    for (int mg = 0; mg < NumMg; mg++)
    {

        powerEC[mg] = IloNumVarArray(env, T, 0, IloInfinity);
        heatAC[mg] = IloNumVarArray(env, T, 0, IloInfinity);

        powergridbuy[mg] = IloNumVarArray(env, T, 0, IloInfinity);
        powergridsell[mg] = IloNumVarArray(env, T, 0, IloInfinity);
        heatgridbuy[mg] = IloNumVarArray(env, T, 0, IloInfinity);
        heatgridsell[mg] = IloNumVarArray(env, T, 0, IloInfinity);

        HssSOC[mg] = IloNumVarArray(env, T, 0, 1);;
        Hsschg[mg] = IloNumVarArray(env, T, 0, IloInfinity);
        Hssdischg[mg] = IloNumVarArray(env, T, 0, IloInfinity);
        BssSOC[mg] = IloNumVarArray(env, T, 0, 1);
        Bsschg[mg] = IloNumVarArray(env, T, 0, IloInfinity);
        Bssdischg[mg] = IloNumVarArray(env, T, 0, IloInfinity);

        psur[mg] = IloNumVarArray(env, T, 0, IloInfinity);
        pshort[mg] = IloNumVarArray(env, T, 0, IloInfinity);
        Hsur[mg] = IloNumVarArray(env, T, 0, IloInfinity);
        Hshort[mg] = IloNumVarArray(env, T, 0, IloInfinity);



        powerdg[mg] = NumVar2D(env, NumDg);

        for (int dg = 0; dg < NumDg; dg++)
        {
            powerdg[mg][dg] = IloNumVarArray(env, T, dgmin[mg][dg], dgcap[mg][dg]);
        }


        powerchp[mg] = NumVar2D(env, NumCHP);
        heatchp[mg] = NumVar2D(env, NumCHP);

        for (int chp = 0; chp < NumCHP; chp++)
        {
            powerchp[mg][chp] = IloNumVarArray(env, T, chpmin[mg][chp], dgcap[mg][chp]);
            heatchp[mg][chp] = IloNumVarArray(env, T, 0, IloInfinity); // Recheck to add the limit here in addition to the constraints
        }

        heathob[mg] = NumVar2D(env, NumHOB);
        for (int hob = 0; hob < NumHOB; hob++)
        {
            heathob[mg][hob] = IloNumVarArray(env, T, hobmin[mg][hob], hobcap[mg][hob]);
        }


        evSOC[mg] = NumVar2D(env, NumEvs);
        evchg[mg] = NumVar2D(env, NumEvs);
        evdischg[mg] = NumVar2D(env, NumEvs);

        for (int ev = 0; ev < NumEvs; ev++)
        {
            evSOC[mg][ev] = IloNumVarArray(env, T, 0, 1);
            evchg[mg][ev] = IloNumVarArray(env, T, 0, evcap[mg][ev]);
            evdischg[mg][ev] = IloNumVarArray(env, T, 0, evcap[mg][ev]);
        }

        powersend[mg] = NumVar2D(env, NumMg);;
        powersrec[mg] = NumVar2D(env, NumMg);;
        heatsend[mg] = NumVar2D(env, NumMg);;
        heatrec[mg] = NumVar2D(env, NumMg);;



        for (int mgl = 0; mgl < NumMg; mgl++)
        {
            powersend[mg][mgl] = IloNumVarArray(env, T, 0, IloInfinity);
            powersrec[mg][mgl] = IloNumVarArray(env, T, 0, IloInfinity);
            heatsend[mg][mgl] = IloNumVarArray(env, T, 0, IloInfinity);
            heatrec[mg][mgl] = IloNumVarArray(env, T, 0, IloInfinity);
        }

    }

#pragma endregion



#pragma region Objective Function

    IloExpr objective(env);


    for (int t = 0; t < T; t++)
    {

        IloExpr exp0(env);
        IloExpr exp1(env);
        IloExpr exp2(env);
        IloExpr exp3(env);

        IloExpr exp4(env);
        IloExpr exp5(env);
        IloExpr exp6(env);

        for (int mg = 0; mg < NumMg; mg++)
        {
            exp0 += powergridbuy[mg][t];
            exp1 += powergridsell[mg][t];
            exp2 += heatgridbuy[mg][t];
            exp3 += heatgridsell[mg][t];

            for (int dg = 0; dg < NumDg; dg++)
            {
                exp4 += Cdg[mg][dg] * powerdg[mg][dg][t];
            }

            for (int chp = 0; chp < NumCHP; chp++)
            {
                exp5 += Cchp[mg][chp] * powerchp[mg][chp][t];
            }

            for (int hob = 0; hob < NumHOB; hob++)
            {
                exp6 += Chob[mg][hob] * heathob[mg][hob][t];
            }


        }

        objective += CGbuy[t] * exp0 - CGsell[t] * exp1 + CHbuy[t] * exp2 - CHsell[t] * exp3 + exp4 + exp5 + exp6;


    }

    model.add(IloMinimize(env, objective));

#pragma endregion


#pragma region constraints



    for (int t = 0; t < T; t++)
    {



        for (int mg = 0; mg < NumMg; mg++)
        {
            for (int dg = 0; dg < NumDg; dg++)
            {
                model.add(dgmin[mg][dg] <= powerdg[mg][dg][t]);
                model.add(powerdg[mg][dg][t] <= dgcap[mg][dg]);
            }


            for (int chp = 0; chp < NumCHP; chp++)
            {
                model.add(chpmin[mg][chp] <= powerchp[mg][chp][t]);
                model.add(powerchp[mg][chp][t] <= chpcap[mg][chp]);
                model.add(heatchp[mg][chp][t] == k_chp[mg][chp] * powerchp[mg][chp][t]);
            }

            for (int hob = 0; hob < NumHOB; hob++)
            {
                model.add(hobmin[mg][hob] <= heathob[mg][hob][t]);
                model.add(heathob[mg][hob][t] <= hobcap[mg][hob]);
            }



            for (int n = 0; n < NumEvs; n++)
            {

                // soc bounds
                model.add(0 <= evSOC[mg][n][t]);
                model.add(evSOC[mg][n][t] <= 1);

                // Ev battery Constraints
                if (t == ta[mg][n])
                {
                    model.add(evSOC[mg][n][t] == evsocini[mg][n] + (eveffin[mg][n] * evchg[mg][n][t] - evdischg[mg][n][t] / eveffin[mg][n]) / evcap[mg][n]);
                    model.add(0 <= evchg[mg][n][t]);
                    model.add(0 <= evdischg[mg][n][t]);
                    model.add(evchg[mg][n][t] <= (evcap[mg][n] * (1 - evsocini[mg][n]) / eveffin[mg][n]));
                    model.add(evdischg[mg][n][t] <= (evcap[mg][n] * evsocini[mg][n] * eveffin[mg][n]));
                }
                else if (t > ta[mg][n] && t <= td[mg][n])

                {
                    model.add(evSOC[mg][n][t] == evSOC[mg][n][t-1] + ((eveffin[mg][n] * evchg[mg][n][t] - (evdischg[mg][n][t] / eveffin[mg][n])) / evcap[mg][n]));
                    model.add(0 <= evchg[mg][n][t]);
                    model.add(0 <= evdischg[mg][n][t]);
                    model.add(evchg[mg][n][t] <= (evcap[mg][n] * (1 - evSOC[mg][n][t-1])) / eveffin[mg][n]);
                    model.add(evdischg[mg][n][t] <= evcap[mg][n] * evSOC[mg][n][t-1] * eveffin[mg][n]);


                if (t == td[mg][n]) { model.add(evSOC[mg][n][t] >= 0.5); }

                }
                else
                {

                    model.add(evchg[mg][n][t] == 0);
                    model.add(evdischg[mg][n][t] == 0);

                }

            }

           


            // Adding the HSS and BSS Constraints constraints
            model.add(0 <= BssSOC[mg][t]);
            model.add(BssSOC[mg][t] <= 1);
            model.add(0 <= HssSOC[mg][t]);
            model.add(HssSOC[mg][t] <= 1);

            if (t == 0)
            {
                model.add(BssSOC[mg][t] == socini[mg] + ((effin[mg]  * Bsschg[mg][t] - (Bssdischg[mg][t] / effin[mg])) / Bsscap[mg]));
                model.add(0 <= Bsschg[mg][t]);
                model.add(0 <= Bssdischg[mg][t]);
                model.add(Bsschg[mg][t] <= (Bsscap[mg] * (1 - socini[mg]) / effin[mg]));
                model.add(Bssdischg[mg][t] <= (Bsscap[mg] * socini[mg] * effin[mg]));


                model.add(HssSOC[mg][t] == 0.5 + ((Heffin[mg] * Hsschg[mg][t] - (Hssdischg[mg][t] / Heffin[mg])) / Hsscap[mg]));
                model.add(0 <= Hsschg[mg][t]);
                model.add(0 <= Hssdischg[mg][t]);
                model.add(Hsschg[mg][t] <= (Hsscap[mg] * (1 - 0.5) / Heffin[mg]));
                model.add(Hssdischg[mg][t] <= (Hsscap[mg] * 0.5 * Heffin[mg]));

            }
            else
            {
                model.add(BssSOC[mg][t] == BssSOC[mg][t-1] + ((effin[mg] * Bsschg[mg][t] - (Bssdischg[mg][t] / effin[mg])) / Bsscap[mg]));
                model.add(0 <= Bsschg[mg][t]);
                model.add(0 <= Bssdischg[mg][t]);
                model.add(Bsschg[mg][t] <= (Bsscap[mg] * (1 - BssSOC[mg][t - 1])) / effin[mg]);
                model.add(Bssdischg[mg][t] <= Bsscap[mg] * BssSOC[mg][t - 1] * effin[mg]);


                model.add(HssSOC[mg][t] == HssSOC[mg][t - 1] + ((Heffin[mg] * Hsschg[mg][t] - (Hssdischg[mg][t] / Heffin[mg])) / Hsscap[mg]));
                model.add(0 <= Hsschg[mg][t]);
                model.add(0 <= Hssdischg[mg][t]);
                model.add(Hsschg[mg][t] <= (Hsscap[mg] * (1 - HssSOC[mg][t - 1]) / Heffin[mg]));
                model.add(Hssdischg[mg][t] <= (Hsscap[mg] * HssSOC[mg][t - 1] * Heffin[mg]));
            }


            // Preparing electric vehicles expression for the power balance equation
            IloExpr exp0(env);
            IloExpr exp1(env);
            for (int n = 0; n < NumEvs; n++)
            {
                exp0 += evchg[mg][n][t];
                exp1 += evdischg[mg][n][t];
            }
            IloExpr dgpsum(env);
            for (int dg = 0; dg < NumDg; dg++)
            {
                dgpsum += powerdg[mg][dg][t];
            }
            IloExpr chpEsum(env);
            IloExpr chpHsum(env);
            for (int chp = 0; chp < NumCHP; chp++)
            {
                chpEsum += powerchp[mg][chp][t];
                chpHsum += heatchp[mg][chp][t];

            }
            IloExpr hobsum(env);
            for (int hob = 0; hob < NumHOB; hob++)
            {
                hobsum += heathob[mg][hob][t];
            }


            
            
            IloExpr  a(env);
            IloExpr  b(env);
            IloExpr  c(env);
            IloExpr  d(env);

            for (int l = 0; l < NumMg; l++)
            {
                a += powersend[mg][l][t];
                b += powersrec[mg][l][t];
                c += heatsend[mg][l][t];
                d += heatrec[mg][l][t];
            }

            // Adding the Electric power balance constraint


            model.add(dgpsum - powerEC[mg][t] + Rdg[mg][t] + chpEsum + Bssdischg[mg][t]  + powergridbuy[mg][t]  -a+b + exp1 - exp0 - Bsschg[mg][t]  - powergridsell[mg][t] == Pload[mg][t]);

            model.add(hobsum - heatAC[mg][t] + chpHsum +heatgridbuy[mg][t]- heatgridsell[mg][t] - Hsschg[mg][t] -c+d + Hssdischg[mg][t] == Hload[mg][t]);

            model.add(0.85 * heatAC[mg][t] + 0.95 * powerEC[mg][t] == Cload[mg][t]);


        }

    }





    IloExpr exp0(env);
    IloExpr exp1(env);
    IloExpr exp2(env);
    IloExpr exp3(env);

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

                        model.add(powersend[k][l][t] == 0);
                        model.add(powersrec[k][l][t] == 0);
                        model.add(heatsend[k][l][t] == 0);
                        model.add(heatrec[k][l][t] == 0);
                }
            }
        }
        model.add(exp0 == exp1);
        model.add(exp2 == exp3);
    }



#pragma endregion



#pragma endregion
    IloCplex cplex(env);
    cplex.extract(model);
    cplex.exportModel("ModelLP.lp");
    cplex.setOut(env.getNullStream());
    cplex.solve();
    double obj = cplex.getObjValue();
    cout << "Solution status: " << cplex.getStatus() << endl;
    cout << "Minimized Objective Funtion of " << obj << endl;



    for(int mg = 0; mg < NumMg; mg++)
    {

    // Create and open the CSV file for writing
    ofstream outputFile(to_string(mg) + "_output.csv");
    // Create and open the CSV file for writing

    if (outputFile.is_open()) {

        // Write the header row
        outputFile << "Time,Pload+Pec,Hload+Hac,Cload,CGbuy,CGsell,CHsell,CHbuy,Rdg,PGshort/sur,statoc,Bchg/dischg,Pdg,Pchp,Hhob,Hchp,HGshort/sur,Hchg/dischg,MgSend/rec";
        for (int ev = 0; ev < NumEvs; ev++) {
            outputFile << ",Pevchg/dischg" << ev + 1;
        }

        outputFile << std::endl;



        // Write the data rows
        for (int i = 0; i < T; i++) {


            double dg_power = 0;

            for (int dg = 0; dg < NumDg; dg++)
            {
                dg_power += cplex.getValue(powerdg[mg][dg][i]);

            }


            double chp_power = 0;
            double chp_heat = 0; 
            for (int chp = 0; chp < NumCHP; chp++)
            {
                chp_power += cplex.getValue(powerchp[mg][chp][i]);
                chp_heat += cplex.getValue(heatchp[mg][chp][i]);

            }

            double hob_power = 0;

            for (int hob = 0; hob < NumHOB; hob++)
            {
                hob_power += cplex.getValue(heathob[mg][hob][i]);

            }
            double ps=0; double pr = 0;



            for (int l = 0; l < NumMg; l++)
            {
                ps += cplex.getValue(powersend[mg][l][i]);
                pr += cplex.getValue(powersrec[mg][l][i]);
               
            }

            outputFile << i + 1 << "," << Pload[mg][i] + cplex.getValue(powerEC[mg][i]) << "," << Hload[mg][i] + cplex.getValue(heatAC[mg][i]) << "," << Cload[mg][i] << "," << CGbuy[i] << ","
                << CGsell[i] << "," << CHsell[i] << ","
                << CHbuy[i] << "," << Rdg[mg][i] << ","
                << cplex.getValue(powergridbuy[mg][i]) - cplex.getValue(powergridsell[mg][i]) << ","
                << cplex.getValue(BssSOC[mg][i]) << "," << -cplex.getValue(Bsschg[mg][i]) + cplex.getValue(Bssdischg[mg][i]) << "," << dg_power<< ","
                << chp_power << ","
                << hob_power << "," << chp_heat << "," << cplex.getValue(heatgridbuy[mg][i]) - cplex.getValue(heatgridsell[mg][i]) << "," << -cplex.getValue(Hsschg[mg][i]) + cplex.getValue(Hssdischg[mg][i]) << "," << -ps + pr;


            for (int ev = 0; ev < NumEvs; ev++) {
                outputFile << "," << -cplex.getValue(evchg[mg][ev][i]) + cplex.getValue(evdischg[mg][ev][i]);
            }

            outputFile << std::endl;



        }


        // Close the CSV file
        outputFile.close();
        std::cout << "Data saved to " << mg << std::endl;
    }
    else {
        std::cerr << "Failed to open the " << mg << " file for writing." << std::endl;
    }







    }

    ofstream outFile("All_output.csv");



    if (outFile.is_open()) {

        outFile << "Mg1Send/rec" << "," << "mg1buy_sell" << "," << "Mg2Send/rec" << "," << "mg2buy_sell" << "," << "Mg3Send/rec"  << "," << "mg3buy_sell" << endl;

        for (int t = 0; t < T; t++)
        {

            double gridtrade=0;
            for (int mg = 0; mg < NumMg; mg++)
            {
                double gridtrade = 0;

                double ps = 0; double pr = 0;
                for (int l = 0; l < NumMg; l++)
                {
                    ps += cplex.getValue(powersend[mg][l][t]);
                    pr += cplex.getValue(powersrec[mg][l][t]);
                }

                gridtrade = cplex.getValue(powergridbuy[mg][t]) - cplex.getValue(powergridsell[mg][t]);

                outFile << -ps+pr << ","<< gridtrade<<",";
            }


            outFile<< endl;
        }


    }









    return 1;

}