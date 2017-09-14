#pragma once

#include <windows.h>

#include <map>
#include <set>
#include <vector>

#include <neurology/address.hpp>
#include <neurology/exception.hpp>

#define BlockData(ptr, size) Neurology::Data((LPBYTE)(ptr),((LPBYTE)(ptr))+(size))
#define PointerData(ptr) BlockData(ptr, sizeof(*ptr))
#define VarData(var) PointerData(&var)

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

      typedef std::map<Address, SIZE_T> MemoryPool;
      typedef std::map<Address, AddressPool *> AddressPoolMap;
      typedef std::map<Allocation *, Address> AssociationMap;
      typedef std::map<Address, std::set<Allocation *> > BindingMap;
      typedef std::map<Allocation *, std::set<Allocation *> > ChildMap;
      typedef std::map<Allocation *, Allocation *> ParentMap;
      
   protected:
      bool split;
      bool local;
      
      AddressPool pooledAddresses;
      MemoryPool pooledMemory;
      AddressPoolMap addressPools;
      AssociationMap associations;
      BindingMap bindings;
      ChildMap children;
      ParentMap parents;

   public:
      Allocator(void);
      Allocator(bool split);
      ~Allocator(void);

      void allowSplitting(void);
      void denySplitting(void);
      bool splits(void) const;

      bool isLocal(void) const;
      bool isPooled(const Address &address) const;
      bool isAssociated(const Allocation &allocation) const;
      bool isBound(const Allocation &allocation) const;
      bool hasAddress(const Address &address) const;
      bool willSplit(const Address &address, SIZE_T size) const;
      bool sharesPool(const Allocation &left, const Allocation &right) const;
      bool hasParent(const Allocation &allocation) const;
      bool hasChildren(const Allocation &allocation) const;
      bool isChild(const Allocation &parent, const Allocation &child) const;

      void throwIfNotPooled(const Address &address) const;
      void throwIfNotAssociated(const Allocation &allocation) const;
      void throwIfBound(const Allocation &allocation) const;
      void throwIfNoAddress(const Address &address) const;
      void throwIfNotBound(const Allocation &allocation) const;
      void throwIfNoAllocation(const Address &address) const;
      void throwIfNoParent(const Allocation &allocation) const;

      const Address addressOf(const Allocation &allocation) const;
      Address address(const Allocation &allocation);
      Address address(const Allocation &allocation, SIZE_T offset);
      Address newAddress(const Allocation &allocation);
      Address newAddress(const Allocation &allocation, SIZE_T offset);
      SIZE_T bindCount(const Address &address) const;
      SIZE_T querySize(const Allocation &allocation) const;

      Address pool(SIZE_T size);
      Address repool(Address &address, SIZE_T newSize);
      void unpool(Address &address);
      
      virtual Allocation &find(const Address &address) const;

      virtual Allocation null(void);
      virtual Allocation null(void) const;
      
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
         
      void reallocate(const Allocation &allocation, SIZE_T size);

      template <class Type> void reallocate(const Allocation &allocation)
      {
         this->reallocate(allocation, sizeof(Type));
      }

      template <class Type> Allocation reallocate(const Allocation &allocation, SIZE_T size)
      {
         if (sizeof(Type) > size)
            throw InsufficientSizeException(*this, size);

         return this->reallocate(allocation, size);
      }
      
      void deallocate(Allocation &allocation);

      Data read(const Address &address, SIZE_T size) const;
      void write(const Address &address, const Data data);

      Allocation &root(const Allocation &allocation) const;
      Allocation &parent(const Allocation &allocation);
      const Allocation &parent(const Allocation &allocation) const;
      std::set<const Allocation *> getChildren(const Allocation &allocation);
      
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

      void addChild(Allocation *parent, Allocation *child);
      void disownChild(Allocation *child);

      Data splitRead(const Address &startAddress, SIZE_T size) const;
      void splitWrite(const Address &destination, const Data data);

      /* functions for writing to/reading from directly to/from an allocation */
      Data read(const Allocation *allocation, const Address &address, SIZE_T size) const;
      void write(const Allocation *allocation, const Address &destination, const Data data);

      /* overloadable functions for writing to/from addresses themselves */
      virtual Data readAddress(const Address &address, SIZE_T size) const;
      virtual void writeAddress(const Address &destination, const Data data);

      Allocation spawn(const Allocation *allocation, const Address &address, SIZE_T size);
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

      bool isNull(void) const;
      bool isBound(void) const;
      bool isValid(void) const;
      bool isLocal(void) const;
      bool allocatedFrom(const Allocator *allocator) const;
      bool inRange(SIZE_T offset) const;
      bool inRange(SIZE_T offset, SIZE_T size) const;
      bool inRange(const Address &address) const;
      bool inRange(const Address &address, SIZE_T size) const;
      bool sharesPool(const Allocation &allocation) const;
      bool hasParent(void) const;
      bool hasChildren(void) const;
      bool isChild(const Allocation &parent) const;
      bool isParent(const Allocation &child) const;

      void throwIfNoAllocator(void) const;
      void throwIfInvalid(void) const;
      void throwIfNotInRange(SIZE_T offset) const;
      void throwIfNotInRange(SIZE_T offset, SIZE_T size) const;
      void throwIfNotInRange(const Address &address) const;
      void throwIfNotInRange(const Address &address, SIZE_T size) const;

      Address address(void);
      const Address address(void) const;
      Address address(SIZE_T offset);
      const Address address(SIZE_T offset) const;
      Address newAddress(void);
      const Address newAddress(void) const;
      Address newAddress(SIZE_T offset);
      const Address newAddress(SIZE_T offset) const;
      Address start(void);
      const Address start(void) const;
      Address end(void);
      const Address end(void) const;
      const Address baseAddress(void) const;
      SIZE_T offset(const Address &address) const;

      SIZE_T size(void) const;

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

      Allocation slice(const Address &address, SIZE_T size);

      Allocation &root(void) const;
      Allocation &parent(void);
      const Allocation &parent(void) const;
      std::set<const Allocation *> children(void) const;
   };
}
