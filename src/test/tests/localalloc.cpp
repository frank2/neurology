#include "localalloc.hpp"

using namespace Neurology;
using namespace NeurologyTest;

LocalAllocatorTest LocalAllocatorTest::Instance;

LocalAllocatorTest::LocalAllocatorTest
(void)
   : Test()
{
}

void
LocalAllocatorTest::run
(FailVector *failures)
{
   this->testAllocator(failures);
   this->testAllocation(failures);
}

void
LocalAllocatorTest::testAllocator
(FailVector *failures)
{
   LocalAllocator allocator;
   Allocation allocation, otherAlloc;
   Address allocAddress, cmpAddress;
   std::uintptr_t uintptr = 0xDEADBEEFDEFACED1;
   Data writeData, recvData;

   this->assertMessage(L"[*] Running Allocator tests.");

   writeData = VarData(uintptr);

   /* local allocator shouldn't split-- it's simply a shell for the C++ heap
      operators */
   NASSERT(!allocator.splits());

   /* local allocator is literally local my guy, my lady, my gender-neutral
      coder du jour */
   NASSERT(allocator.isLocal());
   
   NASSERT(!allocator.isPooled(&allocator));
   NASSERT(!allocator.isAssociated(allocation));
   NASSERT(!allocator.isBound(allocation));
   NASSERT(!allocator.hasAddress(&allocator));
   NASSERT(!allocator.hasParent(allocation));
   NASSERT(!allocator.hasChildren(allocation));

   allocation = allocator.allocate<std::uintptr_t>();
   allocAddress = Address(allocator.address(allocation).label());

   NASSERT(allocAddress == allocation.address());

   NASSERT(allocator.isPooled(allocAddress));
   NASSERT(allocator.isAssociated(allocation));
   NASSERT(allocator.isBound(allocation));
   NASSERT(allocator.hasAddress(allocAddress));
   NASSERT(allocator.hasAddress(allocAddress+4));
   NASSERT(!allocator.hasParent(allocation));
   NASSERT(!allocator.hasChildren(allocation));

   NASSERT(allocator.addressOf(allocation) == allocAddress);
   NASSERT(allocator.address(allocation, 4) == allocAddress+4);

   cmpAddress = allocator.newAddress(allocation);

   NASSERT(allocAddress == cmpAddress);
   NASSERT(!allocAddress.sharesIdentifier(cmpAddress));

   NASSERT(allocator.bindCount(allocAddress) == 1);

   otherAlloc = allocation;

   NASSERT(allocator.bindCount(allocAddress) == 2);
   NASSERT(allocator.querySize(allocation) == sizeof(std::uintptr_t));

   allocator.reallocate(allocation, sizeof(std::uintptr_t)*4);

   NASSERT(allocator.querySize(allocation) == sizeof(std::uintptr_t)*4);
   NASSERT(allocator.querySize(otherAlloc) == sizeof(std::uintptr_t)*4);

   allocAddress = Address(allocation.address().label());
   allocator.write(allocAddress, writeData);
   recvData = allocator.read(allocAddress, sizeof(std::uintptr_t));
   
   NASSERT(writeData == recvData);

   /* should still technically be around in some ephemeral way because we have
      otherAlloc pointing to the underlying allocation */
   allocator.deallocate(allocation);

   NASSERT(allocator.isPooled(allocAddress));
   NASSERT(!allocator.isAssociated(allocation));
   NASSERT(!allocator.isBound(allocation));
   NASSERT(allocator.hasAddress(allocAddress));
   NASSERT(allocator.hasAddress(allocAddress+4));
   NASSERT(!allocator.hasParent(allocation));
   NASSERT(!allocator.hasChildren(allocation));

   /* now the underlying pool should go away, leaving the
      allocation copy null */
   allocator.deallocate(otherAlloc);

   NASSERT(!allocator.isPooled(allocAddress));
   NASSERT(!allocator.isAssociated(allocation));
   NASSERT(!allocator.isBound(allocation));
   NASSERT(!allocator.hasAddress(allocAddress));
   NASSERT(!allocator.hasAddress(allocAddress+4));
   NASSERT(!allocator.hasParent(allocation));
   NASSERT(!allocator.hasChildren(allocation));
   
   this->assertMessage(L"[*] Finished Allocator tests.");
}

