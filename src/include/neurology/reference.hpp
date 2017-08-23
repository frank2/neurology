#pragma once

#include <windows.h>

#include <neurology/exception.hpp>

namespace Neurology
{
   class ReferenceException : public NeurologyException
   {
   public:
      const Reference &reference;
      
      ReferenceException(const Reference &reference, const LPWSTR message);
      ReferenceException(ReferenceException &exception);
   };
   
   class NullReferenceException : public ReferenceException
   {
   public:
      NullReferenceException(const Reference &reference);
   };
   
   class DoubleAllocationException : public ReferenceException
   {
   public:
      DoubleAllocationException(const Reference &reference);
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

      DWORD refs(void) const;
      void ref(void);
      void deref(void);
      virtual bool isNull(void) const;

   protected:
      virtual void allocate(void);
      virtual void release(void);
   };
}
