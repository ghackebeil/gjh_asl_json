#include "AmplInterface.hpp"
#include <sstream>
#include <algorithm>
#include <stdexcept>

#include "asl.h"
#include "asl_pfgh.h"
#include "getstub.h"

namespace {

void _ASSERT_(bool x) {if (!(x)) {throw std::runtime_error("Assertion Failed");}}
void _ASSERT_(bool x, std::string message) {if (!(x)) {throw std::runtime_error(message);}}

} // end local namespace

AmplInterface::AmplInterface(int argc, char**& argv)
  :
   stubname(""),
   asl_(NULL),
   objn_(0),
   nnz_hes_lag_(-1)
{
   rows_map.clear();
   cols_map.clear();

   // The ASL include files #define certain
   // variables that they expect you to work with.
   // These variables then appear as though they are
   // global variables when, in fact, they are not
   // Most of them are data members of an asl object

   // Create the ASL structure
   ASL_pfgh* asl(NULL);
   asl = (ASL_pfgh*)ASL_alloc(ASL_read_pfgh);
   _ASSERT_(asl != NULL, "ASL Error: ASL_alloc return NULL for asl struct pointer");
   asl_ = asl; // keep the pointer for ourselves to use later...

   // Read the options and stub
   Option_Info* Oinfo = new Option_Info;
   char sname[] = "gjh_asl_json";
   Oinfo->sname = new char[strlen(sname)+1];
   strcpy(Oinfo->sname, sname);

   char bsname[] = "gjh_asl_json: Version 1.0.0";
   Oinfo->bsname = new char[strlen(bsname)+1];
   strcpy(Oinfo->bsname, bsname);

   char opname[] = "gjh_asl_json_options";
   Oinfo->opname = new char[strlen(opname)+1];
   strcpy(Oinfo->opname, opname);

   int options_count = 2;
   char* rows_res(NULL);
   char* cols_res(NULL);
   keyword keywords[] = {/* must be alphabetical */
      /* one may notice that I'm going overboard here to shut up warnings
	 about 'deprecated conversion from string const to char*' */
      KW(const_cast<char*>("cols"),C_val,&cols_res,const_cast<char*>("- Map of variable names to variable ids")),
      KW(const_cast<char*>("rows"),C_val,&rows_res,const_cast<char*>("- Map of constraint names to constraint ids"))};

   Oinfo->keywds = keywords;
   Oinfo->n_keywds = options_count;
   Oinfo->flags = 0;
   Oinfo->version = NULL;
   Oinfo->usage = NULL;
   Oinfo->kwf = NULL;
   Oinfo->feq = NULL;
   Oinfo->options = NULL;
   Oinfo->n_options = 0;
   Oinfo->driver_date = 0;
   Oinfo->wantsol = 0;
   Oinfo->nS = 0;
   Oinfo->S = NULL;
   Oinfo->uinfo = NULL;
   Oinfo->asl = NULL;
   Oinfo->eqsign = NULL;
   Oinfo->n_badopts = 0;
   Oinfo->option_echo = 0;
   Oinfo->nnl = 0;

   // read the options and get the name of the .nl file (stub)
   char* stub = getstops(argv, Oinfo);

   // Try to handle most of the input suffixes commonly
   // handled by solvers. Is there a way to except any
   // suffix?  If new ones are encountered they should be
   // added. This program is only meant to summarize what
   // has been passed to an ASL solver, so we are ignoring
   // outonly suffixes.
   SufDecl suftab[] = {
         /* these are used for testing */
      {const_cast<char*>("var_int"), 0, ASL_Sufkind_var},
      {const_cast<char*>("var_real"), 0, ASL_Sufkind_var | ASL_Sufkind_real},
      {const_cast<char*>("con_int"), 0, ASL_Sufkind_con},
      {const_cast<char*>("con_real"), 0, ASL_Sufkind_con | ASL_Sufkind_real},
      {const_cast<char*>("obj_int"), 0, ASL_Sufkind_obj},
      {const_cast<char*>("obj_real"), 0, ASL_Sufkind_obj | ASL_Sufkind_real},
      {const_cast<char*>("obj_int"), 0, ASL_Sufkind_obj},
      {const_cast<char*>("obj_real"), 0, ASL_Sufkind_obj | ASL_Sufkind_real},
      {const_cast<char*>("prob_int"), 0, ASL_Sufkind_prob},
      {const_cast<char*>("prob_real"), 0, ASL_Sufkind_prob | ASL_Sufkind_real},
         /* these are commonly associated with solvers */
      {const_cast<char*>("direction"), 0, ASL_Sufkind_var},
      {const_cast<char*>("lazy"), 0, ASL_Sufkind_con},
      {const_cast<char*>("lbpen"), 0, ASL_Sufkind_var | ASL_Sufkind_real},
      {const_cast<char*>("priority"), 0, ASL_Sufkind_var},
      {const_cast<char*>("ref"), 0, ASL_Sufkind_var | ASL_Sufkind_real},
      {const_cast<char*>("rhspen"), 0, ASL_Sufkind_con | ASL_Sufkind_real},
      {const_cast<char*>("sos"), 0, ASL_Sufkind_var},
      {const_cast<char*>("sos"), 0, ASL_Sufkind_con},
      {const_cast<char*>("sosno"), 0, ASL_Sufkind_var | ASL_Sufkind_real},
      {const_cast<char*>("sosref"), 0, ASL_Sufkind_var | ASL_Sufkind_real},
      {const_cast<char*>("sstatus"), 0, ASL_Sufkind_var, 1},
      {const_cast<char*>("sstatus"), 0, ASL_Sufkind_con, 1},
      {const_cast<char*>("ubpen"), 0, ASL_Sufkind_var | ASL_Sufkind_real}};

   suf_declare(suftab, sizeof(suftab)/sizeof(SufDecl));

   _ASSERT_(stub != NULL, "ASL Error: nl filename pointer is NULL");
   stubname = std::string(stub);

   FILE* nl = NULL;
   nl = jac0dim(stub, (int)strlen(stub));
   _ASSERT_(nl != NULL, "ASL Error: jac0dim return NULL for nl file pointer");

   // tells ASL to get initial values for primal and dual
   // if available and allocate memory for X0 and pi0
   want_xpi0 = 1 | 2;
   // tells ASL to keep track of which dual and primal
   // indicies were supplied by the user
   havex0 = new char[n_var];
   std::fill_n(havex0,n_var,0);
   havepi0 = new char[n_con];
   std::fill_n(havepi0,n_con,0);

   // read the rest of the nl file accepting the most general
   // problem form
   int read_flags = ASL_return_read_err;
   // Still confused as to why this one causes a segfault.
   // Maybe keep means "keep out of memory" and NOT
   // "allocate memory for"...????
   //read_flags |= ASL_keep_all_suffixes;
   read_flags |= ASL_allow_CLP;
   read_flags |= ASL_findgroups;
   read_flags |= ASL_find_co_class;
   int retcode = pfgh_read(nl, read_flags);
   // close the stub.nl file, we are finished with it
   // calling this causes a seg fauilt on some systems
   //fclose(nl);

   switch (retcode) {
   case ASL_readerr_none : {}
      break;
   case ASL_readerr_nofile : {
      throw std::runtime_error("ASL Error: cannot open .nl file");
   }
      break;
   case ASL_readerr_nonlin : {
      throw std::runtime_error("ASL Error: model involves nonlinearities (ed0read)");
   }
      break;
   case  ASL_readerr_argerr : {
      throw std::runtime_error("ASL Error: user-defined function with bad args");
   }
      break;
   case ASL_readerr_unavail : {
      throw std::runtime_error("ASL Error: user-defined function not available");
   }
      break;
   case ASL_readerr_corrupt : {
      throw std::runtime_error("ASL Error: corrupt .nl file");
   }
      break;
   case ASL_readerr_bug : {
      throw std::runtime_error("ASL Error: bug in .nl reader");
   }
      break;
   case ASL_readerr_CLP : {
      throw std::runtime_error("ASL Error: AMPL model contains a constraint without \"=\", \">=\", or \"<=\".");
   }
      break;
   default: {
      std::ostringstream output;
      output << "ASL Error: Unknown error in stub file read. retcode = " << retcode;
      throw std::runtime_error(output.str());
   }
      break;
   }

   // get the sense of the obj function (1 for min or -1 for max)
   obj_sense_.resize(n_obj,1);
   for (int i = 0; i < n_obj; ++i) {
      if (objtype[i] != 0) {
	 obj_sense_[i] = -1;
      }
   }

   // Build the variable and constraint output maps
   // use .col/.row files if supplied as options
   if (cols_res) {
      std::ifstream in;
      in.open(cols_res, std::ios_base::in);
      if (in.is_open() && in.good()) {
	 for (int i = 0; i < n_var; ++i) {
	    in >> cols_map[i];
	 }
      }
      else {
         std::ostringstream output;
         output << "Failed to open cols file: " << std::string(cols_res);
         throw std::runtime_error(output.str());
      }
      in.close();
   }
   else {
      for (int i = 0; i < n_var; ++i) {
	 std::ostringstream stream;
	 stream << i;
	 cols_map[i] = stream.str();
      }
   }
   if (rows_res) {
      std::ifstream in;
      in.open(rows_res, std::ios_base::in);
      if (in.is_open() && in.good()) {
	 for (int i = 0; i < n_con+n_obj; ++i) {
	    in >> rows_map[i];
	 }
      }
      else {
         std::ostringstream output;
         output << "Failed to open rows file: " << std::string(rows_res);
         throw std::runtime_error(output.str());
      }
      in.close();
   }
   else {
      for (int i = 0; i < n_con; ++i) {
	 std::ostringstream stream;
	 stream << i;
	 rows_map[i] = stream.str();
      }
   }

   // delete options object
   delete [] Oinfo->sname;
   Oinfo->sname = NULL;
   delete [] Oinfo->bsname;
   Oinfo->bsname = NULL;
   delete [] Oinfo->opname;
   Oinfo->opname = NULL;
   delete Oinfo;
   Oinfo = NULL;

   set_objective(0);
}

