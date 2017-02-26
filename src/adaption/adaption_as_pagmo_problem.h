/**
This file is part of tumorcode project.
(http://www.uni-saarland.de/fak7/rieger/homepage/research/tumor/tumor.html)

Copyright (C) 2016  Michael Welter and Thierry Fredrich

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PAGMO_PROBLEM_ADAPTION_H
#define PAGMO_PROBLEM_ADAPTION_H

// #include <list>
// #include <memory>
// #include <fstream>
// #include <string>
// 
// #include <cstdio> // remove, std::autoptr inteface wrong in dinkumware
// #include <boost/config.hpp>
// #if defined(BOOST_NO_STDC_NAMESPACE)
// namespace std{ 
//     using ::remove;
// }
// #endif

//#include <boost/archive/tmpdir.hpp>
//#include <boost/archive/text_oarchive.hpp>
//#include <boost/archive/text_iarchive.hpp>

//#include <boost/serialization/serialization.hpp>
//#include <boost/serialization/split_free.hpp> // comes all trough pagmo

#include <boost/foreach.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>


//#define PAGMO_ENABLE_MPI --> done in cmake now
#include <pagmo/src/pagmo.h>
#include <pagmo/src/config.h>
#include <pagmo/src/serialization.h>
#include <pagmo/src/types.h>
#include <pagmo/src/problem/base.h>

#include <adaption/adaption_model2.h>

namespace pagmo{ namespace problem {
  /* this class is meant to implement the 
   * radii adaption problem to pagmo
   * in order to estimate decent parameters*/
class __PAGMO_VISIBLE adaption_problem : public base
{
	public:
		//constructor
		adaption_problem(std::auto_ptr<VesselList3d> vl,Adaption::Parameters params_, BloodFlowParameters bfparams) ;
		//copy constructor
		base_ptr clone() const;
		std::string get_name() const;
		Adaption::Parameters get_params() const;
		BloodFlowParameters get_bfparams() const;
		std::auto_ptr<VesselList3d> get_vl() const;
	protected:
		void objfun_impl(fitness_vector &, const decision_vector &) const;
	private:
		friend class boost::serialization::access;
		template <class Archive>
		void serialize(Archive &ar, unsigned int);
		mutable Adaption::Parameters params;
		BloodFlowParameters bfparams;
		mutable std::auto_ptr<VesselList3d> vl;
};


}} //namespaces

BOOST_CLASS_EXPORT_KEY(pagmo::problem::adaption_problem)


//from http://www.boost.org/doc/libs/1_54_0/libs/serialization/example/demo_auto_ptr.cpp
namespace boost{ namespace serialization{
template<class Archive>
inline void save_construct_data(
  Archive &ar, const pagmo::problem::adaption_problem *t, const unsigned int file_version)
{
  //save data required to construct instances
  Adaption::Parameters tmp_params = t->get_params();
  ar<<tmp_params;
  BloodFlowParameters tmp_bfparams = t->get_bfparams();
  ar<<tmp_bfparams;
  std::auto_ptr<VesselList3d> tmp_vl = t->get_vl();
  ar<<tmp_vl;
}
template<class Archive>
inline void load_construct_data(
  Archive &ar, pagmo::problem::adaption_problem *t, const unsigned int file_version)
{
  //retrieve data from archive required to construct new instance
  Adaption::Parameters params;
  ar >> params;
  BloodFlowParameters bfparams;
  ar >> bfparams;
  std::auto_ptr<VesselList3d> vl;
  cout<<"I am here "<<endl;
  //vl->init_from_other_vl();
  ar >> vl;
  // invoke inplace constructor to initialize instance of adaption_problem
  ::new(t)pagmo::problem::adaption_problem(vl,params,bfparams);
  //t(vl,params,bfparams);
}
/////////////////////////////////////////////////////////////
// implement serialization for auto_ptr< T >
// note: this must be added to the boost namespace in order to
// be called by the library
template<class Archive, class T>
inline void save(
    Archive & ar,
    const std::auto_ptr< T > &t,
    const unsigned int file_version
){
    // only the raw pointer has to be saved
    // the ref count is rebuilt automatically on load
    const T * const tx = t.get();
    ar << tx;
}

template<class Archive, class T>
inline void load(
    Archive & ar,
    std::auto_ptr< T > &t,
    const unsigned int file_version
){
    T *pTarget;
    ar >> pTarget;
    // note that the reset automagically maintains the reference count
    #if BOOST_WORKAROUND(BOOST_DINKUMWARE_STDLIB, == 1)
        t.release();
        t = std::auto_ptr< T >(pTarget);
    #else
        t.reset(pTarget);
    #endif
}

// split non-intrusive serialization function member into separate
// non intrusive save/load member functions
template<class Archive, class T>
inline void serialize(
    Archive & ar,
    std::auto_ptr< T > &t,
    const unsigned int file_version
){
    boost::serialization::split_free(ar, t, file_version);
}


}}//namespace boost{ namespace serialization{

#endif