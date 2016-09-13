#include <cmath>
#include "sa.h"

void SA_Floorplan(FPlan &fp, string filename_short, int k, float init_T, float term_T)
{
  int MT,uphill,reject;
  double pre_cost,best,cost;
  float d_cost,reject_rate;

  int N = k * fp.size();   //fp.size: # modules
  double T=init_T;
  float conv_rate = 0.99;
  double Tratio = 0.85;
 // double time=seconds();

  // get inital solution
  fp.packing();
  fp.keep_sol();
  fp.keep_best();
  pre_cost = best = fp.getCost();

  int good_num=0,bad_num=0;
  double total_cost=0;
  int count=0;
  //ofstream of("/tmp/btree_debug");

  int flag=0;

  ofstream frsa;
  string f_dir;
  f_dir = "./out/"+filename_short+".SA";
  frsa.open(f_dir.c_str());
  double t1 = clock();
  do{
	count++;
	total_cost = 0;
	MT=uphill=reject=0;
	printf("Iteration %d, T= %.2f\n", count,  T);
	//frsa << "Iteration= " << count << ", T= " << T << endl;


	//vector<double> chain;

	for(; uphill < N && MT < 2*N && flag==0; MT++){
		fp.perturb();
		fp.packing();
		cost = fp.getCost();
		d_cost = cost - pre_cost;
		float p = exp(-d_cost/T);
		//cout << "Temp = " << T << ", ";
		//cout << "d_cost = " << d_cost << ", cost=" << cost << ", ";
		//cout << "prob = " << p << endl;
		//getchar();

		//chain.push_back(cost);

		if(d_cost <=0 || rand_01() < p ){
			fp.keep_sol();
			pre_cost = cost;
			if(d_cost > 0){
				uphill++;
				bad_num++;
			}
			else if(d_cost < 0){
				good_num++;
			}
			// keep best solution
			if(cost < best){
				fp.keep_best();
				best = cost;
				printf("   ==>  Cost= %f, Area= %f, ", best, fp.getArea());
				printf("Wire= %f, ", fp.getWireLength());
				cout << endl;
				t1 = clock();
				double tdiff = t1 / CLOCKS_PER_SEC;
				frsa << best << " " << fp.getArea() << " " << fp.getWireLength() << " " << tdiff << endl;
				assert(fp.getArea() >= fp.getTotalArea());
			}
		}
		else{
			reject++;
			fp.recover();
		}
	}
	T = Tratio*T;
	reject_rate = float(reject)/MT;
	printf("  T= %.2f, reject= %.2f\n\n", T, reject_rate);
  }while(reject_rate < conv_rate && T > term_T);

  frsa.close();

  if(reject_rate >= conv_rate)
    cout << "\n  Convergent!\n";
  else if(T <= term_T)
    cout << "\n Cooling Enough!\n";

  fp.recover_best();
  fp.packing();
 // time = seconds();
 // return time;
}
