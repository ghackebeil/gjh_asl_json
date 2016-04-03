#include <AmplInterface.hpp>

#include <iostream>
#include <fstream>

int main(int argc, char** argv)
{
   AmplInterface solver(argc, argv);

   std::string output_name = solver.stubname+".json";

   std::ofstream out;
   out.open(output_name.c_str(), std::ios::out | std::ios::trunc);
   out.precision(10);
   solver.WriteSummary(out);
   out.close();

   return 0;
}