void AmplInterface::WriteSummary(std::ostream& out)
{
   ASL_pfgh* asl = asl_;
   _ASSERT_(asl_, "Found NULL for asl struct pointer");
   int tmpx;
   int tmpc;
   int tmpnnzjacc;
   int tmpnnzheslag;
   get_nlp_dimensions(tmpx, tmpc, tmpnnzjacc, tmpnnzheslag);
   _ASSERT_(tmpx == n_var, "Number of vars does not match");
   _ASSERT_(tmpc == n_con, "Number of cons does not match");
   _ASSERT_(tmpnnzjacc == nzc, "Number nonzeros in constraints jacobian does not match");
   _ASSERT_(tmpnnzheslag == nnz_hes_lag_, "Number of nonzeros in lagrangian hessian does not match");

   out << "{" << std::endl; // JSON START
   ////////////////////////
   // PROBLEM STATISTICS //
   ////////////////////////
   out << "\"problem statistics\": {" << std::endl;
   out << "  \"no. linear network constraints\": " << lnc << "," << std::endl;
   out << "  \"no. of linear binary variables\": " << nbv << "," << std::endl;
   out << "  \"no. of linear non-binary integer variables\": " << niv << "," << std::endl;
   out << "  \"total no. of nonlinear constraints\": " << nlc << "," << std::endl;
   out << "  \"number of equality constraints or -1 if unknown (ampl prior to 19970627)\": " << n_eqn << "," << std::endl;
   out << "  \"total complementarity conditions\": " << n_cc << "," << std::endl;
   out << "  \"nonlinear complementarity conditions\": " << nlcc << "," << std::endl;
   /* ndcc is not defined */
   //out << "  \"number of complementarities involving double inequalities\": " << ndcc << "," << std::endl;
   /* nzlb is not defined */
   //out << "  \"number of complemented variables with a nonzero lower bound\": " << nzlb << "," << std::endl;
   out << "  \"no. of nonlinear network constraints\": " << nlnc << "," << std::endl;
   out << "  \"no. of nonlinear objectives\": " << nlo << "," << std::endl;
   out << "  \"no. of nonlinear variables in both constraints and objectives\": " << nlvb << "," << std::endl;
   /* nlvc and nlvo include nlvb\": "*/
   out << "  \"no. of nonlinear variables in constraints\": " << nlvc << "," << std::endl;
   out << "  \"no. of nonlinear variables in objectives\": " << nlvo << "," << std::endl;
   out << "  \"integer nonlinear variables in both constraints and objectives\": " << nlvbi << "," << std::endl;
   out << "  \"integer nonlinear vars just in constraints\": " << nlvci << "," << std::endl;
   out << "  \"integer nonlinear vars just in objectives\": " << nlvoi << "," << std::endl;
   out << "  \"no. of (linear) network variables (arcs)\": " << nwv << "," << std::endl;
   out << "  \"no. of nonzeros in constraints' Jacobian\": " << nzc << "," << std::endl;
   out << "  \"no. of nonzeros in all objective gradients\": " << nzo << "," << std::endl;
   _ASSERT_(n_obj == 1, "multiple objectives found");
   // This one is precomputed in the constructor
   out << "  \"no. of nonzeros in full lagrangian hessian\": " << nnz_hes_lag_ << "," << std::endl;
   out << "  \"total no. of variables\": " << n_var << "," << std::endl;
   out << "  \"total no. of constraints\": " << n_con << "," << std::endl;
   out << "  \"total no. of objectives\": " << n_obj << "," << std::endl;
   if (obj_sense_[objn_] == -1) {
      out << "  \"objective sense\": " << "\"maximize\"" << "," << std::endl;
   }
   else if (obj_sense_[objn_] == 1) {
      out << "  \"objective sense\": " << "\"minimize\"" << "," << std::endl;
   }
   out << "  \"no. of logical constraints\": " << n_lcon << std::endl;
   out << "}," << std::endl; // end PROBLEM STATISTICS

   /////////////////////
   // SOS CONSTRAINTS //
   /////////////////////
   // treat sos suffixes a little differently than the rest
   out << "\"sos constraints\": [" << std::endl;
   int n_sos_sets = 0;
   int nsosnz = 0;
   char *sostype(NULL);
   int *sosbeg(NULL), *sosind(NULL);
   real *sosref(NULL);
   int i = ASL_suf_sos_explict_free;
   n_sos_sets = suf_sos(i, &nsosnz, &sostype, 0, 0, &sosbeg, &sosind, &sosref);
   for(int i = 0; i < n_sos_sets; ++i) {
      out << "  {" << std::endl;
      out << "  \"type\": " << sostype[i] << "," << std::endl;
      out << "  \"vars\": [ ";
      int start = sosbeg[i];
      int stop = sosbeg[i+1];
      for (int j = start; j < stop; ++j) {
	 out << "[\"" << cols_map[sosind[j]] << "\"," << sosref[j] << "]";
	 if (j != stop-1) {out << ",";}
	 out << " ";
      }
      out << "]" << std::endl;
      out << "  }";
      if (i != n_sos_sets-1) {out << ",";}
      out << std::endl;
   }
   free(sosref);
   out << "]," << std::endl; // end SOS CONSTRAINTS

   //////////////////////////////
   // SUFFIXES //
   //////////////////////////////
   out << "\"suffixes\": {" << std::endl;
   out << "  \"variable\": {" << std::endl;
   SufDesc d;
   for (int i = 0; i < asl->i.nsuff[ASL_Sufkind_var]; ++i) {
      d = asl->i.suffixes[ASL_Sufkind_var][i];
      if (d.u.i || d.u.r) {
	 out << "    \"" << std::string(d.sufname) << "\": {" << std::endl;
	 if (d.u.i) {
	    for (int j = 0; j < n_var; ++j) {
	       if (d.u.i[j] != 0) {
		  out << "      \"" << cols_map[j] << "\": " << d.u.i[j];
		  // check if anything else will output, if so we need a comma
		  for (int k=j+1; k < n_var; ++k) {
		     if (d.u.i[k] != 0) {
			out << ",";
			break;
		     }
		  }
		  out << std::endl;
	       }
	    }
	 }
	 if (d.u.r) {
	    for (int j = 0; j < n_var; ++j) {
	       if (d.u.r[j] != 0) {
		  out << "      \"" << cols_map[j] << "\": " << d.u.r[j];
		  // check if anything else will output, if so we need a comma
		  for (int k=j+1; k < n_var; ++k) {
		     if (d.u.r[k] != 0) {
			out << ",";
			break;
		     }
		  }
		  out << std::endl;
	       }
	    }
	 }
	 out << "    }";
	 // check if anything else will output, if so we need a comma
	 for (int k=i+1; k < asl->i.nsuff[ASL_Sufkind_var]; ++k) {
	    d = asl->i.suffixes[ASL_Sufkind_var][k];
	    if (d.u.i || d.u.r) {
	       out << ",";
	       break;
	    }
	 }
	 out << std::endl;
      }
   }
   out << "  }," << std::endl; // end variable

   out << "  \"constraint\": {" << std::endl;
   for (int i = 0; i < asl->i.nsuff[ASL_Sufkind_con]; ++i) {
      d = asl->i.suffixes[ASL_Sufkind_con][i];
      if (d.u.i || d.u.r) {
	 out << "    \"" << std::string(d.sufname) << "\": {" << std::endl;
	 if (d.u.i) {
	    for (int j = 0; j < n_con; ++j) {
	       if (d.u.i[j] != 0) {
		  out << "      \"" << rows_map[j] << "\": " << d.u.i[j];
		  // check if anything else will output, if so we need a comma
		  for (int k=j+1; k < n_con; ++k) {
		     if (d.u.i[k] != 0) {
			out << ",";
			break;
		     }
		  }
		  out << std::endl;
	       }
	    }
	 }
	 if (d.u.r) {
	    for (int j = 0; j < n_con; ++j) {
	       if (d.u.r[j] != 0) {
		  out << "      \"" << rows_map[j] << "\": " << d.u.r[j];
		  // check if anything else will output, if so we need a comma
		  for (int k=j+1; k < n_con; ++k) {
		     if (d.u.r[k] != 0) {
			out << ",";
			break;
		     }
		  }
		  out << std::endl;
	       }
	    }
	 }
	 out << "    }";
	 // check if anything else will output, if so we need a comma
	 for (int k=i+1; k < asl->i.nsuff[ASL_Sufkind_con]; ++k) {
	    d = asl->i.suffixes[ASL_Sufkind_con][k];
	    if (d.u.i || d.u.r) {
	       out << ",";
	       break;
	    }
	 }
	 out << std::endl;
      }
   }
   out << "  }," << std::endl; // end constraint


   out << "  \"objective\": {" << std::endl;
   for (int i = 0; i < asl->i.nsuff[ASL_Sufkind_obj]; ++i) {
      d = asl->i.suffixes[ASL_Sufkind_obj][i];
      if (d.u.i || d.u.r) {
	 out << "    \"" << std::string(d.sufname) << "\": {" << std::endl;
	 if (d.u.i) {
	    for (int j = 0; j < n_obj; ++j) {
	       if (d.u.i[j] != 0) {
		  out << "      \"" << j << "\": " << d.u.i[j];
		  // check if anything else will output, if so we need a comma
		  for (int k=j+1; k < n_obj; ++k) {
		     if (d.u.i[k] != 0) {
			out << ",";
			break;
		     }
		  }
		  out << std::endl;
	       }
	    }
	 }
	 if (d.u.r) {
	    for (int j = 0; j < n_obj; ++j) {
	       if (d.u.r[j] != 0) {
		  out << "      \"" << j << "\": " << d.u.r[j];
		  // check if anything else will output, if so we need a comma
		  for (int k=j+1; k < n_obj; ++k) {
		     if (d.u.r[k] != 0) {
			out << ",";
			break;
		     }
		  }
		  out << std::endl;
	       }
	    }
	 }
	 out << "    }";
	 // check if anything else will output, if so we need a comma
	 for (int k=i+1; k < asl->i.nsuff[ASL_Sufkind_obj]; ++k) {
	    d = asl->i.suffixes[ASL_Sufkind_obj][k];
	    if (d.u.i || d.u.r) {
	       out << ",";
	       break;
	    }
	 }
	 out << std::endl;
      }
   }
   out << "  }," << std::endl; // end obj

   out << "  \"problem\": {" << std::endl;
   for (int i = 0; i < asl->i.nsuff[ASL_Sufkind_prob]; ++i) {
      d = asl->i.suffixes[ASL_Sufkind_prob][i];
      if (d.u.i || d.u.r) {
	 out << "    \"" << std::string(d.sufname) << "\": {" << std::endl;
	 if (d.u.i) {
	    for (int j = 0; j < asl->i.n_prob; ++j) {
	       if (d.u.i[j] != 0) {
		  out << "      \"" << j << "\": " << d.u.i[j];
		  // check if anything else will output, if so we need a comma
		  for (int k=j+1; k < asl->i.n_prob; ++k) {
		     if (d.u.i[k] != 0) {
			out << ",";
			break;
		     }
		  }
		  out << std::endl;
	       }
	    }
	 }
	 if (d.u.r) {
	    for (int j = 0; j < asl->i.n_prob; ++j) {
	       if (d.u.r[j] != 0) {
		  out << "      \"" << j << "\": " << d.u.r[j];
		  // check if anything else will output, if so we need a comma
		  for (int k=j+1; k < asl->i.n_prob; ++k) {
		     if (d.u.r[k] != 0) {
			out << ",";
			break;
		     }
		  }
		  out << std::endl;
	       }
	    }
	 }
	 out << "    }";
	 // check if anything else will output, if so we need a comma
	 for (int k=i+1; k < asl->i.nsuff[ASL_Sufkind_prob]; ++k) {
	    d = asl->i.suffixes[ASL_Sufkind_prob][k];
	    if (d.u.i || d.u.r) {
	       out << ",";
	       break;
	    }
	 }
	 out << std::endl;
      }
   }
   out << "  }" << std::endl; // end problem
   out << "}," << std::endl; // end SUFFIXES

   //////////////////////////////
   // SUPPLIED STARTING POINTS //
   //////////////////////////////
   out << "\"supplied starting points\": {" << std::endl;
   std::map<int,double> start_primal_sparse, start_dual_sparse;
   get_primal_starting_point(start_primal_sparse);
   get_dual_starting_point(start_dual_sparse);
   out << "  \"primal\": {" << std::endl;
   for (std::map<int,double>::iterator pos = start_primal_sparse.begin(),
           pos_stop = start_primal_sparse.end();
        pos != pos_stop; /*++inside loop*/) {
      out << "    \"" << cols_map[pos->first] << "\": " << pos->second;
      if (++pos != pos_stop) {out << ",";}
      out << std::endl;
   }
   out << "  }," << std::endl; // end primal
   out << "  \"dual\": {" << std::endl;
   for (std::map<int,double>::iterator pos = start_dual_sparse.begin(),
           pos_stop = start_dual_sparse.end();
        pos != pos_stop; /*++ inside loop*/) {
      out << "    \"" << rows_map[pos->first] << "\": " << pos->second;
      if (++pos != pos_stop) {out << ",";}
      out << std::endl;
   }
   out << "  }" << std::endl; // end dual
   out << "}," << std::endl; // end SUPPLIED STARTING POINTS

   /////////////////////////////
   // ASSUMED STARTING POINTS //
   /////////////////////////////
   out << "\"assumed starting points\": {" << std::endl;
   double primal_assumed = 1.0;
   double dual_assumed = 1.0;
   std::vector<double> start_primal_dense(n_var,primal_assumed);
   std::vector<double> start_dual_dense(n_con,dual_assumed);
   get_primal_starting_point(&(start_primal_dense[0]));
   get_dual_starting_point(&(start_dual_dense[0]));
   out << "  \"primal\": " << primal_assumed << "," << std::endl;
   out << "  \"dual\": " << dual_assumed << std::endl;
   out << "}," << std::endl; // end ASSUMED STARTING POINTS

   /////////////////////
   // VARIABLE BOUNDS //
   /////////////////////
   out << "\"variable bounds\": {" << std::endl;
   std::vector<double> XL(n_var), XU(n_var);
   get_var_bounds(&(XL[0]),&(XU[0]));
   for (int i = 0; i < n_var; ++i) {
      std::ostringstream XLs;
      std::ostringstream XUs;
      if (XU[i] == negInfinity) {XUs << "-Infinity";} // JSON
      else if (XU[i] == Infinity) {XUs << "Infinity";} // JSON
      else {XUs << XU[i];}
      if (XL[i] == negInfinity) {XLs << "-Infinity";} // JSON
      else if (XL[i] == Infinity) {XLs << "Infinity";} // JSON
      else {XLs << XL[i];}
      out << "  \"" << cols_map[i] << "\": [" << XLs.str() << "," << XUs.str() << "]";
      if (i < n_var-1) {out << ",";}
      out << std::endl;
   }
   out << "}," << std::endl; // end VARIABLE BOUNDS

   ///////////////////////
   // CONSTRAINT BOUNDS //
   ///////////////////////
   out << "\"constraint bounds\": {" << std::endl;
   std::vector<double> CL(n_con), CU(n_con);
   get_con_bounds(&(CL[0]),&(CU[0]));
   for (int i = 0; i < n_con; ++i) {
      std::ostringstream CLs;
      std::ostringstream CUs;
      if (CU[i] == negInfinity) {CUs << "-Infinity";} // JSON
      else if (CU[i] == Infinity) {CUs << "Infinity";} // JSON
      else {CUs << CU[i];}
      if (CL[i] == negInfinity) {CLs << "-Infinity";} // JSON
      else if (CL[i] == Infinity) {CLs << "Infinity";} // JSON
      else {CLs << CL[i];}
      out << "  \"" << rows_map[i] << "\": [" << CLs.str() << "," << CUs.str() << "]";
      if (i < n_con-1) {out << ",";}
      out << std::endl;
   }
   out << "}," << std::endl; // end CONSTRAINT BOUNDS

   /////////////////////////
   // INITIAL EVALUATIONS //
   /////////////////////////
   out << "\"initial evaluations\": {" << std::endl;
   // objective
   double F = 0.0;
   eval_f(&(start_primal_dense[0]),F);
   out << "  \"objective function\": " << F << "," << std::endl;
   // derivative of objective
   std::vector<double> deriv_F(n_var);
   eval_deriv_f(&(start_primal_dense[0]),&(deriv_F[0]));
   out << "  \"objective gradient\": {" << std::endl;
   for (int i = 0; i < n_var; ++i) {
      out << "    \"" << cols_map[i] << "\": " << deriv_F[i];
      if (i < n_var-1) {out << ",";}
      out << std::endl;
   }
   out << "  }," << std::endl; // end objective gradient

   // constraints
   std::vector<double> con_eval(n_con);
   eval_c(&(start_primal_dense[0]),&(con_eval[0]));
   out << "  \"constraints\": {" << std::endl;
   for (int i = 0; i < n_con; ++i) {
      out << "    \"" << rows_map[i] << "\": " << con_eval[i];
      if (i < n_con-1) {out << ",";}
      out << std::endl;
   }
   out << "  }," << std::endl; // end constraints

   // jacobian of constraints
   std::vector<int> jac_irow(nzc), jac_jcol(nzc);
   struct_jac_c(&(jac_irow[0]),&(jac_jcol[0]));
   std::vector<double> jac_vals(nzc);
   eval_jac_c(&(start_primal_dense[0]),&(jac_vals[0]));
   out << "  \"constraints' jacobian\": {" << std::endl;
   for (int i = 0; i < nzc; ++i) {
      out << "    \"" << rows_map[jac_irow[i]] << "_" << cols_map[jac_jcol[i]] << "\": " << jac_vals[i];
      if (i < nzc-1) {out << ",";}
      out << std::endl;
   }
   out << "  }," << std::endl; // end constraints' jacobian

   // lagrangian hessian
   out << "  \"lagrangian hessian\": {" << std::endl;
   if (nnz_hes_lag_ > 0) {
      std::vector<int> hlag_irow(nnz_hes_lag_), hlag_jcol(nnz_hes_lag_);
      struct_hes_lag(&(hlag_irow[0]),&(hlag_jcol[0]));
      std::vector<double> hlag_vals(nnz_hes_lag_);
      eval_hes_lag(&(start_dual_dense[0]),&(hlag_vals[0]));
      for (int i = 0; i < nnz_hes_lag_; ++i) {
	 out << "    \"" << cols_map[hlag_irow[i]] << "_" << cols_map[hlag_jcol[i]] << "\": " << hlag_vals[i];
	 if (i < nnz_hes_lag_-1) {out << ",";}
	 out << std::endl;
      }
   }
   out << "  }" << std::endl; // end lagrangian hessian
   out << "}" << std::endl; // end INITIAL EVALUATIONS
   out << "}" << std::endl; // JSON STOP
}

