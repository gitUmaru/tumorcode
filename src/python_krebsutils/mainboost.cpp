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
#include <boost/version.hpp>
#include <boost/preprocessor/stringize.hpp>

#include <boost/python/exception_translator.hpp>
#include "python_helpers.h"
#include <boost/python/object.hpp>
#include <boost/python/tuple.hpp>
#include <boost/python/str.hpp>
#include <boost/python/call.hpp>
#include <boost/python/exec.hpp>
#include <pyerrors.h>

#include <fenv.h>
#include <map>

#include "mwlib/helpers-sys.h"
#include "mwlib/lattice-data.h"
#include "mwlib/random.h"

#include "common.h"
#include "shared-objects.h"//needed for ReadVesselList3d
#include "lattice-data-polymorphic.h"
#include "distancemap.h"
//#include "continuum-flow.h"


#include "pylatticedata.h"
#include "vessels3d.h"

#include "python_elliptic_solver_test.cpp"
enum Mode {
  DATA_PER_NODE = 1,
  DATA_CONST = 0,
  DATA_LINEAR = 4,
};
//#pragma message("BOOST_VERSION=" BOOST_PP_STRINGIZE(BOOST_VERSION))
//#if ((BOOST_VERSION/100)%1000) < 63
bool is_vbl_used()
{
#ifdef MILOTTI_MTS 
  return true;
#else 
  return false;
#endif 
}
py::tuple read_vessel_positions_from_hdf_by_filename(const string fn, const string groupname)
{
  cout << "read vessel from file: " << fn << endl;
  cout << "at: " << groupname << endl;
  //h5cpp::Group g_vess = PythonToCppGroup(vess_grp_obj);
  H5::H5File readInFile;
  H5::Group g_vess;
  std::shared_ptr<VesselList3d> vl;
  try{
   readInFile = H5::H5File(fn, H5F_ACC_RDONLY );
  //h5cpp::Group g_vess = h5cpp::Group(readInFile->root().open_group(groupname)); // groupname should end by vesselgroup
   g_vess = readInFile.openGroup(groupname); // groupname should end by vesselgroup
   vl = ReadVesselList3d(g_vess, make_ptree("filter", false));
  }
  catch(H5::Exception &e)
  {
    e.printErrorStack();
  }
  
  Py_ssize_t ndims[] = { 3, vl->GetNCount() };
  std::vector<float> x; x.resize(vl->GetNCount());
  std::vector<float> y; y.resize(vl->GetNCount());
  std::vector<float> z; z.resize(vl->GetNCount());
  float max_x = std::numeric_limits<float>::min();
  float min_x = std::numeric_limits<float>::max();
  py::list py_x;
  py::list py_y;
  py::list py_z;
//   py::object iter = get_iter(x);
//   py::list py_x(iter);
  
  // create numpy array
  //np::arrayt<float> wp = np::zeros(2, ndims, np::getItemtype<float>());

//   cout << ld << endl;
  Float3 p;
  for (int i=0; i<vl->GetNCount(); ++i)
  {
    VesselNode *nd = vl->GetNode(i);
    if( !vl->HasLattice() )
    {
      p = nd->worldpos;
    }
    else
    {
      myAssert(vl->Ld().IsInsideLattice(nd->lpos));
      p = vl->Ld().LatticeToWorld(nd->lpos);
    }
    if(p[0]<min_x)
      min_x=p[0];
    if(p[0]>max_x)
      max_x=p[0];
    
    py_x.append(p[0]);
    py_y.append(p[1]);
    py_z.append(p[2]);
   
//     for (int j=0; j<3; ++j)
//     {
//       wp(j, i) = p[j];
//     }
    
  }
  readInFile.close();
  std::cout << " min    " << "   max   " << endl;
  std::cout << min_x << "    " << max_x << endl;
  
  //return wp.getObject();
  return py::make_tuple(py_x, py_y, py_z);
}


