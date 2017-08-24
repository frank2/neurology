#pragma once

#include <windows.h>

#include <neurology/exception.hpp>

namespace Neurology
{
   class Reference;
   
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

   class SlaughteredReferenceException : public ReferenceException
   {
   public:
      const Reference &suspect;
      
      SlaughteredReferenceException(const Reference &reference, const Reference &suspect);
   };

   class ConstMismatchException : public ReferenceException
   {
   public:
      ConstMismatchException(const Reference &reference);
   };
   
   /* for objects that require this kind of design pattern. basically, this is a way to make
      sure that dynamic objects, such as memory addresses, are consistent across multiple objects
      referring to the same information. this is not the same as a C++ reference. */
   class Reference
   {
   protected:
      LPDWORD refCount;
      bool constReference;
      
   public:
      Reference(void);
      Reference(Reference &reference);
      Reference(const Reference &reference);
      ~Reference(void);

      virtual void operator=(Reference &reference);
      virtual void operator=(const Reference &reference);

      DWORD refs(void) const;
      virtual void ref(void);
      virtual void deref(void);
      virtual bool isNull(void) const;
      bool isConst(void) const;
      void throwIfNull(void) const;

   protected:
      virtual void allocate(void);
      virtual void release(void);
   };
}