AmplInterface::~AmplInterface()
{
   ASL_pfgh* asl = asl_;

   delete [] havex0;
   havex0 = NULL;
   delete [] havepi0;
   havepi0 = NULL;

   if (asl) {
     ASL* asl_to_free = (ASL*)asl_;
     ASL_free(&asl_to_free);
     asl_ = NULL;
   }

   // output maps
   rows_map.clear();
   cols_map.clear();

   stubname = std::string("");
}

void AmplInterface::set_objective(int objective_number) {
   ASL_pfgh* asl = asl_;
   _ASSERT_(asl_, "Found NULL for asl struct pointer");
   _ASSERT_(objective_number >= 0, "objective id must be non-negative when calling set_objective");
   _ASSERT_(objective_number < n_obj, "found invalid objective id when calling set_objective");

   objn_ = objective_number;

   int mult_supplied = 1; // multipliers will be supplied
   // I specifically ask for the entire matrix (even though it is
   // symmetric) so that we can compare AMPL and Pyomo. Because of
   // differences in the variable ordering, we can have that dxdy ends up
   // in the upper triangle for AMPL nl files whereas dydx ends up in the
   // upper triangle for Pyomo nl files. Obtaining the entire matrix
   // avoids this complication.
   int uptri = 0; // use the full sparse matrix
   nnz_hes_lag_ = sphsetup(objn_, NULL, mult_supplied, uptri);
}