#if 0 
//depricted since H5Cpp
py::object read_vessel_positions_from_hdf(const py::object &vess_grp_obj)
{
  
  h5cpp::Group g_vess = PythonToCppGroup(vess_grp_obj);
  std::auto_ptr<VesselList3d> vl = ReadVesselList3d(g_vess, make_ptree("filter", false));

  Py_ssize_t ndims[] = { 3, vl->GetNCount() };

  // create numpy array
#if BOOST_VERSION>106300
  py::tuple shape = py::make_tuple(3,vl->GetNCount());
  np::dtype dtype = np::dtype::get_builtin<float>();
  np::ndarray wp = np::zeros(shape, dtype);
#else
  np::arrayt<float> wp = np::zeros(2, ndims, np::getItemtype<float>());
#endif

//   cout << ld << endl;
  Float3 p;
  for (int i=0; i<vl->GetNCount(); ++i)
  {
    VesselNode *nd = vl->GetNode(i);
    if( !vl->HasLattice() )
    {
      p = nd->worldpos;
    }
    else
    {
      myAssert(vl->Ld().IsInsideLattice(nd->lpos));
      p = vl->Ld().LatticeToWorld(nd->lpos);
    }
    for (int j=0; j<3; ++j)
    {
#if BOOST_VERSION>106300
      wp[j][i] = p[j];
#else
      wp(j, i) = p[j];
#endif
    }
  }
#if BOOST_VERSION>106300
  return wp;
#else
  return wp.getObject();
#endif
}
#endif

#if 0
//depricated since H5Cpp
py::object read_vessel_positions_from_hdf_edges(const py::object &vess_grp_obj)
{
  
  h5cpp::Group g_vess = PythonToCppGroup(vess_grp_obj);
  std::auto_ptr<VesselList3d> vl = ReadVesselList3d(g_vess, make_ptree("filter", false));

  Py_ssize_t ndims[] = { 3, vl->GetECount() };

  // create numpy array
#if BOOST_VERSION>106300
  py::tuple shape = py::make_tuple(3,vl->GetECount());
  np::dtype dtype = np::dtype::get_builtin<float>();
  np::ndarray wp = np::zeros(shape, dtype);
#else
  np::arrayt<float> wp = np::zeros(2, ndims, np::getItemtype<float>());
#endif

//   cout << ld << endl;
  Float3 p;
  for (int i=0; i<vl->GetECount(); ++i)
  {
    //VesselNode *nd = vl->GetNode(i);
    Vessel *v = vl->GetEdge(i);
    VesselNode *a = v->NodeA();
    VesselNode *b = v->NodeB();
    if( !vl->HasLattice() )
    {
      //p = nd->worldpos;
      p = 0.5*(a->worldpos-b->worldpos);
    }
    else
    {
      //p = vl->Ld().LatticeToWorld(nd->lpos);
      myAssert(vl->Ld().IsInsideLattice(a->lpos));
      myAssert(vl->Ld().IsInsideLattice(b->lpos));
      Float3 a_pos = vl->Ld().LatticeToWorld(a->lpos);
      Float3 b_pos = vl->Ld().LatticeToWorld(b->lpos);
#ifdef DEBUG
      if(i==42)
      {
	cout<<a_pos<<" und "<< b_pos <<endl;
      }
#endif
      p = 0.5*(a_pos+b_pos);
    }
    for (int j=0; j<3; ++j)
    {
#if BOOST_VERSION>106300
      wp[j][i] = p[j];
#else
      wp(j, i) = p[j];
#endif
    }
  }
#if BOOST_VERSION>106300
  return wp;
#else
  return wp.getObject();
#endif
}
#endif

template<class T> inline void op_logical_and(T &a, const T &b) { a &= b; }
template<> inline void op_logical_and<double>(double &a, const double &T) {}
template<> inline void op_logical_and<float>(float &a, const float &T) {}
template<class T> inline  void op_logical_or(T &a, const T &b) { a |= b; }
template<> inline void op_logical_or<double>(double &a, const double &T) {}
template<> inline void op_logical_or<float>(float &a, const float &T) {}

