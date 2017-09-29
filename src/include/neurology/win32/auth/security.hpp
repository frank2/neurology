#pragma once

#include <windows.h>

#include <neurology/win32/handle.hpp>

namespace Neurology
{
   struct SecurityInformation
   {
      union
      {
         struct
         {
            BYTE owner : 1;
            BYTE group : 1;
            BYTE dacl : 1;
            BYTE sacl : 1;
            BYTE label : 1;
            BYTE attribute : 1;
            BYTE scope : 1;
            BYTE processTrustLabel : 1;
            BYTE __padding1;
            BYTE backup : 1;
            BYTE __padding2 : 11;
            BYTE unprotectedSACL : 1;
            BYTE unprotectedDACL : 1;
            BYTE protectedSACL : 1;
            BYTE protectedDACL : 1;
         };
         DWORD mask;
      };

      SecurityInformation(void) { this->mask = 0; }
      SecurityInformation(DWORD mask) { this->mask = mask; }
      operator DWORD (void) { return this->mask; }
   };

   class SecurityDescriptor
   {
   protected:
      PSID owner, group;
      PACL dacl, sacl;
      PSECURITY_DESCRIPTOR descriptor;

   public:
      SecurityDescriptor(void);
      SecurityDescriptor(PSECURITY_DESCRIPTOR descriptor);
   };

   class Security
   {
   protected:
}
