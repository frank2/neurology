#pragma once

#include <windows.h>

#include <neurology/exception.hpp>
#include <neurology/reference.hpp>

namespace Neurology
{
   class HandleException : public NeurologyException
   {
   protected:
      Handle *handle;

   public:
      HandleException(Handle *handle, const LPWSTR message);
      HandleException(HandleException &exception);
      ~HandleException(void);
   };

   class NullHandleException : public HandleException
   {
   public:
      NullHandleException(Handle *handle);
      NullHandleException(NullHandleException &exception);
   };
   
   class Handle
   {
   public:
      class Reference : public Neurology::Reference
      {
      protected:
         HANDLE *handleRef;
         
      public:
         Reference(void);
         Reference(HANDLE handle);
         Reference(Reference &reference);

         HANDLE handle(void);
         void setHandle(HANDLE handle);

      protected:
         void allocate(void);
         void release(void);
      };
      
   protected:
      Handle::Reference reference;
      SE_OBJECT_TYPE objectType;

   public:
      Handle(void);
      Handle(HANDLE handle, SE_OBJECT_TYPE objectType);
      Handle(Handle &handle);
      ~Handle(void);

      void ref(void);
      void deref(void);
      DWORD refs(void);
      bool isNull(void);
      void invalidate(void);

      HANDLE handle(void);
      void setHandle(HANDLE handle);
      SE_OBJECT_TYPE type(void);
      void setType(SE_OBJECT_TYPE type);
      void close(void);
   };
}