#if ((BOOST_VERSION/100)%1000) > 63
template<class T>
np::ndarray edge_to_node_property_t(int num_nodes, const np::ndarray &edges, const np::ndarray &prop, const int combinefunc_id)
{
  enum {
    ID_MAX = 1,
    ID_MIN,
    ID_AND,
    ID_OR,
    ID_SUM,
    ID_AVG,
  };
  std::vector<int> nbcount(num_nodes);
  int num_components = prop.get_shape()[1];
  int num_edges = edges.get_shape()[0];
  //Py_ssize_t ndims[2] = { num_nodes, num_components };
  //np::arrayt<T> res(np::zeros(2, ndims, np::getItemtype<T>()));
  np::ndarray res = np::zeros(py::make_tuple(num_nodes, num_components),np::dtype::get_builtin<T>());
  for (int i=0; i<num_edges; ++i)
  {
    for (int j=0; j<2; ++j)
    {
      //const int node_id = edges[i][j];
      const int node_id = py::extract<int>(edges[i][j]);
      const bool first = nbcount[node_id] == 0;
      for (int component_index=0; component_index<num_components; ++component_index)
      {
//         T &r = res(node_id, component_index);
//         const T q = prop(i, component_index);
        //T &r = res.get_data()[node_id][component_index];
        //const T q = prop[i][ component_index];
        const T q =py::extract<T>(prop[i][component_index]);
        if (first)
        {
          //r = q;
          res[node_id][component_index] = q;
        }
        else
        {
          switch (combinefunc_id)
          {
            case ID_MAX:
              //r = std::max(r, q);
              //res[node_id][component_index] = std::fmax(py::extract<T>(res[node_id][component_index]), q);
              break;
            case ID_MIN:
//               r = std::min(r, q);
              //res[node_id][component_index] = std::fmin(py::extract<T>(res[node_id][component_index]), q);
              break;
            case ID_AND:
//               op_logical_and<T>(r, q);
              //op_logical_and<T>(py::extract<T>(res[node_id][component_index]), q);
              break;
            case ID_OR:
//               op_logical_or<T>(r, q);
              //op_logical_or<T>(py::extract<T>(res[node_id][component_index]), q);
              break;
            case ID_AVG:
            case ID_SUM:
              res[node_id][component_index]=res[node_id][component_index]+q;
              //r += q;
              break;
            default:
              throw std::invalid_argument(str(format("edge_to_node_property_t: dont know combinefunc %i") % combinefunc_id));
          }
        }
      }
      nbcount[node_id]++;
    }
  }

  if (combinefunc_id == ID_AVG)
  {
    for (int node_id=0; node_id<num_nodes; ++node_id)
    {
      int n = std::max<int>(1, nbcount[node_id]);
      for (int component_index=0; component_index<num_components; ++component_index)
      {
//         res(node_id, component_index) /= n;
        res[node_id][component_index] /= n;
      }
    }
  }
  return res;
}
#else
template<class T>
np::arraytbase edge_to_node_property_t(int num_nodes, const np::arrayt<int> &edges, const np::arrayt<T> &prop, const int combinefunc_id)
{
  enum {
    ID_MAX = 1,
    ID_MIN,
    ID_AND,
    ID_OR,
    ID_SUM,
    ID_AVG,
  };
  std::vector<int> nbcount(num_nodes);
  int num_components = prop.shape()[1];
  int num_edges = edges.shape()[0];
  Py_ssize_t ndims[2] = { num_nodes, num_components };
  np::arrayt<T> res(np::zeros(2, ndims, np::getItemtype<T>()));
  for (int i=0; i<num_edges; ++i)
  {
    for (int j=0; j<2; ++j)
    {
      const int node_id = edges(i, j);
      const bool first = nbcount[node_id] == 0;
      for (int component_index=0; component_index<num_components; ++component_index)
      {
        T &r = res(node_id, component_index);
        const T q = prop(i, component_index);
        if (first)
        {
          r = q;
        }
        else
        {
          switch (combinefunc_id)
          {
            case ID_MAX:
              r = std::max(r, q);
              break;
            case ID_MIN:
              r = std::min(r, q);
              break;
            case ID_AND:
              op_logical_and<T>(r, q);
              break;
            case ID_OR:
              op_logical_or<T>(r, q);
              break;
            case ID_AVG:
            case ID_SUM:
              r += q;
              break;
            default:
              throw std::invalid_argument(str(format("edge_to_node_property_t: dont know combinefunc %i") % combinefunc_id));
          }
        }
      }
      nbcount[node_id]++;
    }
  }

  if (combinefunc_id == ID_AVG)
  {
    for (int node_id=0; node_id<num_nodes; ++node_id)
    {
      int n = std::max<int>(1, nbcount[node_id]);
      for (int component_index=0; component_index<num_components; ++component_index)
      {
        res(node_id, component_index) /= n;
      }
    }
  }
  return res;
}
#endif

