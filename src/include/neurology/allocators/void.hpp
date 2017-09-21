#pragma once

#include <windows.h>

#include <list>
#include <map>
#include <set>
#include <vector>

#include <neurology/address.hpp>
#include <neurology/exception.hpp>

#define BlockData(ptr, size) Neurology::Data((LPBYTE)(ptr),((LPBYTE)(ptr))+(size))
#define PointerData(ptr) BlockData(ptr, sizeof(*ptr))
#define VarData(var) PointerData(&var)

/*
  development notes for allocations and allocators:

  bind typically has three entrypoints: allocate, spawn and copy
*/

namespace Neurology
{
   typedef std::vector<BYTE> Data;

   class Allocation;

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

      class UnpooledAddressException : public Exception
      {
      public:
         const Address &address;

         UnpooledAddressException(Allocator &allocator, const Address &address);
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

      class InsufficientSizeException : public Exception
      {
      public:
         const SIZE_T size;
         
         InsufficientSizeException(Allocator &allocation, const SIZE_T size);
      };

      class VoidAllocatorException : public Exception
      {
      public:
         VoidAllocatorException(Allocator &allocator);
      };

      class UnallocatedAddressException : public Exception
      {
      public:
         Address &address;

         UnallocatedAddressException(Allocator &allocator, Address &address);
      };

      class SplitsExceededException : public Exception
      {
      public:
         Address &address;
         SIZE_T size;

         SplitsExceededException(Allocator &allocator, Address &address, SIZE_T size);
      };

      class OrphanAllocationException : public Exception
      {
      public:
         Allocation &allocation;
         
         OrphanAllocationException(Allocator &allocator, Allocation &allocation);
      };

      class PoolCollisionException : public Exception
      {
      public:
         Address &address;

         PoolCollisionException(Allocator &allocator, Address &address);
      };

      class AddressNotFoundException : public Exception
      {
      public:
         Address &address;

         AddressNotFoundException(Allocator &allocator, Address &address);
      };

      typedef std::set<Allocation *> AllocationSet;
      typedef std::map<const Allocation *, Address> AssociationMap;
      typedef std::map<const Address, SIZE_T> MemoryPool;
      typedef std::map<const Address, AllocationSet> BindingMap;

   protected:
      bool local;
      
      AddressPool pooledAddresses;
      AllocationSet allocations;
      AssociationMap associations;
      MemoryPool pooledMemory;
      BindingMap bindings;

   public:
      Allocator(void);
      ~Allocator(void);

      bool isLocal(void) const noexcept;
      bool isPooled(const Address &address) const noexcept;
      bool isBound(const Allocation &allocation) const noexcept;
      bool hasAddress(const Address &address) const noexcept;
      bool sharesPool(const Allocation &left, const Allocation &right) const noexcept;
      bool hasParent(const Allocation &allocation) const noexcept;
      bool hasChildren(const Allocation &allocation) const noexcept;
      bool isChild(const Allocation &parent, const Allocation &child) const noexcept;

      void throwIfNotPooled(const Address &address) const;
      void throwIfBound(const Allocation &allocation) const;
      void throwIfNoAddress(const Address &address) const;
      void throwIfNotBound(const Allocation &allocation) const;
      void throwIfNoAllocation(const Address &address) const;
      void throwIfNoParent(const Allocation &allocation) const;

      const Address addressOf(const Allocation &allocation) const noexcept;
      Address address(Allocation &allocation);
      Address address(Allocation &allocation, SIZE_T offset);
      Address newAddress(Allocation &allocation);
      Address newAddress(Allocation &allocation, SIZE_T offset);
      SIZE_T bindCount(const Address &address) const;
      SIZE_T querySize(const Allocation &allocation) const;

      Address pool(SIZE_T size);
      Address repool(Address &address, SIZE_T newSize);
      void unpool(Address &address);
      
      Allocation &find(const Address &address) const;
      virtual Allocation &find(const Address &address, SIZE_T size) const;

      Allocation null(void);
      Allocation null(void) const;
      
      Allocation allocate(SIZE_T size);
      
      template <class Type> Allocation allocate(void)
      {
         return this->allocate(sizeof(Type));
      }

      template <class Type> Allocation allocate(SIZE_T size)
      {
         if (sizeof(Type) > size)
            throw InsufficientSizeException(*this, size);

         return this->allocate(size);
      }
         
      void reallocate(Allocation &allocation, SIZE_T size);

      template <class Type> void reallocate(const Allocation &allocation)
      {
         this->reallocate(allocation, sizeof(Type));
      }

