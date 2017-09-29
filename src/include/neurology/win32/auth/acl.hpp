#pragma once

#include <windows.h>

namespace Neurology
{
   struct ACLInformation
   {
      union
      {
         struct
         {
            BYTE revision:1;
            BYTE size:1;
         };
         DWORD mask;
      };

      ACLInformation(void) : mask(0) {}
      ACLInformation(DWORD mask) : mask(mask) {}
      ACLInformation(const ACLInformation &info) : mask(info.mask) {}
      ACLInformation &operator=(const ACLInformation &info) { this->mask = info.mask; return *this; }
      operator DWORD(void) { return this->mask; }
   };
        
   class AccessControlList
   {
   protected:
      PACL acl;

   public:
      AccessControlList(void) : acl(NULL) {}
      AccessControlList(PACL acl) : acl(acl) {}
      AccessControlList(AccessControlList &acl) : acl(acl.acl) {}
      AccessControlList &operator=(AccessControlList &acl) { this->setACL(acl.acl); return *this; }
      AccessControlList &operator=(PACL acl) { this->setACL(acl); return *this; }
      PACL &operator*(void) { return this->getACL(); }
      const PACL &operator*(void) const { return this->getACL(); }
      PACL *operator&(void) { return &this->acl; }
      const PACL *operator&(void) { return &this->acl; }

      bool isNull(void) const nothrow { return this->acl == NULL; }
      PACL &getACL(void) { return this->acl; }
      const PACL &getACL(void) const { return this->acl; }
      void setACL(PACL acl) { this->acl = acl; }
   };
}