#if ((BOOST_VERSION/100)%1000) > 63
py::object flood_fill(const np::ndarray &py_field, const Int3 &startpos)
{
//   np::arrayt<uchar> field(py_field);
  //np::ndarray field(py_field);
  assert(py_field.get_nd() == 3);
  auto shape_as_Py_intptr_t = py_field.get_shape();
  std::cout << shape_as_Py_intptr_t[0] << "," << shape_as_Py_intptr_t[1]<< "," << shape_as_Py_intptr_t[2] << std::endl;
  //py::tuple shape = py::tuple(py_field.get_shape());
  np::ndarray res = np::zeros(py_field.get_nd(),shape_as_Py_intptr_t,np::dtype::get_builtin<uint>());
  //np::arrayt<uchar> res(np::zeros(field.rank(), field.shape(), np::getItemtype<uchar>()));
  LatticeDataQuad3d ld;
  ld.Init(Int3(shape_as_Py_intptr_t[0], shape_as_Py_intptr_t[1], shape_as_Py_intptr_t[2]), 1.);

//   auto bla = startpos[0];
//   bool bla2 = ld.IsInsideLattice(startpos);
//   auto bal3 = py_field[startpos[0]][startpos[1]][startpos[2]];
  
  if (!ld.IsInsideLattice(startpos) ||
      py_field[startpos[0]][startpos[1]][startpos[2]]) 
    return res;
  
  DynArray<Int3> stack(1024,ConsTags::RESERVE);
  stack.push_back(startpos);
  while(stack.size()>0)
  {
    Int3 p = stack.back();
    stack.pop_back();
    res[p[0]][p[1]][p[2]] = 1;
    for(int i=0; i<LatticeDataQuad3d::DIR_CNT; ++i)
    {
      Int3 pnb = ld.NbLattice(p,i);
      if(!ld.IsInsideLattice(pnb) ||
         res[pnb[0]][pnb[1]][pnb[2]] ||
         py_field[pnb[0]][pnb[1]][pnb[2]])
        continue;
      stack.push_back(pnb);
    }
  }
  std::cout << "done flood_fill " << std::endl;
  return res;
}
#else
py::object flood_fill(const nm::array &py_field, const Int3 &startpos)
{
  np::arrayt<uchar> field(py_field);
  assert(field.rank() == 3);
  np::arrayt<uchar> res(np::zeros(field.rank(), field.shape(), np::getItemtype<uchar>()));
  LatticeDataQuad3d ld;
  ld.Init(Int3(field.shape()[0], field.shape()[1], field.shape()[2]), 1.);

  if (!ld.IsInsideLattice(startpos) ||
      field(startpos[0], startpos[1], startpos[2])) return res.getObject();
  
  DynArray<Int3> stack(1024,ConsTags::RESERVE);
  stack.push_back(startpos);
  while(stack.size()>0)
  {
    Int3 p = stack.back();
    stack.pop_back();
    res(p[0],p[1],p[2]) = 1;
    for(int i=0; i<LatticeDataQuad3d::DIR_CNT; ++i)
    {
      Int3 pnb = ld.NbLattice(p,i);
      if(!ld.IsInsideLattice(pnb) ||
         res(pnb[0],pnb[1],pnb[2]) ||
         field(pnb[0],pnb[1],pnb[2]))
        continue;
      stack.push_back(pnb);
    }
  }
  return res.getObject();
}
#endif


#if ((BOOST_VERSION/100)%1000) > 63
py::object distancemap(const np::ndarray &py_field)
{
  int theDim = py_field.get_nd();
  std::cout << "theDim in distancemap: " << theDim << std::endl;
  std::cout.flush();
  assert(theDim == 3);
  auto shape_as_Py_intptr_t = py_field.get_shape();
  std::cout << shape_as_Py_intptr_t[0] << "," << shape_as_Py_intptr_t[1]<< "," << shape_as_Py_intptr_t[2] << std::endl;
  //np::arrayt<uchar> field(py_field);
//   np::arrayt<float> res = np::zeros(field.rank(), field.shape(), np::getItemtype<float>());
  //py::tuple shape = py::tuple(py_field.get_shape());
  np::ndarray res = np::zeros(py_field.get_nd(),shape_as_Py_intptr_t, np::dtype::get_builtin<float>());

  Array3d<float> arr3d = Array3dFromPy<float>(res);
  LatticeDataQuad3d ld;
  ld.Init(arr3d.getBox(), 1.);
  DistanceFieldComputer dfc;
  
  FOR_BBOX3(p, ld.Box())
  {
    if (py_field[p[0]][p[1]][p[2]])
      arr3d(p) = dfc.DIST_MAX;
    else
      arr3d(p) = 0;
  }

  dfc.Do(ld, arr3d);

  FOR_BBOX3(p, ld.Box())
  {
    if (py_field[p[0]][p[1]][p[2]])
      arr3d(p) *= -1;
    else
      arr3d(p) = dfc.DIST_MAX;
  }

  dfc.Do(ld, arr3d);

  return res;
}
#else
py::object distancemap(const nm::array &py_field)
{
  np::arrayt<uchar> field(py_field);

  np::arrayt<float> res = np::zeros(field.rank(), field.shape(), np::getItemtype<float>());

  Array3d<float> arr3d = Array3dFromPy<float>(res);
  LatticeDataQuad3d ld;
  ld.Init(arr3d.getBox(), 1.);
  DistanceFieldComputer dfc;
  
  FOR_BBOX3(p, ld.Box())
  {
    if (field(p[0],p[1],p[2]))
      arr3d(p) = dfc.DIST_MAX;
    else
      arr3d(p) = 0;
  }

  dfc.Do(ld, arr3d);

  FOR_BBOX3(p, ld.Box())
  {
    if (field(p[0],p[1],p[2]))
      arr3d(p) *= -1;
    else
      arr3d(p) = dfc.DIST_MAX;
  }

  dfc.Do(ld, arr3d);

  return res.getObject();
}
#endif

