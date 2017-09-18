#pragma once

#include <neurology/allocators/local.hpp>

#include "../test.hpp"

namespace NeurologyTest
{
   class LocalAllocatorTest : public Test
   {
   public:
      static LocalAllocatorTest Instance;

   protected:
      LocalAllocatorTest(void);

   public:
      virtual void run(FailVector *failures);
      void testAllocator(FailVector *failures);
      void testAllocation(FailVector *failures);
   };
}
