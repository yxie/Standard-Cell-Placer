#include "fp.h"
#include <fstream>
#include <cstdio>
#include <cstring>
#include <climits>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdlib.h>  //rand
#include <math.h>
#include <sstream>
#include <algorithm>

char line[100],t1[40],t2[40], t3[40];
unsigned int seed=0;
#define INF (0x7FFFFFFF)
#define verbose (0)

FPlan::FPlan(float alpha=1, float beta=0){
  norm_area= 1;
  norm_WL= 1;
  cost_alpha = alpha;
  cost_beta = beta;
}

void FPlan::packing(){
  if(cost_alpha!=1)
     calcWireLength();
}

void FPlan::clear(){
  Area = 0;
  WireLength = 0;
}


double FPlan::getCost(){
     return (cost_alpha)*(Area) + (cost_beta)*(WireLength);
}


void FPlan::normalize_cost(int t){
  norm_area=norm_WL=0;
  for(int i=0; i < t; i++){
	cout << "nomalize cost = " << i << endl;
    perturb();
    cout << "perturb" << endl;
    packing();
    cout << "packing" << endl;
    norm_area += Area;
    norm_WL += WireLength;
  }
  norm_area /= t;
  norm_WL /= t;
  cout << "Normalized area = " << norm_area << ", normalized wirelength = " << norm_WL << endl;
}

bool compare(const pair<int,int>&i, const pair<int,int>&j)
{
    return i.second > j.second;
}

void FPlan::read(string fr){
	filename_short = fr;
	filename = "./in/" +  fr + ".blocks";
	ifstream frblock;
	frblock.open(filename.c_str());

	string s1,s2;
	int num_softblock;
	frblock >> s1 >> s2 >> num_softblock;
	frblock >> s1 >> s2 >> num_modules;
	frblock >> s1 >> s2 >> num_terminals;

	for(int i=0; i<num_modules; i++){
		int x1,y1,x2,y2,x3,y3,x4,y4;
		string blockname, s1, s2;
		char l, d, r;
		frblock >> blockname >> s1 >> s2;
		frblock >> l >> x1 >> d >> y1 >> r;
		frblock >> l >> x2 >> d >> y2 >> r;
		frblock >> l >> x3 >> d >> y3 >> r;
		frblock >> l >> x4 >> d >> y4 >> r;
		Module m;
		m.name = blockname;
		m.id = i;
		m.x = x1;
		m.y = y1;
		m.rx = x1 + m.width;
		m.ry = y1 + m.height;
		m.width = x3 - x1;
		m.height = y3 - y1;
		m.area = (x3 - x1) * (y3 - y1);
		m.is_terminal = 0;
		modules.push_back(m);
	}
	for(int i=0; i<num_terminals; i++){
		string terminalname;
		frblock >> terminalname >> s1;
		Module t;
		t.id = i;
		t.name = terminalname;
		t.x = i*2;
		t.y = 0;
		t.rx = i*2;
		t.ry = 0;
		t.width = 0;
		t.height = 0;
		t.degree = 0;
		t.is_terminal = 1;
		terminals.push_back(t);
	}
	frblock.close();

	filename = "./in/" +  fr + ".nets";
	ifstream frnet;
	frnet.open(filename.c_str());
	frnet >> s1 >> s2 >> num_nets;
	frnet >> s2 >> s2 >> num_pins;
	for(int k=0; k<num_nets; k++){
		int pins_in_net;
		string name, B;
		Net n;
		frnet >> s1 >> s2 >> pins_in_net;
		for(int i=0; i<pins_in_net; i++){
			frnet >> name >> B;
			for(int j=0; j<num_modules; j++){
				if(name.compare(modules[j].name) == 0){
					n.push_back(&modules[j]);
					break;
				}
			}
			for(int j=0; j<num_terminals; j++){
				if(name.compare(terminals[j].name) == 0){
					n.push_back(&terminals[j]);
					break;
				}
			}
		}
		network.push_back(n);
	}

	//find terminals' children
	for(int i=0; i<num_nets; i++){
		for(int j=0; j<network[i].size(); j++){
			if(network[i][j]->is_terminal == 1){
				for(int k=0; k<network[i].size(); k++){
					if(network[i][k]->is_terminal == 0){
						network[i][j]->child.push_back(network[i][k]->id);
					}
				}
			}
		}
	}

	//sort terminals by degrees
	for(int i=0; i<num_terminals; i++){
		terminals[i].degree = terminals[i].child.size();
		pair <int,int> t_pair;
		t_pair = make_pair(i, terminals[i].degree);
		terminal_array.push_back(t_pair);
	}
	sort(terminal_array.begin(), terminal_array.end(), compare);
/*
	for(int i=0; i<num_terminals; i++){
		cout << terminals[i].name << ": ";
		for(int j=0; j<terminals[i].child.size(); j++){
			cout << modules[terminals[i].child[j]].name << " " ;
		}
		cout << endl;
	}
	getchar();

	for(int i=0; i<num_terminals; i++){
		cout << terminals[terminal_array[i].first].name << " " << terminal_array[i].second << endl;
	}
	getchar();
*/
}




