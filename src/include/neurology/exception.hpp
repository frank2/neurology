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
}