void
LocalAllocatorTest::testAllocation
(FailVector *failures)
{
   Allocation testAllocation, clonedAllocation, slicedAllocation, superSliced;
   Allocator *allocator = &LocalAllocator::Instance;
   std::uintptr_t uintptr = 0xDEADBEEFDEFACED1;
   std::uint32_t uint32 = 0xDEADBEEF;
   std::uint16_t uint16;
   Data sendData, recvData;

   this->assertMessage(L"[*] Running Allocation tests.");
   
   NASSERT(testAllocation.isNull());
   NASSERT(!testAllocation.isBound());
   NASSERT(!testAllocation.isValid());
   NASSERT(!testAllocation.isLocal()); // not bound to anything, can't be local
   // never allocated from anything, frankly
   NASSERT(!testAllocation.allocatedFrom(allocator));
   // can't be in range of nothing.
   NASSERT(!testAllocation.inRange(0));

   testAllocation = allocator->null();
   
   NASSERT(testAllocation.isNull());
   NASSERT(!testAllocation.isBound());
   NASSERT(!testAllocation.isValid());
   NASSERT(testAllocation.isLocal());
   NASSERT(testAllocation.allocatedFrom(allocator));
   NASSERT(!testAllocation.inRange(0));

   NASSERT(testAllocation.size() == 0);

   testAllocation.allocate(sizeof(std::uintptr_t));

   NASSERT(!testAllocation.isNull());
   NASSERT(testAllocation.isBound());
   NASSERT(testAllocation.isValid());
   NASSERT(testAllocation.allocatedFrom(allocator));
   
   NASSERT(testAllocation.inRange(0));
   NASSERT(!testAllocation.inRange(sizeof(std::uintptr_t)));
   NASSERT(testAllocation.inRange(0, sizeof(std::uintptr_t)));
   NASSERT(!testAllocation.inRange(1, sizeof(std::uintptr_t)));
   NASSERT(testAllocation.inRange(testAllocation.address()+(sizeof(std::uintptr_t)/2)));
   NASSERT(testAllocation.inRange(testAllocation.address(), sizeof(std::uintptr_t)));
   NASSERT(!testAllocation.inRange(testAllocation.address()+1, sizeof(std::uintptr_t)));
   NASSERT(!testAllocation.inRange(testAllocation.address()+sizeof(std::uintptr_t), 1));
   
   NASSERT(testAllocation.end() == testAllocation.start()+sizeof(std::uintptr_t));
   NASSERT(testAllocation.offset(testAllocation.baseAddress()) == 0);
   NASSERT(testAllocation.offset(testAllocation.address()+4) == 4);

   NASSERT(testAllocation.size() == sizeof(std::uintptr_t));

   sendData = VarData(uintptr);
   testAllocation.write(sendData);
   recvData = testAllocation.read();

   NASSERT(sendData == recvData);

   sendData = VarData(uint32);
   
   NASSERT(sendData != testAllocation.read(4));
   NASSERT(sendData == testAllocation.read(4,4));
   NASSERT(sendData != testAllocation.read(testAllocation.address(), 4));
   NASSERT(sendData == testAllocation.read(testAllocation.address()+4, 4));

   testAllocation.write(sendData);
   
   NASSERT(sendData == testAllocation.read(4));
   NASSERT(sendData == testAllocation.read(4,4));
   NASSERT(sendData == testAllocation.read(testAllocation.address(), 4));
   NASSERT(sendData == testAllocation.read(testAllocation.address()+4, 4));

   uint32 = 0xDEFACED1;
   sendData = VarData(uint32);

   testAllocation.write(4, sendData);
   
   NASSERT(sendData != testAllocation.read(4));
   NASSERT(sendData == testAllocation.read(4,4));
   NASSERT(sendData != testAllocation.read(testAllocation.address(), 4));
   NASSERT(sendData == testAllocation.read(testAllocation.address()+4, 4));

   testAllocation.write(testAllocation.address(), sendData);

   NASSERT(sendData == testAllocation.read(4));
   NASSERT(sendData == testAllocation.read(4,4));
   NASSERT(sendData == testAllocation.read(testAllocation.address(), 4));
   NASSERT(sendData == testAllocation.read(testAllocation.address()+4, 4));

   clonedAllocation.clone(testAllocation);

   NASSERT(!allocator->sharesPool(testAllocation, clonedAllocation));
   NASSERT(!clonedAllocation.isChild(testAllocation));
   NASSERT(!clonedAllocation.isParent(testAllocation));
   NASSERT(!testAllocation.isParent(clonedAllocation));
   NASSERT(!testAllocation.isChild(clonedAllocation));
   NASSERT(sendData == clonedAllocation.read(4));
   NASSERT(sendData == clonedAllocation.read(4,4));
   NASSERT(sendData == clonedAllocation.read(clonedAllocation.address(), 4));
   NASSERT(sendData == clonedAllocation.read(clonedAllocation.address()+4, 4));

   slicedAllocation = testAllocation.slice(testAllocation.address()+2, sizeof(uint32_t));
   uint32 = 0xCED1DEFA; // 0xDEFA CED1DEFA CED1
   sendData = VarData(uint32);

   NASSERT(allocator->sharesPool(testAllocation, slicedAllocation));
   NASSERT(slicedAllocation.isChild(testAllocation));
   NASSERT(!slicedAllocation.isParent(testAllocation));
   NASSERT(testAllocation.isParent(slicedAllocation));
   NASSERT(!testAllocation.isChild(slicedAllocation));
   NASSERT(sendData == slicedAllocation.read());
   NASSERT(slicedAllocation.size() != testAllocation.size());
   NASSERT(slicedAllocation.end() != testAllocation.end());

   uint32 = 0xDEADBEEF;
   sendData = VarData(uint32);
   slicedAllocation.write(sendData);

   NASSERT(sendData == slicedAllocation.read());

   uintptr = 0xDEFACED1DEFACED1;
   sendData = VarData(uintptr);

   NASSERT(sendData != testAllocation.read());

   uintptr = 0xDEFADEADBEEFCED1; // 0xDEFA DEADBEEF CED1
   sendData = VarData(uintptr);

   NASSERT(sendData == testAllocation.read());

   superSliced = slicedAllocation.slice(slicedAllocation.address()+1, sizeof(uint16_t));

   this->assertMessage(L"[*] Finished Allocation tests.");
}
