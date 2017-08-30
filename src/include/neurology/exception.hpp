#pragma once

#include <windows.h>

#ifdef _DEBUG
#define EXCSTR(str) (str)
#else
#define EXCSTR(str) (NULL)
#endif

#define UNUSED(expr) do { (void)(expr); } while (0)

namespace Neurology
{
   class Exception
   {
   public:
      const LPWSTR explanation;

      Exception(const LPWSTR message);
   };

   class Win32Exception : public Exception
   {
   public:
      DWORD error;

      Win32Exception(const LPWSTR message);
      Win32Exception(DWORD error, const LPWSTR message);
   };

   class NullPointerException : public Exception
   {
   public:
      NullPointerException(void);
      NullPointerException(const LPWSTR message);
   };
}
