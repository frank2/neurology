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

   /* now the underlying pool should be gone */
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
   Allocation testAllocation;

   this->assertMessage(L"[*] Running Allocation tests.");
   
   NASSERT(testAllocation.isNull());
   NASSERT(!testAllocation.isBound());
   NASSERT(!testAllocation.isValid());

   /* isLocal should return false because this allocation is not bound to an allocator */
   NASSERT(!testAllocation.isLocal());

   testAllocation = LocalAllocator::Instance.null();

   NASSERT(testAllocation.allocatedFrom(&LocalAllocator::Instance));
   NASSERT(testAllocation.isLocal());

   this->assertMessage(L"[*] Finished Allocation tests.");
}
