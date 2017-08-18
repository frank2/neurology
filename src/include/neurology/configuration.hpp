#pragma once

#include <windows.h>

#include <neurology/heap.hpp>

namespace Neurology
{
   class Configuration
   {
   private:
      Configuration(void);
      Configuration(Configuration &config);

   public:
      static void Standard(void);
   };
}