#if BOOST_VERSION>106300
template<class T>
np::ndarray diff_field(const np::ndarray &py_field, int axis, double prefactor)
{
  auto shape_of_py_field_as_Py_intptr_t = py_field.get_shape();
  
#ifndef NDEBUG
  std::cout << "in diff_field with templated data type" << std::endl;
  std::cout << shape_of_py_field_as_Py_intptr_t[0] << ","<< shape_of_py_field_as_Py_intptr_t[1] << ","<< shape_of_py_field_as_Py_intptr_t[2] << std::endl;
#endif
//   Array3d<T> arr3d = Array3dFromPy<T>(py_field);
//   Int3 ex(0);
//   for (int i=0; i<dim; ++i) ex[i] = -1;
//   arr3d = arr3d[arr3d.getBox().Extend(ex)];
//   CopyBorder(arr3d, dim, 1);
  
//   std::unique_ptr<Array3d<T>> field_ptr = std::make_unique<Array3dFromPy<T>>(py_field);
  
  //*field_ptr = Array3dFromPy<T>(py_field);
  Array3d<T> field = Array3dFromPy<T>(py_field);
  
  std::unique_ptr<Array3d<T>> my_p_to_field( new Array3d<T>( std::move(field) ) );
  
  
  const BBox3 bb = my_p_to_field->getBox();
  //Array3d<T> field(ExtendForDim(bb, 3, 1));
  //field[bb].fill(arr3d);
  //CopyBorder(field[bb], 3, 1);

  //np::arrayt<T> py_res = np::zeros(3, ::Size(bb).cast<Py_ssize_t>().eval().data(), np::getItemtype<T>());
  np::ndarray py_res = np::zeros(py_field.get_nd(),shape_of_py_field_as_Py_intptr_t, np::dtype::get_builtin<T>());
//   std::unique_ptr<Array3d<T>> my_p_to_py_res( new Array3d<T>( std::move(py_res) ) );
//   std::unique_ptr<Array3d<T>> myResPointer;
//   *myResPointer = Array3dFromPy<T>(py_res);

  FOR_BBOX3(p, bb)
  {
    Int3 p0(p), p1(p);
    float f = 0.5;
    if (p[axis]<bb.max[axis])
      ++p1[axis];
    else
      f = 1.;
    if (p[axis]>bb.min[axis])
      --p0[axis];
    else
      f = 1.;
    py_res[p[0]][p[1]][p[2]] = f*prefactor*((*my_p_to_field)(p1)-(*my_p_to_field)(p0));
  }

  return py_res;
}
#else
template<class T>
py::object diff_field(np::arrayt<T> py_field, int axis, double prefactor)
{
//   Array3d<T> arr3d = Array3dFromPy<T>(py_field);
//   Int3 ex(0);
//   for (int i=0; i<dim; ++i) ex[i] = -1;
//   arr3d = arr3d[arr3d.getBox().Extend(ex)];
//   CopyBorder(arr3d, dim, 1);
  Array3d<T> field = Array3dFromPy<T>(py_field);
  const BBox3 bb = field.getBox();
  //Array3d<T> field(ExtendForDim(bb, 3, 1));
  //field[bb].fill(arr3d);
  //CopyBorder(field[bb], 3, 1);

  np::arrayt<T> py_res = np::zeros(3, ::Size(bb).cast<np::ssize_t>().eval().data(), np::getItemtype<T>());
  Array3d<T> res = Array3dFromPy<T>(py_res);

  FOR_BBOX3(p, bb)
  {
    Int3 p0(p), p1(p);
    float f = 0.5;
    if (p[axis]<bb.max[axis])
      ++p1[axis];
    else
      f = 1.;
    if (p[axis]>bb.min[axis])
      --p0[axis];
    else
      f = 1.;
    res(p) = f*prefactor*(field(p1)-field(p0));
  }

  return py_res.getObject();
}
#endif


