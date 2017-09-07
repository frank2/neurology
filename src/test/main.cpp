#include "main.hpp"

using namespace Neurology;

int
main
(int argc, char *argv[])
{
   Object<int> intObject;

   intObject = 69; // nice

   std::cout << *intObject << std::endl;

   return 0;
}
