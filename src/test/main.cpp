#include "main.hpp"

using namespace std;
using namespace Neurology;

int
main
(int argc, char *argv[])
{
   LocalObject<int> intTest = LocalObject<int>::New(0);
   intTest = 69; // nice
   return *intTest;
}
