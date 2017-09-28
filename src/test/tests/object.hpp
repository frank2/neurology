#pragma once

#include <neurology/object.hpp>
#include <neurology/allocators/virtual.hpp>

#include "../test.hpp"

namespace NeurologyTest
{
   class ObjectTest : public Test
   {
   public:
      static ObjectTest Instance;

   protected:
      ObjectTest(void);

      void objectTest(FailVector *failures);
      void pointerTest(FailVector *failures);

   public:
      virtual void run(FailVector *failures);
   };
}
