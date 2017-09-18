#pragma once

#include <neurology/object.hpp>

#include "../test.hpp"

namespace NeurologyTest
{
   class ObjectTest : public Test
   {
   public:
      static ObjectTest Instance;

   protected:
      ObjectTest(void);

   public:
      virtual void run(FailVector *failures);
   };
}
