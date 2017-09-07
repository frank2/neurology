#pragma once

#include <windows.h>

#include <neurology/exception.hpp>
#include <neurology/object.hpp>

#define NULL_HANDLE (static_cast<HANDLE>(NULL))

namespace Neurology
{
   class Handle
   {
   public:
      class Exception : public Neurology::Exception
      {
      public:
         Handle &handle;

         Exception(Handle &handle, const LPWSTR message);
      };

      class NullHandleException : public Exception
      {
      public:
         NullHandleException(Handle &handle);
      };

   protected:
      Object<HANDLE> handle;

   public:
      Handle(void);
      Handle(HANDLE handle);
      Handle(const Handle &handle);
      ~Handle(void);

      void operator=(const Handle &handle);

      bool isNull(void) const;

      void throwIfNull(void) const;
      
      HANDLE getHandle(void) const;
      void setHandle(HANDLE handle);
      
      virtual void close(void);
   };
}
