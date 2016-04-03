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

/* forward declarations */
struct ASL_pfgh;
struct Option_Info;

class AmplInterface
{
public:
   AmplInterface(int argc, char**& argv);

   virtual ~AmplInterface();

   // get the nl file stub name
   std::string get_stubname() const {return stubname_;}

   // write a sol file
   void write_solution_file();

   // summarize everything in JSON format
   void write_json_summary(std::ostream& out);

   double dual_assumed() const {return dual_assumed_;}
   double primal_assumed() const {return primal_assumed_;}

   void set_dual_assumed(double x) {dual_assumed_ = x;}
   void set_primal_assumed(double x) {primal_assumed_ = x;}

   // get the current objective
   int current_objective() const {return objn_;}

   // get the number of objectives for this NLP
   int objective_count();

   // Sets up interface for subsequent calls to
   // struct_hes_lag and eval_hes_lag
   // and determines nnz_hes_lag_ for the current objective
   void set_current_objective(int objective_number);

   // return the objective sense for the current objective
   // -1 = maximize, 1 = minimize
   int current_objective_sense() const {return obj_sense_;}

   // called after setting each objective
   void nlp_dimensions(int& n_x,
                       int& n_c,
                       int& nnz_jac_c,
                       int& nnz_hes_lag) const;

   void con_bounds(double* c_l, double* c_u) const;

   void var_bounds(double* x_l, double* x_u) const;

   void eval_f(const double* x, double& f) const;

   void eval_deriv_f(const double* x, double* deriv_f) const;

   void eval_c(const double* x, double* c) const;

   void struct_jac_c(int* irow, int* jcol) const;

   void eval_jac_c(const double* x, double* jac_c_values) const;

   void struct_hes_lag(int* irow, int* jcol) const;

   // NOTE: This must be called AFTER a call to objval and conval
   // (i.e. You must call eval_f and eval_c with the same x before calling this)
   void eval_hes_lag(const double* lam_c, double* hes_lag) const;

   void primal_starting_point(double* x) const;
   // sparse version, map is cleared, then filled with only
   // user supplied values
   void primal_starting_point(std::map<int,double>& x) const;

   void dual_starting_point(double* x) const;
   // sparse version, map is cleared, then filled with only
   // user supplied values
   void dual_starting_point(std::map<int,double>& x) const;

private:

   // Default Constructor
   AmplInterface();

   // Copy Constructor
   AmplInterface(const AmplInterface&);

   // Overloaded Equals Operator
   void operator=(const AmplInterface&);

   // ASL pointer
   ASL_pfgh* asl_;
   Option_Info* Oinfo_ptr_;

   // Assumed primal starting point, when not explicitly given
   double primal_assumed_;

   // Assumed primal starting point, when not explicitly given
   double dual_assumed_;

   // stub file name
   std::string stubname_;

   // keep track if which objective is being used
   int objn_;

   // obj. sense ... -1 = maximize, 1 = minimize
   int obj_sense_;

   // number of nonzeros in the hessian
   int nnz_hes_lag_;

   // output maps to translate ASL indices to
   // variables/constraint names if .col/.row
   // files are provided
   std::map<int,std::string> rows_map;
   std::map<int,std::string> cols_map;
};

#endif
