#include "object.hpp"

using namespace Neurology;
using namespace NeurologyTest;

ObjectTest ObjectTest::Instance;

ObjectTest::ObjectTest
(void)
   : Test()
{
}

void
ObjectTest::run
(FailVector *failures)
{
   Object<int> intObject;

   intObject = 0xDEADBEEF;
}
