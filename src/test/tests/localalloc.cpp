#include "tests/localalloc.hpp"

using namespace Neurology;

void
TestLocalAllocator
(void)
{
   Allocation testAllocation;

   assert(testAllocation.isNull());
   assert(!testAllocation.isBound());
   assert(!testAllocation.isValid());

   /* isLocal should return false because this allocation is not bound to an allocator */
   assert(!testAllocation.isLocal());

   testAllocation = LocalAllocator::Instance.null();

   assert(testAllocation.allocatedFrom(&LocalAllocator::Instance));
   assert(testAllocation.isLocal());
}
