#pragma once

#include <windows.h>

namespace Neurology
{

#ifdef _DEBUG
#define EXCSTR(str) (str)
#else
#define EXCSTR(str) (NULL)
#endif
   
   class NeurologyException
   {
   public:
      LPWSTR explanation;
      
   public:
      NeurologyException(const LPWSTR message);
   };

   class Win32Exception : public NeurologyException
   {
   public:
      DWORD error;

      Win32Exception(const LPWSTR message);
      Win32Exception(DWORD error, const LPWSTR message);
   };

   class NullPointerException : public NeurologyException
   {
   public:
      NullPointerException(void);
      NullPointerException(const LPWSTR message);
   };
}
