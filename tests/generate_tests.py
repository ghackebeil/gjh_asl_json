#
# The Pyomo script that was used to generate the
# test files in this directory.
#

from pyomo.environ import *

model = ConcreteModel()
model.x = Var([1], initialize=2.0)
model.y = Var()
model.o = Objective(expr=model.x[1]**2 + model.y)
model.c = ConstraintList()
model.c.add(model.x[1]*model.y + model.x[1] >= 1)
model.c.add(model.y == 2)
model.c.add(-10 <= model.y + model.x[1] <= 10.5)

model.dual = Suffix(direction=Suffix.EXPORT)
model.dual[model.c[1]] = 1.0
model.dual[model.c[2]] = 0.1

model.sosno = Suffix(direction=Suffix.EXPORT,datatype=Suffix.INT)
model.sosno[model.x[1]] = 1
model.sosno[model.y] = 1
model.ref = Suffix(direction=Suffix.EXPORT,datatype=Suffix.INT)
model.ref[model.x[1]] = 2
model.ref[model.y] = 3

model.prob_int = Suffix(direction=Suffix.EXPORT,
                           datatype=Suffix.INT)
model.prob_int[model] = 1
model.prob_real = Suffix(direction=Suffix.EXPORT,
                             datatype=Suffix.FLOAT)
model.prob_real[model] = 1.5

model.obj_int = Suffix(direction=Suffix.EXPORT,
                           datatype=Suffix.INT)
model.obj_int[model.o] = 2
model.obj_real = Suffix(direction=Suffix.EXPORT,
                             datatype=Suffix.FLOAT)
model.obj_real[model.o] = 2.5

model.con_int = Suffix(direction=Suffix.EXPORT,
                           datatype=Suffix.INT)
model.con_int[model.c[1]] = 3
model.con_int[model.c[2]] = 4
model.con_int[model.c[3]] = 5
model.con_real = Suffix(direction=Suffix.EXPORT,
                             datatype=Suffix.FLOAT)
model.con_real[model.c[1]] = 3.5
model.con_real[model.c[2]] = 4.5
model.con_real[model.c[3]] = 5.5

model.var_int = Suffix(direction=Suffix.EXPORT,
                           datatype=Suffix.INT)
model.var_int[model.x[1]] = 6
model.var_int[model.y] = 7
model.var_real = Suffix(direction=Suffix.EXPORT,
                             datatype=Suffix.FLOAT)
model.var_real[model.x[1]] = 6.5
model.var_real[model.y] = 7.5

model.write('stub1.nl',
            io_options={'symbolic_solver_labels':True})

model = ConcreteModel()
model.x = Var(bounds=(-1, 1))
model.o = Objective(expr=model.x,
                    sense=maximize)

model.write('stub2.nl')
