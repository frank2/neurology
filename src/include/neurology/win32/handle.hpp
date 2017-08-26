#pragma once

#include <windows.h>

#include <neurology/exception.hpp>
#include <neurology/reference.hpp>

namespace Neurology
{
   class HandleException : public NeurologyException
   {
   public:
      Handle handle;

      HandleException(Handle &handle, const LPWSTR message);
   };

   class NullHandleException : public HandleException
   {
   public:
      NullHandleException(Handle &handle);
   };

   class HandleTypeMismatchException : public HandleException
   {
   public:
      Handle otherHandle;

      HandleTypeMismatchException(Handle &handle, Handle &otherHandle);
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

         virtual void operator=(Reference &reference);
         virtual void operator=(HANDLE handle);

         virtual bool isNull(void) const;
         bool hasNullHandle(void) const;
         
         HANDLE handle(void) const;
         void setHandle(HANDLE handle);

         void close(void);

      protected:
         virtual void allocate(void);
         virtual void release(void);
      };
      
   protected:
      Handle::Reference reference;
      SE_OBJECT_TYPE objectType;

   public:
      Handle(void);
      Handle(HANDLE handle);
      Handle(HANDLE handle, SE_OBJECT_TYPE objectType);
      Handle(Handle &handle);
      ~Handle(void);

      virtual void operator=(Handle &handle);
      virtual void operator=(HANDLE handle);

      virtual void ref(void);
      virtual void deref(void);
      DWORD refs(void) const;
      bool isNull(void) const;
      bool hasNullHandle(void) const;
      void close(void);

      HANDLE handle(void) const;
      void setHandle(HANDLE handle);
      void setHandle(HANDLE handle, SE_OBJECT_TYPE objectType);
      SE_OBJECT_TYPE type(void) const;
      void setType(SE_OBJECT_TYPE type);
   };
}
