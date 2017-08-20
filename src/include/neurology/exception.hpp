#pragma once

#include <exception>

namespace Neurology
{
   class NeurologyException : public std::exception
   {
   protected:
      const char *explanation;
      
   public:
      NeurologyException(const char *message);
      NeurologyException(NeurologyException &exception);
      
      virtual const char *what(void) const;
   };

   class Win32Exception : public NeurologyException
   {
   public:
      DWORD error;

      Win32Exception(const char *message);
      Win32Exception(DWORD error, const char *message);
      Win32Exception(Win32Exception &exception);
   };
}
