#include <AmplInterface.hpp>

#include <iostream>
#include <fstream>

int main(int argc, char** argv)
{
   AmplInterface solver(argc, argv);

   std::string output_name = solver.get_stubname()+".json";

   std::ofstream out;
   out.open(output_name.c_str(), std::ios::out | std::ios::trunc);
   out.precision(15);
   solver.write_json_summary(out);
   out.close();

   solver.set_current_objective(0);
   solver.write_solution_file();

   return 0;
}