void AmplInterface::get_nlp_dimensions(int& n_x, int& n_c, int& nnz_jac_c, int& nnz_hes_lag) const
{
   ASL_pfgh* asl = asl_;
   _ASSERT_(asl_, "Found NULL for asl struct pointer");

   n_x = n_var; // # of variables (variable types have been asserted in the constructor
   n_c = n_con; // # of constraints
   nnz_jac_c = nzc; // # of non-zeros in the jacobian
   nnz_hes_lag = nnz_hes_lag_; // # of non-zeros in the hessian
}

void AmplInterface::get_con_bounds(double* c_l, double* c_u) const
{
   ASL_pfgh* asl = asl_;
   _ASSERT_(asl_, "Found NULL for asl struct pointer");
   if (n_con == 0) {return ;} // unconstrained problem
   _ASSERT_(c_l && c_u, "One or more arguments to get_con_bounds is NULL");

   for (int i = 0; i < n_con; ++i) {
      c_l[i] = LUrhs[2*i];
      c_u[i] = LUrhs[2*i+1];
   }
}

void AmplInterface::get_var_bounds(double* x_l, double* x_u) const
{
   ASL_pfgh* asl = asl_;
   _ASSERT_(asl_, "Found NULL for asl struct pointer");
   _ASSERT_(x_l && x_u, "One or more arguments to get_var_bounds is NULL");

   for (int i = 0; i < n_var; ++i) {
      x_l[i] = LUv[2*i];
      x_u[i] = LUv[2*i+1];
   }
}

