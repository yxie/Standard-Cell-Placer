#include <stack>
#include <algorithm>
#include <iostream>
#include <limits.h>
#include "btree.h"


float move_rate = 0.5;
float swap_rate = 0.5;

void BTree::clear(){
  // initial contour value
  contour_root = NIL;
  FPlan::clear();
}


void BTree::init(){
  // initialize contour structure
  contour.resize(num_modules);

  // initialize b*tree by complete binary tree
  nodes.resize(num_modules);

  nodes_root = 0;

  //left partition
  for(int i=0; i < num_modules; i++){
    nodes[i].id = i;
    nodes[i].m_id = modules[i].id;
    if(i==0)
    	nodes[i].parent = NIL;
    else
    	nodes[i].parent = (i+1)/2-1;
    if( (2*i+1) < num_modules)
    	nodes[i].left   = 2*i+1;
    else
    	nodes[i].left = NIL;
    if( (2*i+2) < num_modules)
    	nodes[i].right   = 2*i+2;
    else
    	nodes[i].right = NIL;
  }

  best_sol.clear();
  last_sol.clear();
  clear();
  normalize_cost(0);
}


void BTree::packing(){
  stack<int> S, S2;

  clear();

  int p = nodes_root;
  place_module(p,NIL);
  Node &n = nodes[p];
  if(n.right != NIL)
	  S.push(n.right);
  if(n.left  != NIL)
	  S.push(n.left);

  // inorder traverse
  while(!S.empty()){
    p = S.top();
    S.pop();
    Node &n = nodes[p];

    assert(n.parent != NIL);
    bool is_left = (nodes[n.parent].left == n.id);
    place_module(p, n.parent, is_left); //side, node_id, pnode_id, is_left

    if(n.right != NIL)
    	S.push(n.right);
    if(n.left  != NIL)
    	S.push(n.left);
  }

  // compute Width, Height
  double max_x=-1,max_y=-1;
  for(int p= contour_root; p != NIL; p=contour[p].front){
    max_x = max(max_x,double(modules[p].rx));
    max_y = max(max_y,double(modules[p].ry));
  }

  Width   = max_x;
  Height  = max_y;
  Area         = Height*Width;
  place_terminal();
  FPlan::packing(); 	// calculate wire length

}

void BTree::place_module(int node_id, int abut, bool is_left){
  int mod, mod_abut;

  mod = nodes[node_id].m_id;
  mod_abut = nodes[abut].m_id;
  Module &mod_mf = modules[mod];

  int w =  modules[mod].width;
  int h =  modules[mod].height;

  if(abut==NIL){	// root node
    contour_root = mod;
    contour[mod].back = NIL;
    contour[mod].front = NIL;
    mod_mf.x  = mod_mf.y = 0;
    mod_mf.rx = mod_mf.x + w;
    mod_mf.ry = mod_mf.y + h;
    return;
  }

  int p;   // trace contour from p

  if(is_left){	// left
    int abut_width = modules[mod_abut].width;
    mod_mf.x  = modules[mod_abut].x + abut_width;
    mod_mf.rx = mod_mf.x + w;
    p = contour[mod_abut].front;

    contour[mod_abut].front = mod;
    contour[mod].back = mod_abut;

    if(p==NIL){  // no obstacle in X axis
      mod_mf.y = 0;
      mod_mf.ry = h;
      contour[mod].front = NIL;
      return;
    }
  }
  else{	// upper
    mod_mf.x = modules[mod_abut].x;
    mod_mf.rx = mod_mf.x + w;
    p = mod_abut;

    int n=contour[mod_abut].back;

    if(n==NIL){ // i.e, mod_mf.x==0
      contour_root = mod;
      contour[mod].back = NIL;
    }
    else{
      contour[n].front = mod;
      contour[mod].back = n;
    }
  }

  int min_y = INT_MIN;
  int bx,by;
  assert(p!=NIL); //p = contour[mod_abut].front; or p = mod_abut;

  for(; p!=NIL ; p=contour[p].front)
  {
    bx = modules[p].rx;
    by = modules[p].ry;
    min_y = max(min_y, by);

    if(bx >= mod_mf.rx){ 	// update contour
      mod_mf.y = min_y;
      mod_mf.ry = mod_mf.y + h;
      if(bx > mod_mf.rx){
        contour[mod].front = p;
        contour[p].back = mod;
      }
      else{ 			// bx==mod_mf.rx, delete p
        int n= contour[p].front;
        contour[mod].front = n;
        if(n!=NIL)
          contour[n].back = mod;
      }
      break;
    }
  }


  if(p==NIL){  // max bx < mod_mf.rx
    mod_mf.y  = (min_y==INT_MIN? 0 : min_y);
    mod_mf.ry = mod_mf.y + h;
    contour[mod].front = NIL;
  }

}

