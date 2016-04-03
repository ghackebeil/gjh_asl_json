#include <AmplInterface.hpp>

#include <iostream>
#include <fstream>
#include <exception>

bool endswith(std::string full,
              std::string ending) {
   if (full.length() >= ending.length()) {
      return (0 == full.compare(full.length() - ending.length(),
                                ending.length(),
                                ending));
   }
   else {
      return false;
   }
}

int main(int argc, char** argv)
{
   AmplInterface solver(argc, argv);

   std::string output_name;
   if (endswith(solver.stubname, std::string(".nl"))) {
      output_name = solver.stubname;
      output_name[solver.stubname.size()-2] = 'j';
      output_name[solver.stubname.size()-2] = 's';
      output_name += "on";
   }
   else {
      output_name = solver.stubname+".json";
   }

   std::ofstream out;
   out.open(output_name.c_str(), std::ios::out | std::ios::trunc);
   out.precision(10);
   try {
      solver.WriteSummary(out);
   }
   catch (const std::exception& e) {
      std::cerr << std::endl;
      std::cerr << "Exception: " << e.what() << std::endl;
      std::cerr << std::endl;
      out.close();
      return 1;
   }
   out.close();

   return 0;
}