void AmplInterface::eval_f(const double* x, double& f) const
{
   ASL_pfgh* asl = asl_;
   _ASSERT_(asl_, "Found NULL for asl struct pointer");
   _ASSERT_(n_obj == 1 && "AMPL problem must have a single objective function");
   _ASSERT_(x, "First argument to eval_f is NULL");

   int nerror = 1;
   f = objval(obj_no, (double*)x, &nerror);
   if (nerror != 0) {
      throw std::runtime_error("ASL Error: Problem evaluating objective.");
   }
}

void AmplInterface::eval_deriv_f(const double* x, double* deriv_f) const
{
   ASL_pfgh* asl = asl_;
   _ASSERT_(asl_, "Found NULL for asl struct pointer");
   _ASSERT_(n_obj == 1 && "AMPL problem must have a single objective function.");
   _ASSERT_(x && deriv_f, "One or more arguments to eval_deriv_f is NULL");

   int nerror = 1;
   objgrd(obj_no, (double*)x, deriv_f, &nerror);
   if (nerror != 0) {
      throw std::runtime_error("ASL Error: Problem evaluating objective gradient.");
   }
}

void AmplInterface::eval_c(const double* x, double* c) const
{
   ASL_pfgh* asl = asl_;
   _ASSERT_(asl_, "Found NULL for asl struct pointer");
   if (n_con == 0) {return ;} // unconstrained problem
   _ASSERT_(x && c, "One or more arguments to eval_c is NULL");

   // call AMPL to evaluate the constraints
   int nerror = 1;
   conval((double*)x, c, &nerror);
   if (nerror != 0) {
      throw std::runtime_error("ASL Error: Problem evaluating constraints.");
   }
}