#if ((BOOST_VERSION/100)%1000) > 63
py::object SumIsoSurfaceIntersectionWithVessels(float level, np::ndarray py_edgelist, np::ndarray py_pressure, np::ndarray py_flags, np::ndarray py_nodalLevel, np::ndarray py_datavalue)
{
//   np::arrayt<int> edges(py_edgelist);
//   np::arrayt<float> pressure(py_pressure);
//   np::arrayt<int> flags(py_flags);
//   np::arrayt<float> nodalLevel(py_nodalLevel);
//   np::arrayt<double> dataValue(py_datavalue);
  
  double dataSumIn = 0., dataSumOut = 0.;
  int nVessels = py_edgelist.get_shape()[0];
  float nodealLevel_a=0.0;
  float nodealLevel_b=0.0;
  float pressure_a=0.0;
  float pressure_b=0.0;
  double dataValue = 0.0;
  int flag_i=0;
  for (int i=0; i<nVessels; ++i)
  {
    int a = py::extract<int>(py_edgelist[i][0]);
    int b = py::extract<int>(py_edgelist[i][0]);
    flag_i = py::extract<int>(py_flags[i]);
    if (!(flag_i & CIRCULATED)) continue;
    nodealLevel_a = py::extract<float>(py_nodalLevel[a]);
    nodealLevel_b = py::extract<float>(py_nodalLevel[b]);
    dataValue = py::extract<double>(py_datavalue[i]);
    pressure_a = py::extract<float>(py_pressure[a]);
    pressure_b = py::extract<float>(py_pressure[b]);
    if (nodealLevel_a<level && nodealLevel_b>level) //b is in the tumor
    {
      if (pressure_a<pressure_b)
      {
        dataSumOut += dataValue;
      }
      else
      {
        dataSumIn += dataValue;
      }
    }
    else if(nodealLevel_a>level && nodealLevel_b<level) //: # a is in the tumor
    {
      if (pressure_a>pressure_b)
      {
        dataSumOut += dataValue;
      }
      else
      {
        dataSumIn += dataValue;
      }
    }
  }
  return py::make_tuple(dataSumIn, dataSumOut);
}
#else
py::object SumIsoSurfaceIntersectionWithVessels(float level, nm::array py_edgelist, nm::array py_pressure, nm::array py_flags, nm::array py_nodalLevel, nm::array py_datavalue)
{
  np::arrayt<int> edges(py_edgelist);
  np::arrayt<float> pressure(py_pressure);
  np::arrayt<int> flags(py_flags);
  np::arrayt<float> nodalLevel(py_nodalLevel);
  np::arrayt<double> dataValue(py_datavalue);
  
  double dataSumIn = 0., dataSumOut = 0.;
  int nVessels = edges.shape()[0];
  for (int i=0; i<nVessels; ++i)
  {
    int a = edges(i, 0);
    int b = edges(i, 1);
    if (!(flags(i) & CIRCULATED)) continue;
    if (nodalLevel(a)<level && nodalLevel(b)>level) //b is in the tumor
    {
      if (pressure(a)<pressure(b))
      {
        dataSumOut += dataValue(i);
      }
      else
      {
        dataSumIn += dataValue(i);
      }
    }
    else if(nodalLevel(a)>level && nodalLevel(b)<level) //: # a is in the tumor
    {
      if (pressure(a)>pressure(b))
      {
        dataSumOut += dataValue(i);
      }
      else
      {
        dataSumIn += dataValue(i);
      }
    }
  }
  return py::make_tuple(dataSumIn, dataSumOut);
}
#endif


/* computes c(r) = <a(x)*y(x+r)>_{x,|r|}, where |r| is a fixed parameter argument.
 * The averaging is done over all points x, and concentric shells around it of radius |r|.
 */
