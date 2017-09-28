#include "virtualalloc.hpp"

using namespace Neurology;
using namespace NeurologyTest;

VirtualAllocatorTest VirtualAllocatorTest::Instance;

VirtualAllocatorTest::VirtualAllocatorTest
(void)
   : Test()
{
}

void
VirtualAllocatorTest::run
(FailVector *failures)
{
   this->testAllocator(failures);
   this->testPage(failures);
}

void
VirtualAllocatorTest::testAllocator
(FailVector *failures)
{
   VirtualAllocator allocator;

   NASSERT(allocator.isLocal());

   allocator.enumerate();
   
   this->assertMessage(L"[*] Finished VirtualAllocator tests.");
}

void
VirtualAllocatorTest::testPage
(FailVector *failures)
{
   return;
}
