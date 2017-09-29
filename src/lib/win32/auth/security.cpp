#include <neurology/win32/auth/security.hpp>

void
SecurityDescriptor::fromHandle
(Handle handle, SE_OBJECT_TYPE type)
{
   SecurityInformation information;
   DWORD status;

   information.owner = 1;
   information.group = 1;
   information.dacl = 1;
   information.sacl = 1;

   if ((status = GetSecurityInfo(*handle
                                 ,type
                                 ,information.mask
                                 ,&result.owner
                                 ,&result.group
                                 ,&result.dacl
                                 ,&result.sacl
                                 ,&result.descriptor)) != ERROR_SUCCESS)
      throw Win32Exception(status, EXCSTR(L"GetSecurityInfo failed."));

   return result;
}

SecurityIdentifier &
SecurityDescriptor::getOwner
(void)
{
   if (*this->owner == NULL)
   {
      BOOL ownerDefaulted;
      
      if (this->isNull())
         return this->owner;

      ownerDefaulted = FALSE;

      if (!GetSecurityDescriptorOwner(this->descriptor, &this->owner, &ownerDefaulted))
         throw Win32Exception(EXCSTR(L"GetSecurityDescriptorOwner failed."));
   }

   return this->owner;
}

SecurityIdentifier &
SecurityDescriptor::getGroup
(void)
{
   if (*this->group == NULL)
   {
      BOOL groupDefaulted;
      
      if (this->isNull())
         return this->group;

      groupDefaulted = FALSE;

      if (!GetSecurityDescriptorGroup(this->descriptor, &this->group, &groupDefaulted))
         throw Win32Exception(EXCSTR(L"GetSecurityDescriptorGroup failed."));
   }

   return this->group;
}

SecurityIdentifier &
SecurityDescriptor::getSACL
(void)
{
   if (*this->sacl == NULL)
   {
      BOOL SACLPresent, SACLDefaulted;
      
      if (this->isNull())
         return this->sacl;

      SACLPresent = FALSE;
      SACLDefaulted = FALSE;

      if (!GetSecurityDescriptorSacl(this->descriptor, &SACLPresent, &this->sacl, &SACLDefaulted))
         throw Win32Exception(EXCSTR(L"GetSecurityDescriptorSacl failed."));
   }

   return this->sacl;
}

SecurityIdentifier &
SecurityDescriptor::getDACL
(void)
{
   if (*this->dacl == NULL)
   {
      BOOL DACLPresent, DACLDefaulted;
      
      if (this->isNull())
         return this->dacl;

      DACLPresent = FALSE;
      DACLDefaulted = FALSE;

      if (!GetSecurityDescriptorDacl(this->descriptor, &DACLPresent, &this->dacl, &DACLDefaulted))
         throw Win32Exception(EXCSTR(L"GetSecurityDescriptorDacl failed."));
   }

   return this->sacl;
}
