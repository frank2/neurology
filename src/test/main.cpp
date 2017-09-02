#include "main.hpp"

using namespace std;
using namespace Neurology;

int
main
(int argc, char *argv[])
{
   int *testInt = new int;
   Address testAddress = Address(testInt);

   *testAddress.cast<int>() = 420; // nice

   std::cout << *testInt << std::endl;

   return 0;
}
