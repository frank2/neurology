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
   /**
      A std::vector representing bytes of arbitrary data.
   */
   typedef std::vector<BYTE> Data;

   class Allocation;

   class Allocator
   {
      friend Allocation;
      
   public:
      class Exception : public Neurology::Exception
      {
      public:
         /**
            The allocator which threw this exception.
         */
         Allocator &allocator;

         Exception(Allocator &allocator, const LPWSTR message);
      };
      
      /**
         This exception gets thrown when an allocation was of zero size.
      */
      class ZeroSizeException : public Exception
      {
         
      public:
         ZeroSizeException(Allocator &allocator);
      };
      
      /**
         This exception gets thrown when pool allocation fails.
      */
      class PoolAllocationException : public Exception
      {
         
      public:
         
         PoolAllocationException(Allocator &allocator);
      };

      /**
         This exception gets thrown when an address does not belong to the
         allocation pool.
      */
      class UnpooledAddressException : public Exception
      {
      public:
         /**
            The address which caused this exception.
         */
         const Address &address;

         UnpooledAddressException(Allocator &allocator, const Address &address);
      };

      /**
         The group of exceptions which are raised when binding addresses to
         an allocation.
      */
      class BindingException : public Exception
      {
      public:
         /**
            The allocation which raised this exception.
         */
         Allocation &allocation;
         
         BindingException(Allocator &allocator, Allocation &allocation, const LPWSTR message);
      };

      /**
         The exception raised when an allocation is already bound.
      */
      class BoundAllocationException : public BindingException
      {
      public:
         BoundAllocationException(Allocator &allocator, Allocation &allocation);
      };

      /**
         The exception raised when an allocation is not bound.
      */
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

      /**
         The set of all allocations made by this allocator.
      */
      typedef std::set<Allocation *> AllocationSet;

      /**
         The mapping of addresses associated with allocations.
      */
      typedef std::map<const Allocation *, Address> AssociationMap;

      /**
         The mapping of addressed allocations and their given
         allocation sizes.
      */
      typedef std::map<const Address, SIZE_T> MemoryPool;

      /**
         The mapping of an address to a given set of allocations.
      */
      typedef std::map<const Address, AllocationSet> BindingMap;

   protected:
      /**
         Mark whether or not this allocator is local or remote.
      */
      bool local;

      /**
         Addresses pooled by this allocator.
      */
      AddressPool pooledAddresses;

      /**
         The set of allocations created by this allocator.
      */
      AllocationSet allocations;

      /**
         The mapping of allocations to addresses created by this allocator.
      */
      AssociationMap associations;

      /**
         The size of allocations created by this allocator, mapped by
         each address its created.
      */
      MemoryPool pooledMemory;

      /**
         The mapping of each address to its given allocation within
         the allocator.
      */
      BindingMap bindings;

   public:
      Allocator(void);
      ~Allocator(void);

      /**
         Return whether or not this allocation is local or remote.
      */
      bool isLocal(void) const noexcept;

      /**
         Return whether or not a given address has been pooled.
      */
      bool isPooled(const Address &address) const noexcept;

      /**
         Return whether or not a given allocation is bound to this allocator.
      */
      bool isBound(const Allocation &allocation) const noexcept;

      /**
         Return whether or not the given address exists somewhere in
         this allocator.
      */
      bool hasAddress(const Address &address) const noexcept;

      /**
         Return whether or not the first allocation shares a similar address
         pool with the second.
      */
      bool sharesPool(const Allocation &left, const Allocation &right) const noexcept;

      /**
         Return whether or not the given allocation has a parent. Alternatively,
         return whether or not the given allocation is a child.
      */
      bool hasParent(const Allocation &allocation) const noexcept;

      /** 
         Return whether or not the given allocation has child allocations.
         Alternatively, return whether or not this allocation is a parent.
      */
      bool hasChildren(const Allocation &allocation) const noexcept;

      /** 
          Return whether or not the first allocation is a parent of the second.
       */
      bool isChild(const Allocation &parent, const Allocation &child) const noexcept;

      /**
         Throw an exception if the given address isn't pooled somewhere
         within the allocator.
      */
      void throwIfNotPooled(const Address &address) const;

      /**
         Throw an exception if the given allocation is bound to
         this allocator.
      */
      void throwIfBound(const Allocation &allocation) const;

      /**
         Throw an exception if the given address is not allocated
         by this allocator.
      */
      void throwIfNoAddress(const Address &address) const;

      /**
         Throw an exception if the given allocation is not bound
         to this allocator.
      */
      void throwIfNotBound(const Allocation &allocation) const;

      /**
         Throw an exception if the given address has no allocation
         within an allocation.
      */
      void throwIfNoAllocation(const Address &address) const;

      /**
         Throw an exception if the given allocation does not have
         a parent allocation within the allocator.
      */
      void throwIfNoParent(const Allocation &allocation) const;

      /**
         Return the const base address of the given allocation. If the
         allocation is not bound to this allocator, a null address is returned.
      */
      const Address addressOf(const Allocation &allocation) const noexcept;

      /**
         Return the base address of the given allocation.
      */
      Address address(Allocation &allocation);

      /**
         Return an address from the allocation with an offset
         from its base.
      */
      Address address(Allocation &allocation, SIZE_T offset);

      /**
         Return the number of bindings a given address has
         within the allocator.
      */
      SIZE_T bindCount(const Address &address) const;

      /**
         Return the size of the given allocation. If the allocation
         isn't bound, this function returns 0.
      */
      SIZE_T querySize(const Allocation &allocation) const;

      /**
         Find an allocation which is large enough for the given size
         and return an address from it.
      */
      Address pool(SIZE_T size);

      /**
         Move the given address to this allocator with the given size.
      */
      Address repool(Address &address, SIZE_T newSize);
      
      /**
         Remove the given address from the address pool.
      */
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

      template <class Type> void reallocate(Allocation &allocation)
      {
         this->reallocate(allocation, sizeof(Type));
      }

      template <class Type> void reallocate(Allocation &allocation, SIZE_T size)
      {
         if (sizeof(Type) > size)
            throw InsufficientSizeException(*this, size);

         this->reallocate(&allocation, size);
      }
      
      void deallocate(Allocation &allocation);

      Data read(const Address &address, SIZE_T size) const;
      void write(const Address &address, const Data data);

      Allocation &root(Allocation &allocation) const;
      const Allocation &root(const Allocation &allocation) const;
      Allocation &parent(Allocation &allocation);
      const Allocation &parent(const Allocation &allocation) const;
      AllocationSet children(const Allocation &allocation) const;

      void zeroAddress(const Address &address, SIZE_T size);
      
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

      void zeroFill(void);
      
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