void AmplInterface::struct_jac_c(int* irow, int* jcol) const
{
   ASL_pfgh* asl = asl_;
   _ASSERT_(asl_, "Found NULL for asl struct pointer");
   if (n_con == 0) {return ;} // unconstrained problem
   _ASSERT_(irow && jcol, "One or more arguments to struct_jac_c is NULL");

   // get the non zero structure of the jacobian of c
   int current_nz = 0;
   for (int i = 0; i < n_con; ++i) {
      for (cgrad* cg=Cgrad[i]; cg; cg = cg->next) {
	 irow[cg->goff] = i;
	 jcol[cg->goff] = cg->varno;
	 ++current_nz;
      }
   }
   _ASSERT_(current_nz == nzc, "Problem evaluating struct_jac_c");
}

void AmplInterface::eval_jac_c(const double* x, double* jac_c_values) const
{
   ASL_pfgh* asl = asl_;
   _ASSERT_(asl_, "Found NULL for asl struct pointer");
   if (n_con == 0) {return ;} // unconstrained problem
   _ASSERT_(x && jac_c_values);

   int nerror=1;
   jacval((double*)x, jac_c_values, &nerror);
   if (nerror != 0) {
      throw std::runtime_error("ASL Error: Problem evaluating constraints' jacobian.");
   }
}

