/*
*** NOTE: Although the output from WriteSummary() can be parsed
          using both JSON and YAML, currently the values -inf and inf
          are written in such a way that only JSON will correctly interpret
	  them for Python. In summary:

	  Current implementation -
	  C (-inf, inf) -> JSON (-Infinity, Infinity)
	  -> parsed by python JSON becomes Python (-inf, inf)
	  -> parsed by python YAML becomes Python ('-Infinity','Infinity')

	  Alternative implementation -
	  C (-inf, inf) -> YAML (-.inf, .inf)
	  -> parsed by python JSON becomes Python ERROR
	  -> parsed by python YAML becomes Python (-inf,inf)
*/

#ifndef __AMPLINTERFACE_HPP__
#define __AMPLINTERFACE_HPP__

#include <iostream>
#include <fstream>
#include <cassert>
#include <vector>
#include <map>
#include <string>

/* forward declaration */
struct ASL_pfgh;

class AmplInterface
{
public:
   AmplInterface(int argc, char**& argv);

   virtual ~AmplInterface();

   void WriteSummary(std::ostream& out);

   // Sets up interface for subsequent calls to
   // struct_hes_lag and eval_hes_lag
   // and determines nnz_hes_lag_ for the current objective
   void set_objective(int objective_number);

   void get_nlp_dimensions(int& n_x, int& n_c, int& nnz_jac_c, int& nnz_hes_lag) const;

   void get_con_bounds(double* c_l, double* c_u) const;

   void get_var_bounds(double* x_l, double* x_u) const;

   void eval_f(const double* x, double& f) const;

   void eval_deriv_f(const double* x, double* deriv_f) const;

   void eval_c(const double* x, double* c) const;

   void struct_jac_c(int* irow, int* jcol) const;

   void eval_jac_c(const double* x, double* jac_c_values) const;

   void struct_hes_lag(int* irow, int* jcol) const;

   // NOTE: This must be called AFTER a call to objval and conval
   // (i.e. You must call eval_f and eval_c with the same x before calling this)
   void eval_hes_lag(const double* lam_c, double* hes_lag) const;

   void get_primal_starting_point(double* x) const;
   // sparse version, map is cleared, then filled with only
   // user supplied values
   void get_primal_starting_point(std::map<int,double>& x) const;

   void get_dual_starting_point(double* x) const;
   // sparse version, map is cleared, then filled with only
   // user supplied values
   void get_dual_starting_point(std::map<int,double>& x) const;

   std::string stubname;

private:

   // Default Constructor
   AmplInterface();

   // Copy Constructor
   AmplInterface(const AmplInterface&);

   // Overloaded Equals Operator
   void operator=(const AmplInterface&);

   // ASL pointer
   ASL_pfgh* asl_;

   // keep track if which objective is being used
   int objn_;

   // obj. sense ... -1 = maximize, 1 = minimize
   std::vector<double> obj_sense_;

   // number of nonzeros in the hessian
   int nnz_hes_lag_;

   // output maps to translate ASL indices to
   // variables/constraint names if .col/.row
   // files are provided
   std::map<int,std::string> rows_map;
   std::map<int,std::string> cols_map;
};

#endif
