#pragma once

#include <windows.h>

#include <neurology/win32/access.hpp>

namespace Neurology
{
   class Token
   {
   public:
      struct AccessMask
      {
         union
         {
            struct
            {
               BYTE assignPrimary : 1;
               BYTE duplicate : 1;
               BYTE impersonate : 1;
               BYTE query : 1;
               BYTE querySource : 1;
               BYTE adjustPrivileges : 1;
               BYTE adjustGroups : 1;
               BYTE adjustDefault : 1;
               BYTE adjustSessionID : 1;
            };
            Neurology::AccessMask standard;
         };

         AccessMask(void) { this->standard.mask = 0; }
         AccessMask(DWORD mask) { this->standard.mask = mask; }
         operator DWORD (void) { return this->standard.mask; }
      };
      
   protected:
      Handle handle;

   public:
      Token(void);
      Token(Handle handle);
      Token(Token &token);
      Token(const Token *token);
   };
}