#if ((BOOST_VERSION/100)%1000) > 63
#else
template<class T>
py::tuple radial_correlation(np::arrayt<T> py_field1, np::arrayt<T> py_field2, Int3 distance, int super_samples, bool subtract_avg, py::object &py_obj_mask)
{
  Py_ssize_t num_bins = maxCoeff(distance)*super_samples;
  LatticeWorldTransform<1> ld(1./super_samples);
  ld.SetCellCentering(Vec<bool,1>(true));
  ld.SetOriginPosition(Vec<float,1>(-ld.Scale()*0.5));
  
  std::vector<double> h_cnt(num_bins);
  std::vector<double> h_corr(num_bins);
  std::vector<double> h_sqr(num_bins);

  Array3d<T> field1 = Array3dFromPy<T>(py_field1);
  Array3d<T> field2 = Array3dFromPy<T>(py_field2);
  myAssert(field1.getBox() == field2.getBox());

  Array3d<uchar> mask; // obtain mask if available
  bool use_mask = false;
  if (!py_obj_mask.is_none())
  {
    np::arrayt<uchar> py_mask(py_obj_mask);
    mask = Array3dFromPy<uchar>(py_mask);
    myAssert(mask.getBox() == field1.getBox());
    use_mask = true;
  }
  
  Random rnd;
  
  BBox3 displacements = BBox3().Add(Int3(0)).Extend(distance);

  double average1 = 0., average2 = 0.;
  if (subtract_avg)
  {
    FOR_BBOX3(p, field1.getBox())
    {
      average1 += field1(p);
      average2 += field2(p);
    }
    average1 /= Volume(field1.getBox());
    average2 /= Volume(field2.getBox());
  }
  
  FOR_BBOX3(dp, displacements)
  {
    Float3 fdp = dp.cast<float>();
    const BBox3 bb1 = field2.getBox().Move(dp).Intersection(field1.getBox());
    FOR_BBOX3(p, bb1)
    {
      if (use_mask && (!mask(p) || !mask(p-dp))) continue;
      
      Float3 rdp(rnd.Get11(),rnd.Get11(),rnd.Get11());
      float r = (rdp + fdp).norm();

      int index = ld.WorldToLattice(Vec<float, 1>(r))[0];

      if (index >= num_bins) continue;

      double c = (field1(p)-average1) * (field2(p-dp)-average2);
      h_cnt[index] += 1.;
      h_corr[index] += c;
      h_sqr[index] += c*c;
    }
  }

  np::arrayt<double> py_res_r = np::zeros(1, &num_bins, np::getItemtype<double>());
  np::arrayt<double> py_res_c = np::zeros(1, &num_bins, np::getItemtype<double>());
  np::arrayt<double> py_res_n = np::zeros(1, &num_bins, np::getItemtype<double>());
  np::arrayt<double> py_res_s = np::zeros(1, &num_bins, np::getItemtype<double>());
  for (int i=0; i<num_bins; ++i)
  {
    py_res_r[i] = ld.LatticeToWorld(Vec<int,1>(i))[0];
    py_res_c[i] = h_corr[i];
    py_res_n[i] = h_cnt[i];
    py_res_s[i] = h_sqr[i];
  }
  return py::make_tuple(py_res_r.getObject(), py_res_n.getObject(), py_res_c.getObject(), py_res_s.getObject());
}
#endif

class PyLerp
{
  Float3 o,s;

public:
  PyLerp(const Float3 &o, const Float3 &s) : o(o), s(s) {}
  py::tuple apply(const py::tuple &pyc) const
  {
    float c0 = py::extract<float>(pyc[0]);
    float c1 = py::extract<float>(pyc[1]);
    float c2 = py::extract<float>(pyc[2]);
    Float3 c(c0, c1, c2);
    Float3 res = c.cwiseProduct(s) + o;
    return py::make_tuple(res[0], res[1], res[2]);
  }

  static void export_me()
  {
    py::class_<PyLerp, boost::noncopyable>("PyLerp", py::init<Float3,Float3>())
    .def("apply", &PyLerp::apply);
  }
};


