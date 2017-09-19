#pragma once

#include <windows.h>

namespace Neurology
{
   struct AccessMask
   {
      union
      {
         struct
         {
            WORD __padding;
            BYTE deleteObject : 1;
            BYTE readControl : 1;
            BYTE writeDAC : 1;
            BYTE writeOwner : 1;
            BYTE synchronize : 1;
         };
         DWORD mask;
      };

      AccessMask(void) { this->mask = 0; }
      AccessMask(DWORD mask) { this->mask = mask; }
      operator DWORD (void) { return this->mask; }
   };
}