void BTree::get_solution(Solution &sol){
  sol.nodes_root = nodes_root;
  sol.nodes = nodes;
  sol.cost = getCost();
}

void BTree::keep_sol(){
  get_solution(last_sol);
}

void BTree::keep_best(){
  get_solution(best_sol);
}

void BTree::recover(){
  recover(last_sol);
  // recover_partial();
}

void BTree::recover_best(){
  recover(best_sol);
}

void BTree::recover(Solution &sol){
  nodes_root = sol.nodes_root;
  nodes = sol.nodes;
}


void BTree::perturb(){
  int p,n;

  n = rand()%num_modules;

  if(swap_rate >rand_01()){
      int temp=0;
      do{
    	  p = rand()%num_modules;
    	  temp++;
      }while(temp<100 && (n==p||nodes[n].parent==p||nodes[p].parent==n) );
      if(temp<100)
    	  swap_node(nodes[p],nodes[n]);
  }
  else{
	  do{
		  p = rand()%num_modules;
	  }while(n==p);
	  delete_node(nodes[n]);
	  insert_node(nodes[p],nodes[n]);
  }
}


void BTree::swap_node(Node &n1, Node &n2){
  //change parent of n1 and n2's children
  if(n1.left!=NIL){
    nodes[n1.left].parent  = n2.id;
  }
  if(n1.right!=NIL){
    nodes[n1.right].parent = n2.id;
  }
  if(n2.left!=NIL){
    nodes[n2.left].parent  = n1.id;
  }
  if(n2.right!=NIL){
    nodes[n2.right].parent = n1.id;
  }

  //change children of n1 and n2's parent
  if(n1.parent != NIL){
    if(nodes[n1.parent].left==n1.id)
       nodes[n1.parent].left  = n2.id;
    else
       nodes[n1.parent].right = n2.id;
  }
  else{
    changed_root = n1.id;  //?
    nodes_root = n2.id;
  }

  if(n2.parent != NIL){
    if(nodes[n2.parent].left==n2.id)
       nodes[n2.parent].left  = n1.id;
    else
       nodes[n2.parent].right = n1.id;
  }
  else{
    nodes_root = n1.id;
  }

  //change parent and children of n1 and n2
  swap(n1.left,n2.left);
  swap(n1.right,n2.right);
  swap(n1.parent,n2.parent);
}

void BTree::insert_node(Node &parent, Node &node){
  node.parent = parent.id;
  bool edge = rand_bool();

  if(edge){	//add node to parent's left edge
    node.left  = parent.left;
    node.right = NIL;
    if(parent.left!=NIL)
      nodes[parent.left].parent = node.id;
    parent.left = node.id;
  }
  else{ // add node to parent's right edge
    node.left  = NIL;
    node.right = parent.right;
    if(parent.right!=NIL)
      nodes[parent.right].parent = node.id;
    parent.right = node.id;
  }
}


void BTree::delete_node(Node &node){
  int child    = NIL;	// pull which child
  int subchild = NIL;   // child's subtree
  int subparent= NIL;

  if(!node.isleaf()){
    bool left= rand_bool();			// choose a child to pull up
    if(node.left ==NIL) left=false;
    if(node.right==NIL) left=true;

    if(left){
      child = node.left;			// child will never be NIL
      if(node.right!=NIL){
        subchild  = nodes[child].right;
        subparent = node.right;
        nodes[node.right].parent = child;
        nodes[child].right = node.right;	// abut with node's another child
      }
    }
    else{
      child = node.right;
      if(node.left!=NIL){
        subchild  = nodes[child].left;
        subparent = node.left;
        nodes[node.left].parent = child;
        nodes[child].left = node.left;
      }
    }
    nodes[child].parent = node.parent;
  }

  if(node.parent == NIL){			// root
    nodes_root = child;
  }
  else{					// let parent connect to child
    if(node.id == nodes[node.parent].left)
      nodes[node.parent].left  = child;
    else
      nodes[node.parent].right = child;
  }

  // place subtree
  if(subchild != NIL){
    Node &sc = nodes[subchild];
    assert(subparent != NIL);

    while(1){
      Node &p = nodes[subparent];
      if(p.left==NIL || p.right==NIL){
    	  sc.parent = p.id;
    	  if(p.left==NIL)
    		  p.left = sc.id;
    	  else
    		  p.right = sc.id;
    	  break;
      }
      else{
    	  subparent = (rand_bool() ? p.left : p.right);
      }
    }
  }
}

