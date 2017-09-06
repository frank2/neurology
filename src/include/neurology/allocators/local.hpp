#pragma once

#include <windows.h>

#include <neurology/allocators/void.hpp>
#include <neurology/exception.hpp>

namespace Neurology
{
   LONG CopyData(LPVOID destination, const LPVOID source, SIZE_T size);
   
   class LocalAllocator : public Allocator
   {
   public:
      class Exception : public Allocator::Exception
      {
      public:
         Exception(LocalAllocator &allocator, const LPWSTR message);
      };

      class KernelFaultException : public Exception
      {
      public:
         LONG status;
         Address source, destination;
         SIZE_T size;

         KernelFaultException(LocalAllocator &allocator, LONG status, Address source, Address destination, SIZE_T size);
      };
      
      static LocalAllocator Instance;

      LocalAllocator(void);

      static Allocation Allocate(SIZE_T size);
      static void Deallocate(Allocation &allocation);

      virtual Address pool(SIZE_T size);
      virtual void repool(Address &address, SIZE_T newSize);
      virtual void unpool(Address &address);

      virtual Data readAddress(const Address &address, SIZE_T size);
      virtual void writeAddress(const Address &destination, const Data data);
   };
}
