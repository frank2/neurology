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
      static AddressTest Instance;

   protected:
      AddressTest(void);

   public:
      virtual void run(FailVector *failures);
      void testAddressPool(FailVector *failures);
      void testAddress(FailVector *failures);
   };
}
