#pragma once

#include <windows.h>

#include <neurology/exception.hpp>

namespace Neurology
{
   class NullSIDException : public NeurologyException
   {
   public:
      NullSIDException(void);
      NullSIDException(NullSIDException &exception);
   };

   class NullACLException : public NeurologyException
   {
   public:
      NullACLException(void);
      NullACLException(NullACLException &exception);
   };

   class NullTokenException : public NeurologyException
   {
   public:
      NullTokenException(void);
      NullTokenException(NullTokenException &exception);
   };
   
   class SecurityIdentifier
   {
   protected:
      PSID sid;

   public:
      SecurityIdentifier(void);
      SecurityIdentifier(PSID sid);
      SecurityIdentifier(SecurityIdentifier &identifier);
   };

   class AccessControlList
   {
   protected:
      PACL acl;

   public:
      AccessControlList(void);
      AccessControlList(PACL acl);
      AccessControlList(AccessControlList &acl);
   };

   class Token
   {
   protected:
      HANDLE token;

   public:
      Token(void);
      Token(HANDLE token);
      Token(Token &token);

      static Token Process(HANDLE processHandle, DWORD access);
      static Token Thread(HANDLE threadHandle, DWORD access);

      void openProcessToken(HANDLE processHandle, DWORD access);
      void openThreadToken(HANDLE threadHandle, DWORD access);
   };
}
