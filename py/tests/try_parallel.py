#!/usr/bin/env python2
# -*- coding: utf-8 -*-
'''
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
'''

import random
import operator
from scoop import futures

import multiprocessing

import numpy

from deap import base
from deap import benchmarks
from deap import creator
from deap import tools

import time

def test_deal_pso():
  creator.create("FitnessMax", base.Fitness, weights=(-1.0,))
  creator.create("Particle", list, fitness=creator.FitnessMax, speed=list, 
    smin=None, smax=None, best=None)
  pool=multiprocessing.Pool()
  
  def generate(size, pmin, pmax, smin, smax):
    part = creator.Particle(random.uniform(pmin, pmax) for _ in range(size)) 
    part.speed = [random.uniform(smin, smax) for _ in range(size)]
    part.smin = smin
    part.smax = smax
    return part

  def updateParticle(part, best, phi1, phi2):
      u1 = (random.uniform(0, phi1) for _ in range(len(part)))
      u2 = (random.uniform(0, phi2) for _ in range(len(part)))
#      v_u1 = map(operator.mul, u1, map(operator.sub, part.best, part))
#      v_u2 = map(operator.mul, u2, map(operator.sub, best, part))
      v_u1 = toolbox.map(operator.mul, u1, toolbox.map(operator.sub, part.best, part))
      v_u2 = toolbox.map(operator.mul, u2, toolbox.map(operator.sub, best, part))
      part.speed = list(toolbox.map(operator.add, part.speed, toolbox.map(operator.add, v_u1, v_u2)))
      for i, speed in enumerate(part.speed):
          if speed < part.smin:
              part.speed[i] = part.smin
          elif speed > part.smax:
              part.speed[i] = part.smax
      part[:] = list(toolbox.map(operator.add, part, part.speed))
  
  toolbox = base.Toolbox()
  toolbox.register("particle", generate, size=20, pmin=-15, pmax=30, smin=-3, smax=3)
  #toolbox.register("particle", generate, size=2, pmin=-6, pmax=6, smin=-3, smax=3)
  toolbox.register("population", tools.initRepeat, list, toolbox.particle)
  toolbox.register("update", updateParticle, phi1=2.0, phi2=2.0)
  toolbox.register("evaluate", benchmarks.ackley)
  #pool=multiprocessing.Pool()
  toolbox.register("map", pool.map)
  #toolbox.register("map", futures.map)
  #toolbox.register("evaluate", benchmarks.h1)
  
  pop = toolbox.population(n=1000)
  stats = tools.Statistics(lambda ind: ind.fitness.values)
  stats.register("avg", numpy.mean)
  stats.register("std", numpy.std)
  stats.register("min", numpy.min)
  stats.register("max", numpy.max)

  logbook = tools.Logbook()
  logbook.header = ["gen", "evals"] + stats.fields

  GEN = 10
  best = None

  for g in range(GEN):
      for part in pop:
          part.fitness.values = toolbox.evaluate(part)
          if not part.best or part.best.fitness < part.fitness:
              part.best = creator.Particle(part)
              part.best.fitness.values = part.fitness.values
          if not best or best.fitness < part.fitness:
              best = creator.Particle(part)
              best.fitness.values = part.fitness.values
      for part in pop:
          toolbox.update(part, best)

      # Gather all the fitnesses in one list and print the stats
      logbook.record(gen=g, evals=len(pop), **stats.compile(pop))
      #print(logbook.stream)
  
#  print("pop:")
#  print(pop)
#  print("logbook:")
#  print(logbook)
  print("best")
  print(best)
  return pop, logbook, best


def test_sum():
  bound= 100000
  data = [random.randint(-1000, 1000) for r in range(bound)]
  # Python's standard serial function
  start = time.time()
  serialSum = sum(map(abs, data))
  print(start-time.time())

  # SCOOP's parallel function
  start = time.time()
  parallelSum = futures.mapReduce(abs, operator.add, data)
  print(start-time.time())
  assert serialSum == parallelSum

if __name__ == '__main__':
    #test_sum()
    pop, logbook, best = test_deal_pso()