double FPlan::calcWireLength(){
  WireLength = 0;

  for(int i=0; i < network.size(); i++){
    double max_x= INT_MIN, max_y= INT_MIN;
    double min_x= INT_MAX, min_y= INT_MAX;
    assert(network[i].size() > 0);
    for(int j=0; j < network[i].size(); j++){
      Module p = *network[i][j];
      max_x= max(max_x, p.x+p.width/2);
      max_y= max(max_y, p.y+p.height/2);
      min_x= min(min_x, p.x+p.width/2);
      min_y= min(min_y, p.y+p.height/2);
    }
    WireLength += (max_x-min_x)+(max_y-min_y);
  }

  return WireLength;
}

void FPlan::print_modules(){
	cout << "# modules: " << num_modules << endl;
	for(int i=0; i<num_modules; i++){
		cout << "module id " << modules[i].id << ": " << modules[i].name << " ";
		cout << modules[i].x << " " << modules[i].y << " " ;
		cout << modules[i].width << " " << modules[i].height << endl;
	}
	for(int i=0; i<num_terminals; i++){
		cout << "terminal id " << terminals[i].id << ": " << terminals[i].name << " ";
		cout << terminals[i].x << " " << terminals[i].y << endl;
	}
}

void FPlan::print_nets(){
	for(int i=0; i<num_nets; i++){
		for(int j=0; j<network[i].size(); j++){
			cout << network[i][j]->name << " ";
		}
		cout << endl;
	}
}

void FPlan::print_result(){
	ofstream frresult;
	ofstream frplace;
	filename = "./out/" + filename_short + ".result";
	frresult.open(filename.c_str());
	filename = "./out/" + filename_short + ".pl";
	frplace.open(filename.c_str());

	calcWireLength();

	cout << filename_short << ":" << endl;
	cout << "#modules = " << num_modules << endl;
	cout << "#nets = " << num_nets << endl;
	cout << "Total wirelength = " << getWireLength() << endl;
	cout << "Total area = " << getArea() << endl;
	cout << "Height = " << Height << ", Width = " << Width << endl;

	frresult << filename_short << ":" << endl;
	frresult << "#modules = " << num_modules << endl;
	frresult << "#nets = " << num_nets << endl;
	frresult << "Total wirelength = " << getWireLength() << endl;
	frresult << "Total area = " << getArea() << endl;
	frresult << "Height = " << Height << ", Width = " << Width << endl;

	for(int i=0; i<num_modules; i++){
		frplace << modules[i].name << " " << modules[i].x << " " << modules[i].y << endl;
	}
	frplace << endl;
	for(int i=0; i<num_terminals; i++){
		frplace << terminals[i].name << " " <<terminals[i].x << " " << terminals[i].y << endl;
	}

	frresult.close();
	frplace.close();
}

float rand_01(){
  return float(rand()%10000)/10000;
}

bool rand_bool(){
  return bool(rand()%2);
}
