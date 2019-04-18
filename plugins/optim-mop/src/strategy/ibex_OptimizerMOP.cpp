//                                  I B E X
// File        : ibex_Optimizer.cpp
// Author      : Gilles Chabert, Bertrand Neveu
// Copyright   : Ecole des Mines de Nantes (France)
// License     : See the LICENSE file
// Created     : May 14, 2012
// Last Update : December 24, 2012
//============================================================================

#include "ibex_OptimizerMOP.h"
#include "ibex_Timer.h"
#include "ibex_Function.h"
#include "ibex_NoBisectableVariableException.h"
#include <float.h>
#include <stdlib.h>
#include <iomanip>
#include <iostream>
#include <set>
//#include "ibex_CellSet.h"

#ifndef cdata
#define cdata ((BxpMOPData*) c->prop[BxpMOPData::id])
#endif

using namespace std;

namespace ibex {

const double OptimizerMOP::default_eps=0.01;

bool OptimizerMOP::_plot = false;
double OptimizerMOP::_min_ub_dist = 1e-7;
bool OptimizerMOP::_cy_upper =false;
bool OptimizerMOP::cy_contract_var = false;
bool OptimizerMOP::_eps_contract = false;
double OptimizerMOP::_rh = 0.1;
bool OptimizerMOP::_server_mode=false;
string OptimizerMOP::instructions_file="";
string OptimizerMOP::output_file="";



OptimizerMOP::OptimizerMOP(int n, const Function &f1,  const Function &f2,
		Ctc& ctc, Bsc& bsc, CellBufferOptim& buffer, LoupFinderMOP& finder,
		Mode nds_mode, Mode split_mode, double eps, double rel_eps) : n(n),
                				ctc(ctc), bsc(bsc), buffer(buffer), goal1(f1), goal2(f2),
								finder(finder), trace(false), timeout(-1), status(SUCCESS),
                				time(0), nb_cells(0), eps(eps), nds_mode(nds_mode), split_mode(split_mode),
												rel_eps(rel_eps) {

	if (trace) cout.precision(12);
}


OptimizerMOP::~OptimizerMOP() {

}


Interval OptimizerMOP::eval_goal(const Function& goal, const IntervalVector& x, int n){
	//the objectives are set to 0.0
	IntervalVector xz(x);
	xz.resize(n+2);

	xz[n]=0.0;
	xz[n+1]=0.0;
	return goal.eval(xz);
}

IntervalVector OptimizerMOP::deriv_goal(const Function& goal, const IntervalVector& x, int n){
	//the objectives are set to 0.0
	IntervalVector xz(x);
	xz.resize(n+2);

	xz[n]=0.0;
	xz[n+1]=0.0;
	IntervalVector g(goal.gradient(xz));
	g.resize(n);
	return g;
}



bool OptimizerMOP::update_NDS2(const IntervalVector& box) {

	//We attempt to find two feasible points which minimize both objectives
	//and the middle point between them
	IntervalVector box2(box); box2.resize(n);
	IntervalVector xa(n), xb(n);
	finder.clear();

	list< pair <double, double> > points;
	list< pair< pair< double, double> , pair< double, double> > > segments;

	Vector mid=box2.mid();
	if (finder.norm_sys.is_inner(mid)){
		ndsH.addPoint(make_pair(eval_goal(goal1,mid,n).ub(), eval_goal(goal2,mid,n).ub()));
	}

  if(nds_mode==POINTS) {
		try{
		while(true){
			xa = finder.find(box2,box2,POS_INFINITY).first;
			ndsH.addPoint(make_pair(eval_goal(goal1,xa,n).ub(), eval_goal(goal2,xa,n).ub()));
		}
		}catch (LoupFinder::NotFound& ) {
			return true;
		}
	}

	try{
		xa = finder.find(box2,box2,POS_INFINITY).first;

	}catch (LoupFinder::NotFound& ) {
		return false;
	}

	try{
		xb = finder.find(box2,box2,POS_INFINITY).first;
	}catch (LoupFinder::NotFound& ) {
		ndsH.addPoint(make_pair(eval_goal(goal1,xa,n).ub(), eval_goal(goal2,xa,n).ub()));
		return true;
	}

	hamburger(xa, xb);

	return true;

}

void OptimizerMOP::discard_generalized_monotonicty_test(IntervalVector& box, const IntervalVector& initbox){
	IntervalVector grad_f1= goal1.gradient(box);
	IntervalVector grad_f2= goal2.gradient(box);

	IntervalVector new_box(box);

	//bool discard=false;

	for(int i=0;i<n;i++){
		for(int j=0;j<n;j++){
			if(grad_f1[i].lb() > 0.0 && grad_f2[j].lb()>0.0 &&
				    (i==j || (Interval(grad_f2[i].lb()) / grad_f2[j].lb() -  Interval(grad_f1[i].lb()) / grad_f1[j].lb()).lb()  > 0.0) ){

				    if(i==j) new_box[i] = box[i].lb(); //simple monotonicity test

					if( box[i].lb() != initbox[i].lb() && box[j].lb() != initbox[j].lb() ){
						if(is_inner_facet(box,i,box[i].lb()) && is_inner_facet(box,j,box[j].lb())){
							box.set_empty();
							return;
						}
					}
			}else if(grad_f1[i].ub() < 0.0 && grad_f2[j].lb()>0.0 &&
					(Interval(grad_f1[i].ub()) / grad_f1[j].lb() -  Interval(grad_f2[i].ub()) / grad_f2[j].lb()).lb()  > 0.0) {

				    if( box[i].ub() == initbox[i].ub() && box[j].lb() != initbox[j].lb() ) {
						if(is_inner_facet(box,i,box[i].ub()) && is_inner_facet(box,j,box[j].lb())){
							box.set_empty();
							return;
						}
					}

			}else if(grad_f1[i].lb() > 0.0 && grad_f2[j].ub()<0.0 &&
					(Interval(grad_f1[i].lb()) / grad_f1[j].ub() -  Interval(grad_f2[i].lb()) / grad_f2[j].ub()).lb()  > 0.0) {

				    if( box[i].lb() != initbox[i].lb() && box[j].ub() != initbox[j].ub() )
				    {
						if(is_inner_facet(box,i,box[i].lb()) && is_inner_facet(box,j,box[j].ub())){
							box.set_empty();
							return;
						}
					}

			 }else if(grad_f1[i].ub() < 0.0 && grad_f2[j].ub() < 0.0 &&
					(i==j || (Interval(grad_f2[i].ub()) / grad_f2[j].ub() -  Interval(grad_f1[i].ub()) / grad_f1[j].ub()).lb()  > 0.0) ){

					if(i==j) new_box[i] = box[i].ub(); //simple monotonicity test

					if( box[i].ub() != initbox[i].ub() && box[j].ub() != initbox[j].ub() )
					{
						if(is_inner_facet(box,i,box[i].ub()) && is_inner_facet(box,j,box[j].ub())){
							box.set_empty();
							return;
						}
					}
			}
		}
	}

	IntervalVector bb=new_box;
	bb.resize(n);

	if(finder.norm_sys.is_inner(bb))
		box=new_box;

}

void OptimizerMOP::cy_contract2(Cell& c, list <pair <double,double> >& inpoints){
	IntervalVector& box = c.box;
	IntervalVector box3(box);
	box3.resize(n+4);
	box3[n+3] = 1.0;

	pair <double, double> firstp=inpoints.front();
	pair <double, double> lastp=inpoints.back();

	//Pent of cy

	if(firstp!=lastp)
		box3[n+3] = (lastp.first-firstp.first)/(firstp.second-lastp.second);
	if(box3[n+3]==0.0 || box3[n+3]==1.0 || box3[n+3].is_empty())
		box3[n+3] = box[n].diam()/box[n+1].diam(); // a

   //cout <<box3[n+3]  << endl;

   //setting w_ub with the NDS points
	 double w_ub=POS_INFINITY;

   if(_cy_upper){
			w_ub=NEG_INFINITY;
			for(auto pmax:inpoints){
				  if(pmax.first == POS_INFINITY || pmax.second == POS_INFINITY) {
						w_ub=POS_INFINITY;
						break;
					}

					double ww;
					if(_eps_contract)
						ww = ( Interval(pmax.first)-eps + box3[n+3]*(Interval(pmax.second)-eps) ).ub();
					else
						ww = ( Interval(pmax.first) + box3[n+3]*(Interval(pmax.second)) ).ub();
				  if(w_ub < ww )  w_ub = ww;
			}
	 }

	box3[n+2] = Interval(NEG_INFINITY, w_ub); // w
   // cout << 	box3[n+2] <<endl;
	//the contraction is performed
	ctc.contract(box3);
	((BxpMOPData*) c.prop[BxpMOPData::id])->a = box3[n+3].mid();
	((BxpMOPData*) c.prop[BxpMOPData::id])->w_lb = box3[n+2].lb();

	box=box3;
	box.resize(n+2);
}

void OptimizerMOP::dominance_peeler2(IntervalVector& box, list <pair <double,double> >& inpoints){
	/*=================Dominance peeler for NDS2 and cy =========*/

	pair <double, double> firstp=inpoints.front();
	pair <double, double> lastp=inpoints.back();

	// contract c.box[n]
	if(firstp.second < box[n+1].ub())
		box[n+1] = Interval(box[n+1].lb(),firstp.second);

	// contract c.box[n+1]
	if(lastp.first < box[n].ub() )
		box[n] = Interval(box[n].lb(), lastp.first);

}


void OptimizerMOP::contract_and_bound(Cell& c, const IntervalVector& init_box) {

	list<pair <double,double> > inner_segments = ndsH.non_dominated_points(c.box[n].lb(), c.box[n+1].lb());

	dominance_peeler2(c.box,inner_segments);

	//discard_generalized_monotonicty_test(c.box, init_box);

	if (c.box.is_empty()) return;

	if(cy_contract_var)
			cy_contract2(c,inner_segments);
	else
			ctc.contract(c.box);


	if (c.box.is_empty()) return;



}

OptimizerMOP::Status OptimizerMOP::optimize(const IntervalVector& init_box) {

	status=SUCCESS;

	nb_cells=0;
	buffer.flush();
	ndsH.clear();

	//the box in cells have the n original variables plus the two objective variables (y1 and y2)
	Cell* root=new Cell(IntervalVector(n+2));

	root->box=init_box;

	root->prop.add(new BxpMOPData());

	// add data required by the cell buffer
	buffer.add_property(init_box, root->prop);

	// add data required by the bisector
	bsc.add_property(init_box, root->prop);

	// add data required by the contractor
	ctc.add_property(init_box, root->prop);

	BxpMOPData::y1_init=eval_goal(goal1, root->box, n);
	BxpMOPData::y2_init=eval_goal(goal2, root->box, n);

	cout << BxpMOPData::y1_init << endl;
	cout << BxpMOPData::y2_init << endl;

	y1_ub.first=POS_INFINITY;
	y2_ub.second=POS_INFINITY;


	list<IntervalVector> B; //list of discarded boxes Y

	time=0;
	Timer timer;
	timer.start();


	set<Cell*> cells;
	set<Cell*> paused_cells;
	IntervalVector focus(2);
	focus[0]=BxpMOPData::y1_init;
	focus[1]=BxpMOPData::y2_init;

	buffer.push(root);
	cells.insert(root);

	int iter=0;
	try {
		bool server_pause=false;
		/** Criterio de termino: todas los nodos filtrados*/
		while (!buffer.empty() || !paused_cells.empty()) { 		  iter++;
			if(buffer.empty() && !_server_mode) break;

			if( _server_mode && iter%10==0 ) server_pause=true;

			while(buffer.empty() || server_pause){
				if(server_pause) {
			    	cout << "buffer size:" << buffer.size() << endl;
			    	cout << "eps:" << eps << endl;
					write_envelope(cells, paused_cells, focus);
				}
				sleep(2);
				read_instructions(cells, paused_cells, focus);
				server_pause=false;
			}
			if(rel_eps>0.0)	eps=std::max(focus[0].diam(),focus[1].diam())*rel_eps;

			Cell *c = buffer.pop();
			cells.erase(c);


			if(_server_mode){
				IntervalVector boxy(2); boxy[0]=c->box[n]; boxy[1]=c->box[n+1];
				if(!focus.intersects(boxy) ){
					paused_cells.insert(c);
					continue;
				}
			}



			if(cdata->ub_distance <= eps){
				 if(_server_mode){
					 while(!buffer.empty())
					   paused_cells.insert(buffer.pop());
					 cells.clear();
					 continue;
				 }
				 break;
			}

			nb_cells++;
			contract_and_bound(*c, init_box);

			if (c->box.is_empty()) {
				delete c;
				continue;
			}

			update_NDS2(c->box);

			y1_ub.first = ndsH.lb().first;
			y2_ub.second= ndsH.lb().second;

			pair<Cell*,Cell*> new_cells;
			bool atomic_box=false;
			try {
				new_cells=pair<Cell*,Cell*>(bsc.bisect(*c));
			}
			catch (NoBisectableVariableException& ) {
				atomic_box=true;
			}

    	double dist=0.0;
    	if(!atomic_box) dist=ndsH.distance(c);

			//se elimina la caja
    	if(dist < eps || atomic_box){

    		if(new_cells.first){
    			delete new_cells.first;
    			delete new_cells.second;
    		}

				if(_server_mode && dist>0.0){
					paused_cells.insert(c);
				}else delete c;

				continue;
		}

      /** Improvement for avoiding big boxes when lb1 < y1_ub or lb2< y2_ub*/
		  if(c->box[n].lb() < y1_ub.first && c->box[n].ub() > y1_ub.first &&
				  (c->box[n].ub()-y1_ub.first)*(c->box[n+1].ub()-y1_ub.second) <  (c->box[n].diam())*(c->box[n+1].diam()) ){
       		BisectionPoint p(n, y1_ub.first, false);
			new_cells=c->bisect(p);

		  }else if(c->box[n+1].lb() < y2_ub.second && c->box[n+1].ub() > y2_ub.second  &&
				(c->box[n].ub()-y2_ub.first)*(c->box[n+1].ub()-y2_ub.second) <  (c->box[n].diam())*(c->box[n+1].diam()) ) {
			BisectionPoint p(n+1, y2_ub.second, false);
			new_cells=c->bisect(p);
		 }
      /****/

			delete c; // deletes the cell.

			buffer.push(new_cells.first);
			cells.insert(new_cells.first);

			buffer.push(new_cells.second);
			cells.insert(new_cells.second);


			if (timeout>0) timer.check(timeout); // TODO: not reentrant, JN: done
			time = timer.get_time();


		}
	}
	catch (TimeOutException& ) {
		status = TIME_OUT;
		cout << "timeout" << endl;
	}


	timer.stop();
	time = timer.get_time();

	if(!_server_mode)
	   py_Plotter::offline_plot(ndsH.NDS2, NULL, "output2.txt");
	return status;
}


