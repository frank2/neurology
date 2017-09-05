#pragma once

#include <windows.h>

#include <map>
#include <set>
#include <vector>

#include <neurology/address.hpp>
#include <neurology/exception.hpp>
#include <neurology/object.hpp>

#define BlockData(ptr, size) Neurology::Data(static_cast<LPBYTE>(ptr), static_cast<LPBYTE>(ptr)+(size))
#define PointerData(ptr) BlockData(ptr, sizeof(*ptr))
#define VarData(var) PointerData(&var)

namespace Neurology
{
   typedef std::vector<BYTE> Data;

   LONG CopyData(LPVOID destination, const LPVOID source, SIZE_T size);

   class Allocator;
   
   class Allocation
   {
      friend Allocator;
      
   public:
      class Exception : public Neurology::Exception
      {
      public:
         const Allocation &allocation;

         Exception(const Allocation &allocation, const LPWSTR message);
      };

      class NoAllocatorException : public Exception
      {
      public:
         NoAllocatorException(const Allocation &allocation);
      };

      class DoubleAllocationException : public Exception
      {
      public:
         DoubleAllocationException(const Allocation &allocation);
      };

      class DeadAllocationException : public Exception
      {
      public:
         DeadAllocationException(const Allocation &allocation);
      };

      class ZeroSizeException : public Exception
      {
      public:
         ZeroSizeException(const Allocation &allocation);
      };

      class InsufficientSizeException : public Exception
      {
      public:
         const SIZE_T size;
         
         InsufficientSizeException(const Allocation &allocation, const SIZE_T size);
      };

      class AddressOutOfRangeException : public Exception
      {
      public:
         const Address address;
         const SIZE_T size;

         AddressOutOfRangeException(const Allocation &allocation, const Address address, const SIZE_T size);
      };

      class OffsetOutOfRangeException : public Exception
      {
      public:
         const SIZE_T offset, size;

         OffsetOutOfRangeException(const Allocation &allocation, const SIZE_T offset, const SIZE_T size);
      };

   protected:
      Allocator *allocator;

   private:
      Allocation(Allocator *allocator, Address &address, SIZE_T size);
      
   public:
      Allocation(void);
      Allocation(Allocator *allocator);
      Allocation(Allocation &allocation);
      Allocation(const Allocation *allocation);
      ~Allocation(void);

      void operator=(Allocation &allocation);
      void operator=(const Allocation *allocation);
      Address operator*(void);
      const Address operator*(void) const;

      bool isValid(void) const;
      bool isBound(void) const;
      bool isNull(void) const;
      bool inRange(SIZE_T offset) const;
      bool inRange(SIZE_T offset, SIZE_T size) const;
      bool inRange(const Address &address) const;
      bool inRange(const Address &address, SIZE_T size) const;

      void throwIfNoAllocator(void) const;
      void throwIfInvalid(void) const;
      void throwIfNotInRange(SIZE_T offset) const;
      void throwIfNotInRange(SIZE_T offset, SIZE_T size) const;
      void throwIfNotInRange(Address &address) const;
      void throwIfNotInRange(Address &address, SIZE_T size) const;

      Address address(void);
      const Address address(void) const;
      Address address(SIZE_T offset);
      const Address address(SIZE_T offset) const;
      Address start(void);
      const Address start(void) const;
      Address end(void);
      const Address end(void) const;

      SIZE_T getSize(void) const;

      void allocate(SIZE_T size);
      void reallocate(SIZE_T size);
      void deallocate(void);
      
      Data read(void) const;
      Data read(SIZE_T size) const;
      Data read(SIZE_T offset, SIZE_T size) const;
      Data read(const Address &address, SIZE_T size) const;
      void write(const Data data);
      void write(SIZE_T offset, const Data data);
      void write(Address &address, const Data data);

      void copy(Allocation &allocation);
      void clone(const Allocation &allocation);
   };

   class Allocator
   {
      friend Allocation;
      
   public:
      /* define the Allocator exceptions */
      class Exception : public Neurology::Exception
      {
      public:
         Allocator &allocator;

         Exception(Allocator &allocator, const LPWSTR message);
      };

      class ZeroSizeException : public Exception
      {
      public:
         ZeroSizeException(Allocator &allocator);
      };

      class PoolAllocationException : public Exception
      {
      public:
         PoolAllocationException(Allocator &allocator);
      };

      class BindingException : public Exception
      {
      public:
         Allocation &allocation;
         
         BindingException(Allocator &allocator, Allocation &allocation, const LPWSTR message);
      };

      class BoundAllocationException : public BindingException
      {
      public:
         BoundAllocationException(Allocator &allocator, Allocation &allocation);
      };

      class UnboundAllocationException : public BindingException
      {
      public:
         UnboundAllocationException(Allocator &allocator, Allocation &allocation);
      };

      class UnmanagedAllocationException : public Exception
      {
      public:
         Allocation &allocation;

         UnmanagedAllocationException(Allocator &allocator, Allocation &allocation);
      };

      class BadPointerException : public Exception
      {
      public:
         Allocation &allocation;
         const Address &address;
         const SIZE_T size;

         BadPointerException(Allocator &allocator, Allocation &allocation, const Address &address, SIZE_T size);
      };

      class InsufficientSizeException : public Exception
      {
      public:
         const SIZE_T size;
         
         InsufficientSizeException(Allocator &allocation, const SIZE_T size);
      };

      typedef std::map<Address, SIZE_T> MemoryPool;
      typedef std::map<Allocation *, Address> AssociationMap;
      typedef std::map<Address, std::set<Allocation *> > BindingMap;
      typedef std::map<Allocation *, AddressPool> PoolMap;
      
   protected:
      AddressPool pooledAddresses;
      MemoryPool pooledMemory;
      AssociationMap associations;
      BindingMap bindings;
      PoolMap addressPools;

   public:
      Allocator(void);
      ~Allocator(void);
      
      bool isPooled(const Address &address) const;
      bool isAllocated(const Allocation &allocation) const;
      bool isBound(const Allocation &allocation) const;
      bool hasAllocation(const Allocation &allocation) const;

      void throwIfNotPooled(const Address &address) const;
      void throwIfNotAllocated(const Allocation &allocation) const;
      void throwIfBound(const Allocation &allocation) const;
      void throwIfNotBound(const Allocation &allocation) const;

      SIZE_T bindCount(const Address &address) const;

      virtual Address pool(SIZE_T size);
      virtual void repool(Address &address, SIZE_T newSize);
      virtual void unpool(Address &address);
      
      virtual Allocation find(const Address &address);
      virtual Allocation null(void);
      virtual Allocation allocate(SIZE_T size);
      virtual void reallocate(Allocation &allocation, SIZE_T size);
      virtual void deallocate(Allocation &allocation);

      virtual Data read(const Address &address, SIZE_T size) const;
      virtual void write(Address &destination, const Address &source, SIZE_T size);
      
   protected:
      void bind(Allocation *allocation, Address address);
      void rebind(Allocation *allocation, Address newAddress);
      void unbind(Allocation *allocation);
   };
}
