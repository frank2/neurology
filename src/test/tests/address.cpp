#include "address.hpp"

using namespace Neurology;
using namespace NeurologyTest;

AddressTest::AddressTest
(void)
   : Test()
{
}

void
AddressTest::run
(std::vector<TestFailure> *failures)
{
   Address address;
   Address addressOfAddress;
   Address basicAddress = Address(0xDEADBEEFDEFACED1);

   NASSERT(address.isNull());
   NASSERT(!address.hasPool()); // has no pool yet
   NASSERT(!address.inRange()); // not in any range because it has no pool;

   addressOfAddress = Address(&address);

   NASSERT(addressOfAddress.label() == (Label)&address);
   NASSERT(!addressOfAddress.isNull());
   NASSERT(addressOfAddress.hasPool()); // has a pool because it's bound to the default AddressPool
   NASSERT(addressOfAddress.usesPool(&AddressPool::Instance));
   NASSERT(addressOfAddress.inRange()); // by default it should be in range because the default range is all possible addresses

   NASSERT(basicAddress+1 == basicAddress.label()+1);
   NASSERT(basicAddress-1 == basicAddress.label()-1);
   NASSERT(basicAddress+(-1) == basicAddress.label()-1);
   NASSERT(basicAddress-(-1) == basicAddress.label()+1);

   basicAddress += 1;

   NASSERT(basicAddress == 0xDEADBEEFDEFACED1 + 1);

   basicAddress -= 1;

   NASSERT(basicAddress == 0xDEADBEEFDEFACED1);

   basicAddress += -1;

   NASSERT(basicAddress == 0xDEADBEEFDEFACED1 - 1);

   basicAddress -= -1;

   NASSERT(basicAddress == 0xDEADBEEFDEFACED1);
   
   NASSERT(basicAddress < basicAddress+1);
   NASSERT(basicAddress > basicAddress-1);
   NASSERT(basicAddress != addressOfAddress);
}
