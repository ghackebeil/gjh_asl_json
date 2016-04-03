g3 1 1 0	# problem unknown
 2 3 1 1 1 	# vars, constraints, objectives, ranges, eqns
 1 1 0 0 0 0	# nonlinear constrs, objs; ccons: lin, nonlin, nd, nzlb
 0 0	# network constraints: nonlinear, linear
 2 1 1 	# nonlinear vars in constraints, objectives, both
 0 0 0 1	# linear network variables; functions; arith, flags
 0 0 0 0 0 	# discrete variables: binary, integer, nonlinear (b,c,o)
 5 2 	# nonzeros in Jacobian, obj. gradient
 0 0	# max name lengths: constraints, variables
 0 0 0 0 0	# common exprs: b,c,o,c1,o1
S7 1 prob_real
0 1.5
S6 1 obj_real
0 2.5
S4 2 var_real
0 6.5
1 7.5
S0 2 var_int
0 6
1 7
S3 1 prob_int
0 1
S1 3 con_int
0 3
1 4
2 5
S2 1 obj_int
0 2
S5 3 con_real
0 3.5
1 4.5
2 5.5
S0 2 sosno
0 2
1 3
C0	#c[1]
o2	#*
v0	#x[1]
v1	#y
C1	#c[2]
n0
C2	#c[3]
n0
O0 0	#o
o5	#pow
v0	#x[1]
n2.0
d2	# dual initial guess
0 1.0
1 0.1
x1	# initial guess
0 2.0
r	#3 ranges (rhs's)
2 1.0
4 2.0
0 -10.0 10.5
b	#2 bounds (on variables)
3
3
k1	#intermediate Jacobian column lengths
2
J0 2
0 1.0
1 0
J1 1
1 1
J2 2
0 1
1 1
G0 2
0 0
1 1.0
