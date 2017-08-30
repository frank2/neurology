#include "main.hpp"

using namespace std;
using namespace Neurology;

int
main
(int argc, char *argv[])
{
   LocalObject<int> intTest;
   int fuck = intTest.resolve();

   return *intTest;
}
