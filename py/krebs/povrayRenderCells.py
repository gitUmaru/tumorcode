#!/usr/bin/env python2
# -*- coding: utf-8 -*-
'''
This file is part of tumorcode project.
(http://www.uni-saarland.de/fak7/rieger/homepage/research/tumor/tumor.html)

Copyright (C) 2018  Thierry Fredrich

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
'''
'''
a nice rendering command
submitPovrayRender fakeTumMTS-default-typeI-sample00-vbl_safe_1.h5 out0442 --fontcolor='white' --background=0.0 --cam=pie -f --cells
'''

if __name__ == '__main__':
  import os.path, sys
  sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)),'..'))


import os,sys
from os.path import basename, splitext

import krebsutils
import numpy as np
import povrayRenderVessels
#from povrayRenderVessels import *
import povrayEasy
import myutils
import math
import copy
import matplotlib

def paraview_color_space(scheme='jet'):
  #colors = eval(scheme);
  N = len(colors);
  outfile_name = scheme + '.xml'
  with open(outfile_name, 'w') as f:
    f.write('<ColorMap name="%s" space="HSV">\n'%scheme)
    for i in range(N):
      #x = [(i-1)/(N-1); colors(i,:)'];
      f.write('  <Point x="%f" o="1" r="%f" g="%f" b="%f"/>\n' % x)
    f.write('</ColorMap>');

def convert_to_mmHg(o2_in_pg, cell_radii):
  cell_o2_concentration = o2_in_pg/ (4/float(3)* np.pi*np.power(cell_radii,3))
  #cell_o2_concentration = cell_o2_mass
  volume_o2_ml = cell_o2_concentration/(1.429*1e9)
  #volume_o2_ml = cell_o2_mass/1.429
  ''' o2 density 1.429 g/L --> 1.429*10^9 pg/ml
      1cm^3 = 10^12 (mum^3)
  '''
  solubility = 3.1e-3 #ml O2/cm^3 mmHg
  #solubility = 2.8e-3 #ml O2/cm^3 mmHg
  #solubility = 1.1e-4 #ml O2/cm^3 mmHg
  solubility = solubility*1e-12 #ml O2/mum^3 mmHg
  #volume_density = 1.429e9 #pg/ml
  #x = cell_o2_concentration/volume_density # ml / mum^3
  
  #cell_po2 = x/solubility
  
  return volume_o2_ml/solubility

def createColormapForCells(data_color):
  ''' convert data to color here'''
  data_color = data_color
  print(data_color[0:100])
  cNorm = matplotlib.colors.Normalize(vmin=np.min(data_color), vmax=np.max(data_color))
  scalar_map = matplotlib.cm.ScalarMappable(norm=cNorm, cmap='terrain')
  return scalar_map

def addVBLCells(epv, quantity , cell_hdf_group, options):
    print('path to vbl group is: %s' % cell_hdf_group.name)
    print('adding quantity: %s' % quantity)
    
    position_of_cells= np.asarray(cell_hdf_group['cell_center_pos'])
    radii_of_cells= np.asarray(cell_hdf_group['cell_radii'])
    
    ''' convert data to color here'''
    data_of_cells = np.asarray(cell_hdf_group[quantity])
    if quantity == 'o2':
      data_of_cells= convert_to_mmHg(data_of_cells, radii_of_cells )
    cells_cm = createColormapForCells(data_of_cells[:,0])
    
    
    #data_of_cells= np.asarray(cell_hdf_group[quantity])
    data_color = data_of_cells[:,0]
    
#    print(data_color[0:100])
#    cNorm = matplotlib.colors.Normalize(vmin=np.min(data_color), vmax=np.max(data_color))
#    scalar_map = matplotlib.cm.ScalarMappable(norm=cNorm, cmap='terrain')
#    scalar_map.get_cmap()
    print(cells_cm.get_clim())
    
    print(data_color.shape)
    data_color = cells_cm.to_rgba(data_color)
    print(data_color.shape)
    data_color=data_color[:,:3]
    print(data_color.shape)
    rgb_colors_of_cells=data_color
    
    wbbox = options.wbbox
    trafo = povrayEasy.calc_centering_normalization_trafo(wbbox)
    
    epv.addVBLCells(trafo, position_of_cells, radii_of_cells, rgb_colors_of_cells, options)
    
    return cells_cm
    
#    ld = krebsutils.read_lattice_data_from_hdf_by_filename(str(tumorgroup.file.filename), str(tumorgroup['conc'].attrs['LATTICE_PATH']))
#    ld = transform_ld(trafo, ld)
#
#    ds_necro    = np.asarray(tumorgroup['necro'])
#    data        = np.clip(np.asarray(tumorgroup['conc']), 0., 1.) # - np.asanyarray(tumorgroup['necro']), 0., 1.)
#
#    ds_levelset = -np.minimum(np.asarray(tumorgroup['ls']), 0.4 - ds_necro)
#    ds_levelset = krebsutils.distancemap(ds_levelset) * ld.scale
#
##    import matplotlib
##    matplotlib.use('Qt4Agg')
##    import matplotlib.pyplot as pyplot
##    pyplot.imshow(ds_levelset[:,:,8])
##    pyplot.contour(ds_levelset[:,:,8],[0.])
##    pyplot.show()
#    if 'tumor_clip' in options:
#      clip = clipFactory(options.tumor_clip)
#    else:
#      clip = clipFactory('None')
#
#    voldata_ls    = epv.declareVolumeData(ds_levelset, ld.GetWorldBox())
#    voldata_cells = epv.declareVolumeData(data, ld.GetWorldBox())
#
#    value_bounds = voldata_cells.value_bounds
#    style = """
#      texture {
#        pigment {
#          function { %f + %f*%s(x,y,z) }
#          color_map {
#            [0.0  color <0.3,0,0>]
#            [0.5  color <1,0.8, 0.3>]
#            [0.8  color <1,1,0.1>]
#          }
#        }
#        finish { 
#          specular 0.3
#        }
#      }""" % (value_bounds[0], (value_bounds[1]-value_bounds[0]), voldata_cells.name)
#    #style = " texture { pigment { color rgb<1,0.8,0.3> }  finish { specular 0.3 }}"
#    epv.addIsosurface(voldata_ls, 0., lambda : style, clip, style)

if __name__ == '__main__':
  paraview_color_space()