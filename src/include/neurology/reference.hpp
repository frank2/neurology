#pragma once

#include <windows.h>

#include <neurology/exception.hpp>

namespace Neurology
{
   class ReferenceException : public NeurologyException
   {
   public:
      ReferenceException(const LPWSTR message);
   };
   
   class NullReferenceException : public ReferenceException
   {
   public:
      NullReferenceException(void);
   };
   
   class DoubleAllocationException : public ReferenceException
   {
   public:
      DoubleAllocationException(void);
   };
   
   /* for objects that require this kind of design pattern. basically, this is a way to make
      sure that dynamic objects, such as memory addresses, are consistent across multiple objects
      referring to the same information. this is not the same as a C++ reference. */
   class Reference
   {
   protected:
      LPDWORD refCount;

   public:
      Reference(void);
      Reference(Reference &reference);
      ~Reference(void);

      DWORD refs(void);
      void ref(void);
      void deref(void);
      virtual bool isNull(void);

   protected:
      virtual void allocate(void);
      virtual void release(void);
   };
}