#if 0
py::object test(np::ndarray arg)
{
#if 1
  np::arrayt<float> accarg(arg);

  DynArray<Float2> tmp(accarg.shape()[0]);

  myAssert(accarg.rank()==2 && accarg.shape()[1]==2);

  np::copy<float,2>((float*)get_ptr(tmp), Int2(tmp.size(), 2).data(), calc_strides::last_dim_varies_fastest(Int2(tmp.size(), 2)).data(), arg);

  for (int i=0; i<tmp.size(); ++i) cout << tmp[i] << endl;

  py::object a = np::copy<float,2>(Int2(tmp.size(),2).data(), (float*)get_ptr(tmp), calc_strides::last_dim_varies_fastest(Int2(tmp.size(), 2)).data());
  return a;
#else
  py::object ld(LatticeDataQuad3d(BBox3(0, 0, 0, 10, 100, 1000), 30.));
  return ld;
#endif
  //return py::eval(py::str(str(format("numpy.zeros((%i,%i), dtype=numpy.float32)") % 5 % 2)));
}
#endif

// fwd declare since they are stored in other files
namespace mw_py_impl
{
  void exportLatticeData();
  void exportVectorClassConverters();
  //void exportH5Converters();
}
void export_povray_export();
void export_samplevessels();
void export_model_helpers();
void export_NumericalToolsTests();
//void export_iffsim();
void export_vesselgen();
void export_calcflow();
void export_compute_interpolation_field();
void export_get_Murray();

// see http://www.boost.org/doc/libs/1_66_0/libs/python/doc/html/tutorial/tutorial/exception.html
struct VesselGenException : std::exception
{
  char const* what() const noexcept { return "One of my exceptions"; }
};
void translator(VesselGenException const& x) {
  PyErr_SetString(PyExc_UserWarning, x.what());
}
#ifndef NDEBUG
BOOST_PYTHON_MODULE(libkrebs_d)
#else
BOOST_PYTHON_MODULE(libkrebs_)
#endif
{
  Py_Initialize();
#if BOOST_VERSION>106300
  np::initialize();
#else
  np::importNumpyAndRegisterTypes();
#endif
  
  PyEval_InitThreads(); // need for release of the GIL (http://stackoverflow.com/questions/8009613/boost-python-not-supporting-parallelism)
  // setup everything to work with threads.
  //HACK2018
  //my::initMultithreading(0, NULL, 1);
  // register function to set the number of threads
  //py::def("set_num_threads", my::SetNumThreads);
  
  mw_py_impl::exportVectorClassConverters();
  mw_py_impl::exportLatticeData();
  //mw_py_impl::exportH5Converters();
  
  my::checkAbort = PyCheckAbort; // since this is the python module, this is set to use the python signal check function

  //py::numeric::array::set_module_and_type("numpy", "ndarray"); // use numpy
  //py::array::set_module_and_type("numpy", "ndarray"); // use numpy
  
  // register some python wrapped functions
  //py::def("test", test);
  /* depricated */
  //py::def("read_vessel_positions_from_hdf", read_vessel_positions_from_hdf);
  
  py::def("read_vessel_positions_from_hdf_by_filename", read_vessel_positions_from_hdf_by_filename);
  /* depricated */
  //py::def("read_vessel_positions_from_hdf_edges", read_vessel_positions_from_hdf_edges);
  
  py::def("flood_fill", flood_fill);
  py::def("distancemap", distancemap);
  py::def("GetHealthyVesselWallThickness", GetInitialThickness);
  py::def("SumIsoSurfaceIntersectionWithVessels_", SumIsoSurfaceIntersectionWithVessels);
  py::def("is_vbl_used", is_vbl_used);
  // using macros to register some more functions
#define DEFINE_edge_to_node_property_t(T) \
  py::def("edge_to_node_property_"#T, edge_to_node_property_t<T>);
  DEFINE_edge_to_node_property_t(double)
  DEFINE_edge_to_node_property_t(float)
  DEFINE_edge_to_node_property_t(int)
  DEFINE_edge_to_node_property_t(uint)
  DEFINE_edge_to_node_property_t(char)
  DEFINE_edge_to_node_property_t(uchar)
#undef DEFINE_edge_to_node_property_t
#define DEFINE_diff_field_t(T)\
  py::def("diff_field_"#T, diff_field<T>);
  DEFINE_diff_field_t(float)
  DEFINE_diff_field_t(double)
// #define DEFINE_radial_correlation_t(T)\
//   py::def("radial_correlation_"#T, radial_correlation<T>);
//   DEFINE_radial_correlation_t(float)
//   DEFINE_radial_correlation_t(double)
  
  export_povray_export();
  export_samplevessels();
  export_model_helpers();
  export_NumericalToolsTests();
  //export_iffsim();
  export_vesselgen();
  export_calcflow();
  export_get_Murray();
  export_compute_interpolation_field();
  export_elliptic_solver_test();
  PyLerp::export_me();

}


