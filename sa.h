/*
 * sa.h
 *
 *  Created on: Apr 27, 2015
 *      Author: yang
 */

#ifndef SRC_SA_H_
#define SRC_SA_H_
#include "fp.h"

extern float init_avg;
extern float avg_ratio;
extern float lamda;

void SA_Floorplan(FPlan &fp, string filename_short, int k, float init_T, float term_T=0.1);
double Random_Floorplan(FPlan &fp,int times);



#endif /* SRC_SA_H_ */
