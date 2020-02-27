#pragma once

#include <neurology/win32/process.hpp>
#include <neurology/allocators/virtual.hpp>

#include "../test.hpp"

namespace NeurologyTest
{
   class VirtualAllocatorTest : public Test
   {
   public:
      static VirtualAllocatorTest Instance;

   protected:
      VirtualAllocatorTest(void);

   public:
      virtual void run(FailVector *failures);
      void testAllocator(FailVector *failures);
      void testPage(FailVector *failures);
   };
}
