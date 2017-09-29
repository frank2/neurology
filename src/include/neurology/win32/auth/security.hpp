#pragma once

#include <windows.h>

#include <neurology/win32/handle.hpp>
#include <neurology/win32/auth/acl.hpp>
#include <neurology/win32/auth/sid.hpp>

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
      SecurityIdentifier owner, group;
      AccessControlList sacl, dacl;
      PSECURITY_DESCRIPTOR descriptor;

   public:
      SecurityDescriptor(void) : descriptor(NULL) {}
      SecurityDescriptor(PSECURITY_DESCRIPTOR descriptor) : descriptor(descriptor) {}
      SecurityDescriptor(SecurityDescriptor &descriptor) : owner(descriptor.owner), group(descriptor.group), sacl(descriptor.sacl)
                                                         , dacl(descriptor.dacl), descriptor(descriptor.descriptor) {}
      ~SecurityDescriptor(void) { if (this->descriptor != NULL) LocalFree(this->descriptor); }

      static SecurityDescriptor GetDescriptor(Handle handle, SE_OBJECT_TYPE type)
      {
         SecurityDescriptor result;
         result.fromHandle(handle, type);
         return result;
      }

      SecurityDescriptor &operator=(PSECURITY_DESCRIPTOR descriptor) { this->setDescriptor(descriptor); return *this; }
      SecurityDescriptor &operator=(SecurityDescriptor &descriptor)
      {
         this->owner = descriptor.owner;
         this->group = descriptor.group;
         this->sacl = descriptor.sacl;
         this->dacl = descriptor.dacl;
         this->descriptor = descriptor.descriptor;
         return *this;
      }
      
      bool isNull(void) const nothrow { return this->descriptor == NULL; }

      void fromHandle(Handle handle, SE_OBJECT_TYPE type);
      void setDescriptor(PSECURITY_DESCRIPTOR descriptor)
      {
         if (this->descriptor != NULL)
            LocalFree(this->descriptor);

         this->descriptor = descriptor;
         this->owner = NULL;
         this->group = NULL;
         this->sacl = NULL;
         this->dacl = NULL;
      }

      SecurityIdentifier &getOwner(void);
      const SecurityIdentifier &getOwner(void) const { return this->owner; }
      SecurityIdentifier &getGroup(void);
      const SecurityIdentifier &getgroup(void) const { return this->group; }
      AccessControlList &getSACL(void);
      const AccessControlList &getSACL(void) const { return this->sacl; }
      AccessControlList &getDACL(void);
      const AccessControlList &getDACL(void) const { return this->dacl; }
   };
}
