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
   Process process;
   Page page;
   AccessMask access;
   Object<std::uintptr_t> object;

   NASSERT(allocator.isLocal());

   process = Process::Spawn(L"notepad.exe");
   access.terminate = 1;
   access.vmOperation = 1;
   access.vmWrite = 1;
   access.vmRead = 1;
   access.queryLimitedInformation = 1;
   process.open(access);

   NEXCEPT(allocator.setProcessHandle(process.getHandle()), false);
   NASSERT(!allocator.isLocal());
   NEXCEPT(page = allocator.allocate(1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE), false);
   NEXCEPT(object = allocator.object<std::uintptr_t>(page.address()), false);
   NEXCEPT(object = 0xDEADBEEFFACEBABE, false);
   NEXCEPT(object.flush(), false);
   NASSERT(*object == 0xDEADBEEFFACEBABE);
   NEXCEPT(page.release(), false);

   process.kill(0);

   this->assertMessage(L"[*] Finished VirtualAllocator tests.");
}

void
VirtualAllocatorTest::testPage
(FailVector *failures)
{
   return;
}
