#pragma once

#include <stdio.h>
#include <vector>

#include <neurology/address.hpp>

#include "../test.hpp"

namespace NeurologyTest
{
   class AddressTest : public Test
   {
   public:
      AddressTest();

      virtual void run(std::vector<TestFailure> *failures);
   };
}
