#ifndef SRC_FP_H_
#define SRC_FP_H_

#define INF (0x7FFFFFFF)

#include <vector>
#include <string>
#include <fstream>
#include <map>
#include <cstdio>
#include <assert.h> //assert
#include <iostream> //cout

#include <math.h>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <random>
#include <stdio.h>
#include <stdlib.h>
#include <utility>

using namespace std;



struct Module{
  int id;
  string name;
  double width,height; //module width and height
  double x,y;	//left bottom coordinates
  double rx, ry; //right top coordinates
  double area;
  int degree;
  bool is_terminal;
  vector<int> child;
};
typedef vector<Module*> Net;
typedef vector<Net> Nets;


class FPlan{
  public:
    FPlan(float alpha, float beta);
    void read(string fr);
    virtual void init()		=0;
    virtual void packing();
    virtual void perturb()	=0;
    virtual void keep_sol()	=0;
    virtual void keep_best()	=0;
    virtual void recover()	=0;
    virtual void recover_best() =0;
    virtual double getCost();

    int    size()         { return num_modules; }
    double getArea()      { return Area;      }
    double getTotalArea() { return TotalArea; }
    double   getWireLength(){ return WireLength;}
    double getWidth()     { return Width;     }
    double getHeight()    { return Height;    }

    // information
    void print_result();
    void show_modules();
    void normalize_cost(int);
    void print_modules();
    void print_nets();


  protected:
    void clear();
    double calcWireLength();
    void scaleIOPad();

    double Area;
    double Width,Height;
    double TotalArea;
    unsigned long long int WireLength;
    int num_modules;
    int num_terminals;
    int num_nets;
    int num_pins;
    vector<Module> modules;
    vector<Module> terminals;
    Module  root_module;
    Nets network;
    double norm_area, norm_WL;
    float cost_alpha;
    float cost_beta;
    vector<pair<int, int>> terminal_array;

  private:
    string filename_short;
    string filename;

};

bool rand_bool();
float rand_01();
double seconds();

#endif /* SRC_FP_H_ */
