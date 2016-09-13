/*
 * btree.h
 *
 *  Created on: Apr 20, 2015
 *      Author: yang
 *  Based on : B*-tree based placement/floorplanning, Jer-Ming Hsu,  Hsun-Cheng Lee
 *	
 */

#ifndef SRC_BTREE_H_
#define SRC_BTREE_H_

#include <vector>
#include <cassert>
#include "fp.h"
//#include <map>


using namespace std;

const int NIL = -1;
typedef bool DIR;

struct Node{
  int id,parent,left,right, m_id;
  bool rotate,flip;
  bool isleaf(){ return (left==NIL && right==NIL); }
};

/*
struct Point{
	double x;
	double y;
};
*/

struct Contour{
  int front,back;
};

class BTree : public FPlan{
  public:
    BTree(float alpha=1, float beta=0) :FPlan(alpha, beta) {}
    virtual void init();
    virtual void packing();
    virtual void perturb();
    virtual void keep_sol();
    virtual void keep_best();
    virtual void recover();
    virtual void recover_best();

    // debuging
    void testing();
    void fprint_tree();
    void show_tree();
    void print_modules();

    //map<Point, bool> terminal_position;

  protected:
    void place_module(int mod,int abut,bool is_left=true);
    void place_terminal();
    bool legal();
    void clear();

    // Auxilary function
    void wire_nodes(int parent,int child,DIR edge);
    int child(int node,DIR d);
    bool legal_tree(int p,int n,int &num);
    void add_changed_nodes(int n);

    // SA permutating operation
    void swap_node(Node &n1, Node &n2);
    void insert_node(Node &parent,Node &node);
    void delete_node(Node &node);

    bool delete_node2(Node &node,DIR pull);
    void insert_node2(Node &parent,Node &node,
                     DIR edge,DIR push,bool fold);

    int contour_root;//
    vector<Contour> contour;//
    int nodes_root;//
    vector<Node> nodes;  //


  private:
    struct Solution{
      int nodes_root;
      vector<Node> nodes;
      double cost;
      Solution() { cost = 1; }
      void clear() { cost = 1, nodes.clear();}
    };

    void get_solution(Solution &sol);
    void recover(Solution &sol);
    void recover_partial();
    bool terminal_violate(double mid_x, double mid_y, int id);

    Solution best_sol, last_sol;
    // for partial recover
    vector<Node> changed_nodes;
    int changed_root;
};

#endif /* SRC_BTREE_H_ */