  // https://docs.google.com/document/d/1oXQhagd1dgZvkbPs34B4Nvye_GqA8lFxGZNQxqYwEgo/edit
void OptimizerMOP::hamburger(const IntervalVector& aIV, const IntervalVector& bIV) {

	IntervalVector xa=aIV;
	IntervalVector xb=bIV;
	double dist0=POS_INFINITY;

	PFunction pf(goal1, goal2, xa, xb);

	Node_t n_init (Interval(0,1), POS_INFINITY);
	std::priority_queue<Node_t, vector<Node_t> > n;
	if(process_node(pf, n_init)) {
		dist0=n_init.dist;
		n.push(n_init);
	}
	int count=1;

	while(n.size() > 0) {
		Node_t nt = n.top();
		n.pop();
       // cout << nt.dist << endl;
		if(nt.dist < eps || nt.dist < _rh*dist0 ) continue;
		//if(nt.dist < eps || nt.t.diam() < _rh ) continue;

		double pold=nt.t.lb();
		for(auto p:nt.b){
			Node_t n1( Interval(pold, p), nt.dist);
			if(process_node(pf, n1)) n.push(n1);
			count++;
			pold=p;
		}
		Node_t n2( Interval(pold, nt.t.ub()), nt.dist);
		if(process_node(pf, n2)) n.push(n2);
		count++;

		if(_plot) {
			py_Plotter::offline_plot(ndsH.NDS2, NULL, "output2.txt");
			//py_Plotter::offline_plot(NULL, LB.NDS2);
			//getchar();
		}
	}
	//cout << count << endl;

}

bool OptimizerMOP::process_node(PFunction& pf, Node_t& n_t) {
    // https://docs.google.com/document/d/1oXQhagd1dgZvkbPs34B4Nvye_GqA8lFxGZNQxqYwEgo/edit

	Interval t=n_t.t;

  if(!t.is_bisectable())
		 return false;

	// get extreme points
	IntervalVector ft_lb = pf.get_point(t.lb());
	IntervalVector ft_ub = pf.get_point(t.ub());
	Interval ya1=ft_lb[0];
	Interval ya2=ft_lb[1];
	Interval yb1=ft_ub[0];
	Interval yb2=ft_ub[1];

	// add extreme points
	ndsH.addPoint(ft_lb);
	ndsH.addPoint(ft_ub);

	if(nds_mode==POINTS) return false;

	//TODO: what if the extremal points are TOO dominated


    //too-close points
	if( fabs(ya1.ub() - yb1.ub()) < eps && fabs(ya2.ub() - yb2.ub()) < eps )
		return false;


	if(ya1.ub() > yb1.ub() || ya2.ub() < yb2.ub()) {
		Interval aux = ya1;
		ya1 = yb1; yb1 = aux; aux = ya2; ya2 = yb2; yb2 = aux;
	}

	// m ��� getSlope(n.t)
	Interval m = (yb2-ya2)/(yb1-ya1);
	Interval m_horizontal = Interval(0);
	Interval m_vertical = Interval(POS_INFINITY);

	// get minf1 and minf2
	pair<double, double> c1_t1 = make_pair(POS_INFINITY,0);
	pair<double, double> c2_t2 = make_pair(POS_INFINITY,0);

	if(nds_mode==HAMBURGER){
		//minimize f1
		c1_t1 = pf.optimize(m_vertical, PFunction::MIN, PFunction::F1, POS_INFINITY,  t);
		//minimize f2
		c2_t2 = pf.optimize(m_horizontal, PFunction::MIN, PFunction::F2, POS_INFINITY, t);
	}

	// check if the point dominating the curve is dominated by ndsH
	if(ndsH.is_dominated(pair<double,double>(c1_t1.first, c2_t2.first)))
			return false;

	// set bisection of inter
	set< pair<double, double> > v;



	if(m.ub() < 0) {
		pair<double, double> c3_t3 = pf.optimize(m, PFunction::MAX, PFunction::F2_mF1, POS_INFINITY, t);
		pair<double, double> c4_t4;
		if(nds_mode==HAMBURGER)
			 c4_t4 = pf.optimize(m, PFunction::MIN, PFunction::F2_mF1, POS_INFINITY, t);


		//agregar este segmento mejoro el conjunto Y'?
		//cout << "add:(" << ((ya2-c3_t3.first)/m).ub() << "," << ya2.ub() << "); " <<  yb1.ub() << "," << (yb1*m+c3_t3.first).ub() << endl;
		bool improve=ndsH.addSegment(make_pair(((ya2-c3_t3.first)/m).ub(),ya2.ub()),
										make_pair(yb1.ub(),(yb1*m+c3_t3.first).ub()));
		//py_Plotter::offline_plot(NULL, ndsH.NDS2); getchar();

		if(nds_mode==HAMBURGER){
		  IntervalVector points = ndsH.get_points(c1_t1.first,c2_t2.first,m.mid(),c4_t4.first);
			n_t.dist= std::min(n_t.dist, ndsH.distance(points,m.mid(),c4_t4.first));
		}

		v.insert(c3_t3);
		v.insert(c4_t4);

	}else if(nds_mode==HAMBURGER){
		IntervalVector points = ndsH.get_points(c1_t1.first,c2_t2.first);
		n_t.dist= std::min(n_t.dist,ndsH.distance(points));
	}

	split_mode=MIDPOINT;

	if(split_mode==MIDPOINT)
		n_t.b.insert(t.mid());
	else if(split_mode==MAXDIST){
		double vv;
		if(v.size()>0){
          double c=(ya2-m*ya1).mid();
		  double min_dist=POS_INFINITY;
		  for(auto point:v){
			double dist=std::abs(point.first-c);
			 if(dist<min_dist){
				 min_dist=dist;
				 vv=point.second;
			 }
		  }
		}else{
			if(std::min(ya1.mid(),yb1.mid()) - c1_t1.first < std::min(ya2.mid(),yb2.mid()) - c2_t2.first )
				vv=c2_t2.second;
			else
				vv=c1_t1.second;
		}
		if( std::min(vv-t.lb(),t.ub()-vv)/t.diam() > 0.05 ) n_t.b.insert(vv);
	}else if(split_mode==ALL){
		for(auto vv:v)
			if( std::min(vv.second-t.lb(),t.ub()-vv.second)/t.diam() > 0.05 ) n_t.b.insert(vv.second);
	}

	if(n_t.b.size()==0) n_t.b.insert(t.mid());

	if(nds_mode!=HAMBURGER) return false;
	return true;
}


void OptimizerMOP::report(bool verbose) {

	if (!verbose) {
     cout << endl 	<< "time 	#nodes 		|Y|" << endl;
		 cout << get_time() << " " << get_nb_cells() << " " << ndsH.size() << endl;
	   return;
	}

	switch(status) {
		case SUCCESS: cout << "\033[32m" << " optimization successful!" << endl;
		break;
		case INFEASIBLE: cout << "\033[31m" << " infeasible problem" << endl;
		break;
		case NO_FEASIBLE_FOUND: cout << "\033[31m" << " no feasible point found (the problem may be infesible)" << endl;
		break;
		case UNBOUNDED_OBJ: cout << "\033[31m" << " possibly unbounded objective (f*=-oo)" << endl;
		break;
		case TIME_OUT: cout << "\033[31m" << " time limit " << timeout << "s. reached " << endl;
		break;
		case UNREACHED_PREC: cout << "\033[31m" << " unreached precision" << endl;
	}

	cout << "\033[0m" << endl;

	cout << " cpu time used: " << get_time() << "s." << endl;
	cout << " number of cells: " << get_nb_cells() << endl;

	cout << " number of solutions: "  << ndsH.size() << endl;
	for(auto ub : ndsH.NDS2)
		 cout << "(" << ub.first.first << "," << ub.first.second << "): " << ub.second.mid() << endl;


}


} // end namespace ibex