      template <class Type> void reallocate(const Allocation &allocation, SIZE_T size)
      {
         if (sizeof(Type) > size)
            throw InsufficientSizeException(*this, size);

         this->reallocate(allocation, size);
      }
      
      void deallocate(Allocation &allocation);

      Data read(const Address &address, SIZE_T size) const;
      void write(const Address &address, const Data data);

      Allocation &root(Allocation &allocation) const;
      const Allocation &root(const Allocation &allocation) const;
      Allocation &parent(Allocation &allocation);
      const Allocation &parent(const Allocation &allocation) const;
      AllocationSet children(const Allocation &allocation) const;
      
   protected:
      virtual Address poolAddress(SIZE_T size);
      virtual Address repoolAddress(Address &address, SIZE_T newSize);
      virtual void unpoolAddress(Address &address);
      
      virtual void allocate(Allocation *allocation, SIZE_T size);
      virtual void reallocate(Allocation *allocation, SIZE_T size);
      virtual void deallocate(Allocation *allocation);

      void bind(Allocation *allocation, const Address &address);
      void rebind(Allocation *allocation, const Address &newAddress);
      void unbind(Allocation *allocation);

      /* functions for writing to/reading from directly to/from an allocation */
      Data read(const Allocation *allocation, const Address &address, SIZE_T size) const;
      void write(const Allocation *allocation, const Address &destination, const Data data);

      /* overloadable functions for writing to/from addresses themselves */
      virtual Data readAddress(const Address &address, SIZE_T size) const;
      virtual void writeAddress(const Address &destination, const Data data);

      Allocation spawn(Allocation *allocation, const Address &address, SIZE_T size);
   };
   
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

      class OrphanAllocationException : public Exception
      {
      public:
         OrphanAllocationException(const Allocation &allocation);
      };

   protected:
      Allocator *allocator;
      Allocation *parent;
      Allocator::AllocationSet children;
      AddressPool pool;
      
   public:
      Allocation(void);
      Allocation(Allocator *allocator);
      Allocation(Allocation &allocation);
      ~Allocation(void);

      void operator=(Allocation &allocation);
      void operator=(const Allocation *allocation);

      bool isNull(void) const noexcept;
      bool isBound(void) const noexcept;
      bool isLocal(void) const noexcept;
      bool allocatedFrom(const Allocator *allocator) const noexcept;
      bool inRange(SIZE_T offset) const noexcept;
      bool inRange(SIZE_T offset, SIZE_T size) const noexcept;
      bool inRange(const Address &address) const noexcept;
      bool inRange(const Address &address, SIZE_T size) const noexcept;
      bool hasParent(void) const noexcept;
      bool hasChildren(void) const noexcept;
      bool isChild(const Allocation &parent) const noexcept;
      bool isParent(const Allocation &child) const noexcept;

      void throwIfNoAllocator(void) const;
      void throwIfNotBound(void) const;
      void throwIfNotInRange(SIZE_T offset) const;
      void throwIfNotInRange(SIZE_T offset, SIZE_T size) const;
      void throwIfNotInRange(const Address &address) const;
      void throwIfNotInRange(const Address &address, SIZE_T size) const;
      void throwIfNoParent(void) const;

      Address address(void);
      Address address(void) const;
      Address address(SIZE_T offset);
      Address address(SIZE_T offset) const;
      Address newAddress(void);
      Address newAddress(void) const;
      Address newAddress(SIZE_T offset);
      Address newAddress(SIZE_T offset) const;
      Address start(void);
      Address start(void) const;
      Address end(void);
      Address end(void) const;
      const Address baseAddress(void) const;
      SIZE_T offset(const Address &address) const;

      SIZE_T size(void) const noexcept;

      void allocate(SIZE_T size);

      template <class Type> void allocate(void)
      {
         this->allocate(sizeof(Type));
      }

      template <class Type> void allocate(SIZE_T size)
      {
         if (size < sizeof(Type))
            throw InsufficientSizeException(*this, size);

         this->allocate(size);
      }
      
      void reallocate(SIZE_T size);

      template <class Type> void reallocate(void)
      {
         this->reallocate(sizeof(Type));
      }

      template <class Type> void reallocate(SIZE_T size)
      {
         if (size < sizeof(Type))
            throw InsufficientSizeException(*this, size);

         this->reallocate(size);
      }

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

      Allocation slice(const Address &address, SIZE_T size);

      Allocation &root(void) noexcept;
      const Allocation &root(void) const noexcept;
      Allocation &getParent(void);
      const Allocation &getParent(void) const;
      Allocator::AllocationSet getChildren(void) const;

   protected:
      void setParent(Allocation &allocation);
      void disownChild(Allocation &allocation);
      void leaveParent(void);
   };
}