void AmplInterface::struct_hes_lag(int* irow, int* jcol) const
{
   ASL_pfgh* asl = asl_;
   _ASSERT_(asl_, "Found NULL for asl struct pointer");
   _ASSERT_(n_obj == 1 && "AMPL problem must have a single objective function");
   _ASSERT_(irow && jcol, "One or more arguments to struct_jac_c is NULL");

   // setup the structure
   int k = 0;
   for (int i = 0; i < n_var; ++i) {
      for (int j=sputinfo->hcolstarts[i]; j<sputinfo->hcolstarts[i+1]; j++) {
	 irow[k] = i;
	 jcol[k] = sputinfo->hrownos[j];
	 k++;
      }
   }
   _ASSERT_(k == nnz_hes_lag_);
}

void AmplInterface::eval_hes_lag(const double* lam_c, double* hes_lag) const
{
   ASL_pfgh* asl = asl_;
   _ASSERT_(asl_, "Found NULL for asl struct pointer");
   _ASSERT_(n_obj == 1, "AMPL problem must have a single objective function");
   if (n_con == 0) { // unconstrained problem
      _ASSERT_(hes_lag, "The second argument to eval_hes_lag is NULL");
      // NOTE: This must be called AFTER a call to objval and conval
      // (i.e. You must call eval_f and eval_c with the same x before calling this)
      sphes(hes_lag, objn_ , NULL, NULL);
   }
   else {
      _ASSERT_(lam_c && hes_lag, "One or more arguments to eval_hes_lag is NULL");
      // NOTE: This must be called AFTER a call to objval and conval
      // (i.e. You must call eval_f and eval_c with the same x before calling this)
      sphes(hes_lag, objn_ , NULL, const_cast<double*>(lam_c));
   }
}

