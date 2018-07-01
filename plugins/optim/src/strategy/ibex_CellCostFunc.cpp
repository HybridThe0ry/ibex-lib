//============================================================================
//                                  I B E X                                   
// File        : ibex_CellCost_2.cpp
// Author      : Jordan Ninin
// License     : See the LICENSE file
// Created     : Sept 11, 2014
// Last Update : Sept 11, 2014
//============================================================================

#include "ibex_CellCostFunc.h"
#include "ibex_OptimData.h"

namespace ibex {

CellCostFunc::CellCostFunc(bool depends_on_loup) : depends_on_loup(depends_on_loup) {

}

void CellCostFunc::set_loup(double lb) {

}

void CellCostFunc::add_property(Map<Bxp>& map) {

}

void CellCostFunc::set_optim_data(Cell& c, const ExtendedSystem& sys) {

}

CellCostFunc* CellCostFunc::get_cost(criterion crit, int goal_var) {
	switch (crit) {
	case LB :    return new CellCostVarLB(goal_var); break;
	case UB :    return new CellCostVarUB(goal_var); break;
	case C3 :    return new CellCostC3();            break;
	case C5 :    return new CellCostC5();            break;
	case C7 :    return new CellCostC7(goal_var);    break;
	case PU :    return new CellCostPU();            break;
	case PF_LB : return new CellCostPFlb();          break;
	case PF_UB : return new CellCostPFub();          break;
	default:     ibex_error("CellCostFunc::get_cost : error  wrong criterion.");
	             return NULL;
	}
}
// -----------------------------------------------------------------------------------------------------------------------------------

CellCostVarLB::CellCostVarLB(int ind_var) : CellCostFunc(false), goal_var(ind_var) {

}

double CellCostVarLB::cost(const Cell& c) const {
	return c.box[goal_var].lb();
}


// -----------------------------------------------------------------------------------------------------------------------------------

CellCostVarUB::CellCostVarUB(int ind_var) : CellCostFunc(false), goal_var(ind_var) {

}

double CellCostVarUB::cost(const Cell& c) const {
	return c.box[goal_var].ub();
}

// -----------------------------------------------------------------------------------------------------------------------------------

CellCostC3::CellCostC3(double lb) : CellCostFunc(true), loup(lb) {

}

double CellCostC3::cost(const Cell& c) const {
	const BxpOptimData *data = (BxpOptimData*) c.prop[BxpOptimData::prop_id];
	if (data) {
		return -((loup - data->pf.lb()) / data->pf.diam() );
	} else {
		ibex_error("CellCostC3::cost : invalid cost");
		return POS_INFINITY;
	}
}

void CellCostC3::add_property(Map<Bxp>& map) {
	if (!map.found(BxpOptimData::prop_id))
		map.insert_new(BxpOptimData::prop_id, new BxpOptimData());
}

void CellCostC3::set_optim_data(Cell& c, const ExtendedSystem& sys) {
	((BxpOptimData*) c.prop[BxpOptimData::prop_id])->compute_pf(*sys.goal,c.box);
}

// -----------------------------------------------------------------------------------------------------------------------------------

CellCostC5::CellCostC5(double lb) : CellCostFunc(true), loup(lb) {

}

double CellCostC5::cost(const Cell& c) const {
	const BxpOptimData *data = (BxpOptimData*) c.prop[BxpOptimData::prop_id];
	if (data) {
		return (-(data->pu * (loup - data->pf.lb()) / data->pf.diam()));
	} else {
		ibex_error("CellCostC5::cost : invalid cost"); return POS_INFINITY;
	}
}

void CellCostC5::add_property(Map<Bxp>& map) {
	if (!map.found(BxpOptimData::prop_id))
		map.insert_new(BxpOptimData::prop_id, new BxpOptimData());
}

void CellCostC5::set_optim_data(Cell& c, const ExtendedSystem& sys) {
	BxpOptimData *data = (BxpOptimData*) c.prop[BxpOptimData::prop_id];
	data->compute_pu(sys,c.box);
	data->compute_pf(*sys.goal,c.box);
}

// -----------------------------------------------------------------------------------------------------------------------------------

CellCostC7::CellCostC7(int ind_var, double lb) : CellCostFunc(true), loup(lb), goal_var(ind_var) {

}

double CellCostC7::cost(const Cell& c) const {
	const BxpOptimData *data = (BxpOptimData*) c.prop[BxpOptimData::prop_id];
	if (data) {
		return c.box[goal_var].lb()/(data->pu*(loup-data->pf.lb())/data->pf.diam());
	} else {
		ibex_error("CellCostC7::cost : invalid cost"); return POS_INFINITY;
	}
}

void CellCostC7::add_property(Map<Bxp>& map) {
	if (!map.found(BxpOptimData::prop_id))
		map.insert_new(BxpOptimData::prop_id, new BxpOptimData());
}

void CellCostC7::set_optim_data(Cell& c, const ExtendedSystem& sys) {
	BxpOptimData *data = (BxpOptimData*) c.prop[BxpOptimData::prop_id];
	data->compute_pu(sys,c.box);
	data->compute_pf(*sys.goal,c.box);
}

// -----------------------------------------------------------------------------------------------------------------------------------

CellCostPU::CellCostPU() :  CellCostFunc(false) {

}

double CellCostPU::cost(const Cell& c) const {
	const BxpOptimData *data = (BxpOptimData*) c.prop[BxpOptimData::prop_id];
	if (data) {
		return  -data->pu;
	} else {
		ibex_error("CellCostPU::cost : invalid cost"); return POS_INFINITY;
	}
}

void CellCostPU::add_property(Map<Bxp>& map) {
	if (!map.found(BxpOptimData::prop_id))
		map.insert_new(BxpOptimData::prop_id, new BxpOptimData());
}

void CellCostPU::set_optim_data(Cell& c, const ExtendedSystem& sys) {

	((BxpOptimData*) c.prop[BxpOptimData::prop_id])->compute_pu(sys,c.box);
}

// -----------------------------------------------------------------------------------------------------------------------------------

CellCostPFlb::CellCostPFlb() :  CellCostFunc(false) {

}

void CellCostPFlb::add_property(Map<Bxp>& map) {
	if (!map.found(BxpOptimData::prop_id))
		map.insert_new(BxpOptimData::prop_id, new BxpOptimData());
}

void CellCostPFlb::set_optim_data(Cell& c, const ExtendedSystem& sys) {
	((BxpOptimData*) c.prop[BxpOptimData::prop_id])->compute_pf(*sys.goal,c.box);
}

double CellCostPFlb::cost(const Cell& c) const {
	const BxpOptimData *data = (BxpOptimData*) c.prop[BxpOptimData::prop_id];
	if (data) {
		return  data->pf.lb();
	} else {
		ibex_error("CellCostPFlb::cost : invalid cost"); return POS_INFINITY;
	}
}

// -----------------------------------------------------------------------------------------------------------------------------------

CellCostPFub::CellCostPFub() :  CellCostFunc(false) {

}

void CellCostPFub::add_property(Map<Bxp>& map) {
	if (!map.found(BxpOptimData::prop_id))
		map.insert_new(BxpOptimData::prop_id, new BxpOptimData());
}

void CellCostPFub::set_optim_data(Cell& c, const ExtendedSystem& sys) {
	((BxpOptimData*) c.prop[BxpOptimData::prop_id])->compute_pf(*sys.goal,c.box);
}

double CellCostPFub::cost(const Cell& c) const {
	const BxpOptimData *data = (BxpOptimData*) c.prop[BxpOptimData::prop_id];
	if (data) {
		return data->pf.ub();
	} else {
		ibex_error("CellCostPFub::cost : invalid cost"); return POS_INFINITY;
	}
}

// -----------------------------------------------------------------------------------------------------------------------------------

CellCostMaxPFub::CellCostMaxPFub() :  CellCostFunc(false) {

}

void CellCostMaxPFub::add_property(Map<Bxp>& map) {
	if (!map.found(BxpOptimData::prop_id))
		map.insert_new(BxpOptimData::prop_id, new BxpOptimData());
}

void CellCostMaxPFub::set_optim_data(Cell& c, System& sys) {
	((BxpOptimData*) c.prop[BxpOptimData::prop_id])->compute_pf(*sys.goal,c.box);
}

double CellCostMaxPFub::cost(const Cell& c) const {
	const BxpOptimData *data = (BxpOptimData*) c.prop[BxpOptimData::prop_id];
	if (data) {
		return -data->pf.ub();
	} else {
		ibex_error("CellCostMaxPFub::cost : invalid cost");
		return POS_INFINITY;
	}
}

}  // end namespace ibex
