#pragma once

#include <windows.h>

#include <algorithm>

#include <neurology/allocators/void.hpp>
#include <neurology/exception.hpp>

namespace Neurology
{
   LONG CopyData(LPVOID destination, const LPVOID source, SIZE_T size);
   
   class LocalAllocator : public Allocator
   {
   public:
      static LocalAllocator Instance;
      
      class Exception : public Allocator::Exception
      {
      public:
         Exception(LocalAllocator &allocator, const LPWSTR message);
      };

      class KernelFaultException : public Exception
      {
      public:
         LONG status;
         Address &source, &destination;
         SIZE_T size;

         KernelFaultException(LocalAllocator &allocator, LONG status, Address &source, Address &destination, SIZE_T size);
      };

      LocalAllocator(void);

      virtual Address poolAddress(SIZE_T size);
      virtual Address repoolAddress(Address &address, SIZE_T newSize);
      virtual void unpoolAddress(Address &address);

      virtual Data readAddress(const Address &address, SIZE_T size) const;
      virtual void writeAddress(const Address &destination, const Data data);
      void zeroAddress(const Address &address, SIZE_T size);
   };

   Allocation nrlMalloc(SIZE_T size);

   template <class Type> Allocation nrlMalloc(void)
   {
      return LocalAllocator::Instance.allocate<Type>();
   }

   template <class Type> Allocation nrlMalloc(SIZE_T size)
   {
      return LocalAllocator::Instance.allocate<Type>(size);
   }

   void nrlRealloc(Allocation &allocation, SIZE_T size);

   template <class Type> void nrlRealloc(Allocation &allocation)
   {
      LocalAllocator::Instance.reallocate<Type>(allocation);
   }

   template <class Type> void nrlRealloc(Allocation &allocation, SIZE_T size)
   {
      LocalAllocator::Instance.reallocate<Type>(allocation, size);
   }

   void nrlFree(Allocation &allocation);

   /*
   template <class Type> Object<Type> nrlObject(void)
   {
      return Object<Type>(neuMalloc<Type>());
   }

   template <class Type> Pointer<Type> nrlPointer(void)
   {
      return Pointer<Type>(neuMalloc<Type>());
   }

   template <class Type> Pointer<Type> nrlPointer(SIZE_T size)
   {
      return Pointer<Type>(neuMalloc<Type>(size));
   }
   */
}