void AmplInterface::get_dual_starting_point(double* x) const
{
   ASL_pfgh* asl = asl_;
   _ASSERT_(asl_, "Found NULL for asl struct pointer");
   if (n_con == 0) {return ;} // unconstrained problem
   _ASSERT_(x, "Argument of get_dual_starting_point is NULL");

   for (int i = 0; i < n_con; ++i) {
      if (havepi0[i]) {
	 x[i] = pi0[i];
      }
   }
}

void AmplInterface::get_dual_starting_point(std::map<int,double>& x) const
{
   ASL_pfgh* asl = asl_;
   _ASSERT_(asl_, "Found NULL for asl struct pointer");
   x.clear();
   if (n_con == 0) {return ;} // unconstrained problem

   for (int i = 0; i < n_con; ++i) {
      if (havepi0[i]) {
	 x[i] = pi0[i];
      }
   }
}


void AmplInterface::get_primal_starting_point(double* x) const
{
   ASL_pfgh* asl = asl_;
   _ASSERT_(asl_, "Found NULL for asl struct pointer");
   _ASSERT_(x, "Argument of get_primal_starting_point is NULL");

   for (int i = 0; i < n_var; ++i) {
      if (havex0[i]) {
	 x[i] = X0[i];
      }
   }
}

void AmplInterface::get_primal_starting_point(std::map<int,double>& x) const
{
   ASL_pfgh* asl = asl_;
   _ASSERT_(asl_, "Found NULL for asl struct pointer");
   x.clear();

   for (int i = 0; i < n_var; ++i) {
      if (havex0[i]) {
	 x[i] = X0[i];
      }
   }
}