void BTree::show_tree(){
  cout << "root: " << modules[nodes[nodes_root].m_id].name << endl;
  for(int i=0; i < num_modules; i++){
    cout << modules[nodes[i].m_id].name << ": ";
    if(nodes[i].left!=NIL)
    	cout << "left " << modules[nodes[nodes[i].left].m_id].name << ", ";
    if(nodes[i].right!=NIL)
    	cout << "right " << modules[nodes[nodes[i].right].m_id].name << ", ";
    cout << endl;
  }
}

void BTree::print_modules(){
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

void BTree::place_terminal(){
	for(int i=0; i<num_terminals; i++){
		//double best_x, best_y;
		double mid_x, mid_y;
		double max_x= INT_MIN, max_y= INT_MIN;
		double min_x= INT_MAX, min_y= INT_MAX;
		int t_id = terminal_array[i].first;
		for(int j=0; j<terminals[t_id].child.size(); j++){
			int v = terminals[t_id].child[j];
			max_x= max(max_x, modules[v].x+modules[v].width/2);
			max_y= max(max_y, modules[v].y+modules[v].height/2);
			min_x= min(min_x, modules[v].x+modules[v].width/2);
			min_y= min(min_y, modules[v].y+modules[v].height/2);
		}
		mid_x = (max_x + min_x) / 2;
		mid_y = (max_y + min_y) / 2;
		int d1 = min_x;
		int d2 = Width - max_x;
		int d3 = min_y;
		int d4 = Height - max_y;
		if(d1 <= d2 && d1 <= d3 && d1 <= d4){
			terminals[t_id].x = terminals[t_id].rx = 0;
			terminals[t_id].y = terminals[t_id].ry = mid_y;
		}
		else if(d2 <= d1 && d2 <= d3 && d2 <= d4){
			terminals[t_id].x = terminals[t_id].rx = Width;
			terminals[t_id].y = terminals[t_id].ry =mid_y;
		}
		else if(d3 <= d1 && d3 <= d2 && d3 <= d4){
			terminals[t_id].x = terminals[t_id].rx = mid_x;
			terminals[t_id].y = terminals[t_id].ry = 0;
		}
		else if(d4 <= d1 && d4 <= d2 && d4 <= d3){
			terminals[t_id].x = terminals[t_id].rx = mid_x;
			terminals[t_id].y = terminals[t_id].ry = Height;
		}
	}

	//check violation
	for(int i=0; i<num_terminals; i++){
		while(terminal_violate(terminals[i].x, terminals[i].y, i) == 1){
			if( terminals[i].y == 0 && terminals[i].x > 0){
				if(terminals[i].x - 1 < 0){
					terminals[i].x = 0;
					terminals[i].rx = 0;
				}
				else{
					terminals[i].x--;
					terminals[i].rx--;
				}
			}
			else if(terminals[i].x == 0 && terminals[i].y < Height){
				if(terminals[i].y + 1 > Height){
					terminals[i].y = Height;
					terminals[i].ry = Height;
				}
				else{
					terminals[i].y++;
					terminals[i].ry++;
				}
			}
			else if(terminals[i].y == Height && terminals[i].x < Width){
				if(terminals[i].x + 1 > Width){
					terminals[i].x = Width;
					terminals[i].rx = Width;
				}
				else{
					terminals[i].x++;
					terminals[i].rx++;
				}
			}
			else if(terminals[i].x == Width && terminals[i].y > 0){
				if(terminals[i].y - 1 < 0){
					terminals[i].y = 0;
					terminals[i].ry = 0;
				}
				else{
					terminals[i].y--;
					terminals[i].ry--;
				}
			}
			else{
				cout << terminals[i].x << " " << terminals[i].y << " " << Width << " " << Height << endl;
				cout << "error in locating terminals " << endl;
				getchar();
			}
		}
	}
}

bool BTree::terminal_violate(double mid_x, double mid_y, int id){
	bool violate = 0;
	for(int i=0; i<num_terminals; i++){
		if(i == id)
			continue;
		if(abs(mid_x - terminals[i].x) + abs(mid_y - terminals[i].y) < 2 ){
			violate = 1;
			break;
		}
	}
	return violate;
}
