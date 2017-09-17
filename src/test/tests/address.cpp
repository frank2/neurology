#include "address.hpp"

using namespace Neurology;
using namespace NeurologyTest;

AddressTest AddressTest::Instance;

AddressTest::AddressTest
(void)
   : Test()
{
}

void
AddressTest::run
(FailVector *failures)
{
   this->testAddressPool(failures);
   this->testAddress(failures);
}

void
AddressTest::testAddressPool
(FailVector *failures)
{
   AddressPool basicPool, drainagePool;
   AddressPool x86Pool(0, 0xFFFFFFFF);
   Address addressObj, compAddrObj;

   this->assertMessage(L"[*] Testing AddressPool objects.");

   NASSERT(!basicPool.hasLabel(0xDEADBEEF));
   NASSERT(!basicPool.isBound(addressObj));
   
   addressObj = basicPool.address(0xDEADBEEF);

   NASSERT(basicPool.hasLabel(0xDEADBEEF));
   NASSERT(basicPool.isBound(addressObj));
   
   basicPool.drain(drainagePool);

   NASSERT(!basicPool.isBound(addressObj));
   NASSERT(drainagePool.isBound(addressObj));

   drainagePool.drain(basicPool);

   compAddrObj = basicPool.newAddress(0xDEADBEEF);

   NASSERT(!basicPool.sharesIdentity(addressObj, compAddrObj));

   basicPool.shift(5);
   NASSERT(addressObj.label() == 0xDEADBEEF+5);

   NASSERT(x86Pool.inRange(0xDEADBEEF));
   NASSERT(x86Pool.inRange(0xFFFFFFFF));
   NASSERT(!x86Pool.inRange(0x7FFFFFFFF));

   x86Pool.setRange(0x400000, 0x401000);

   NASSERT(!x86Pool.isBound(addressObj)); // setRange should have nuked it
   NASSERT(x86Pool.size() == 0x1000);

   x86Pool.rebase(0x690000); // nice

   NASSERT(x86Pool.maximum() == 0x691000);
   NASSERT(x86Pool.size() == 0x1000);

   this->assertMessage(L"[*] AddressPool test completed.");
}

void
AddressTest::testAddress
(FailVector *failures)
{
   Address address;
   Address addressOfAddress;
   Address basicAddress = Address(0xDEADBEEFDEFACED1);
   Address targetAddress, pivotAddress;

   this->assertMessage(L"[*] Testing Address objects.");

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

   NASSERT(basicAddress == 0xDEADBEEFDEFACED1+1);

   basicAddress -= 1;

   NASSERT(basicAddress == 0xDEADBEEFDEFACED1);

   basicAddress += -1;

   NASSERT(basicAddress == 0xDEADBEEFDEFACED1-1);

   basicAddress -= -1;

   NASSERT(basicAddress == 0xDEADBEEFDEFACED1);
   
   NASSERT(basicAddress < basicAddress+1);
   NASSERT(basicAddress > basicAddress-1);
   NASSERT(basicAddress != addressOfAddress);

   NASSERT(basicAddress.pointer() == reinterpret_cast<LPVOID>(0xDEADBEEFDEFACED1));
   NASSERT(basicAddress.label() == 0xDEADBEEFDEFACED1);

   targetAddress = Address(basicAddress.label());
   pivotAddress = Address(targetAddress.label());

   NASSERT(targetAddress.sharesIdentity(pivotAddress));
   
   targetAddress.move(targetAddress.label()+8);

   NASSERT(!targetAddress.sharesIdentity(pivotAddress));
   NASSERT(targetAddress != pivotAddress);

   targetAddress = Address(basicAddress.label());
   pivotAddress = targetAddress;
   targetAddress.moveIdentity(targetAddress.label()+8);

   NASSERT(targetAddress.sharesIdentity(pivotAddress));
   NASSERT(targetAddress == pivotAddress);

   this->assertMessage(L"[*] Address test completed.");
}
