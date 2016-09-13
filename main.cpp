#include <iostream>
#include <cstring>
#include <stdlib.h> //atoi atof
#include <time.h>
#include "btree.h"
#include "sa.h"
//#include "sa.h"
int main(int argc,char **argv){
	string filename;
	double alpha = 0.5;
	double beta = 1-alpha;
	double times=500, init_temp=1000, term_temp=0.01;
	if(argc <= 5){
		cout << "Input: filename + alpha + times + init_temp + term_temp\n";
		exit(1);
	}
	else{
		filename = string(argv[1]);
		alpha = atof(argv[2]);
		beta = 1-alpha;
		times = atoi(argv[3]);
		init_temp = atof(argv[4]);
		term_temp = atof(argv[5]);
	}

	double t = clock();

	BTree fp(alpha, beta);
	fp.read(filename);
	cout << "Read file finished..." << endl;
	//fp.print_nets();
	//fp.print_modules();
	fp.init();
	cout << "Initial finished..." << endl;
	//fp.print_result();
	//fp.show_tree();
	//getchar();

	SA_Floorplan(fp, filename, times, init_temp, term_temp);
	t = (clock() -t)/CLOCKS_PER_SEC;
	fp.print_result();
	cout << "run time = " << t << endl;
	//fp.show_tree();
}
