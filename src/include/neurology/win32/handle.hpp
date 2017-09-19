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
      Handle(Handle &handle);
      Handle(const Handle *handle);
      ~Handle(void);

      Handle &operator=(Handle &handle);
      Handle &operator=(const Handle *handle);
      bool operator==(const Handle &handle);
      bool operator!=(const Handle &handle);
      HANDLE &operator*(void);
      const HANDLE &operator*(void) const;

      bool isNull(void) const;
      bool isValid(void) const;
      void throwIfNull(void) const;
      
      HANDLE getHandle(void);      
      HANDLE getHandle(void) const;
      void setHandle(HANDLE handle);

      Handle duplicate(void);
      Handle duplicate(DWORD access, BOOL inheritHandle, DWORD options);
      Handle duplicate(Handle sourceProcess, Handle destProcess);
      Handle duplicate(Handle sourceProcess, Handle destProcess, DWORD access, BOOL inheritHandle, DWORD options);
      
      virtual void close(void);
   };
}
