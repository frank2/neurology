#pragma once

#include <windows.h>

#include <map>
#include <set>
#include <vector>

#include <neurology/address.hpp>
#include <neurology/exception.hpp>
#include <neurology/object.hpp>

namespace Neurology
{
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
         const LPVOID address;
         const SIZE_T size;

         AddressOutOfRangeException(const Allocation &allocation, const LPVOID address, const SIZE_T size);
      };

      class OffsetOutOfRangeException : public Exception
      {
      public:
         const SIZE_T offset, size;

         OffsetOutOfRangeException(const Allocation &allocation, const SIZE_T offset, const SIZE_T size);
      };

   protected:
      Address address;
      SIZE_T size;

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
      LPVOID operator*(void);
      const LPVOID operator*(void) const;

      bool isValid(void) const;
      bool isBound(void) const;
      bool isNull(void) const;
      bool inRange(SIZE_T offset) const;
      bool inRange(SIZE_T offset, SIZE_T size) const;
      bool inRange(Address &address) const;
      bool inRange(Address &address, SIZE_T size) const;

      void throwIfNoAllocator(void) const;
      void throwIfInvalid(void) const;
      void throwIfNotInRange(SIZE_T offset) const;
      void throwIfNotInRange(SIZE_T offset, SIZE_T size) const;
      void throwIfNotInRange(Address &address) const;
      void throwIfNotInRange(Address &address, SIZE_T size) const;

      const Address &getAddress(void) const;
      LPVOID pointer(void);
      const LPVOID pointer(void) const;
      LPVOID pointer(SIZE_T offset);
      const LPVOID pointer(SIZE_T offset) const;
      LPVOID start(void);
      const LPVOID start(void) const;
      LPVOID end(void);
      const LPVOID end(void) const;

      SIZE_T getSize(void) const;

      void allocate(SIZE_T size);
      void reallocate(SIZE_T size);
      void deallocate(void);
      
      Data read(void) const;
      Data read(SIZE_T size) const;
      Data read(SIZE_T offset, SIZE_T size) const;
      virtual Data read(Address address, SIZE_T size) const;
      void write(const Data data);
      void write(SIZE_T offset, const Data data);
      void write(Address address, const Data data);
      void write(const Address address, SIZE_T size);
      void write(SIZE_T offset, const LPVOID pointer, SIZE_T size);
      virtual void write(Address address, const LPVOID pointer, SIZE_T size);

      void copy(Allocation &allocation);
      void clone(const Allocation &allocation);
   };

   class Allocator
   {
      friend Allocation;
      
   public:
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
         const LPVOID pointer;
         const SIZE_T size;

         BadPointerException(Allocator &allocator, Allocation &allocation, const LPVOID pointer, SIZE_T size);
      };

      class InsufficientSizeException : public Exception
      {
      public:
         const SIZE_T size;
         
         InsufficientSizeException(Allocator &allocation, const SIZE_T size);
      };

      class Address : public Neurology::Address
      {
      protected:
         Allocation allocation;

         Address(AddressPool *pool, const Allocation &allocation);
      };
         
      static Allocator Instance;
      
   protected:
      AddressPool addressPool;
      std::map<LPVOID, SIZE_T> memoryPool;
      std::set<Allocation *> allocations;
      std::map<const LPVOID, std::set<Allocation *> > bindings;

   public:
      Allocator(void);
      ~Allocator(void);

      static Allocation &Allocate(SIZE_T size);
      static void Deallocate(Allocation &allocation);

      template <class Type, class ...Args>
      static Object<Type> New(Args... args)
      {
         Object<Type> newObject;
         newObject.construct(args...);
         return newObject;
      }
      
      template <class Type, SIZE_T PointerHint, class ...Args>
      static Object<Type, PointerHint> New(Args... args)
      {
         Object<Type, PointerHint> newObject;
         newObject.construct(args...);
         return newObject;
      }
      
      bool isPooled(const LPVOID pointer) const;
      bool isAllocated(const Allocation &allocation) const;
      bool isBound(const Allocation &allocation) const;

      void throwIfNotPooled(const LPVOID pointer) const;
      void throwIfNotAllocated(const Allocation &allocation) const;
      void throwIfBound(const Allocation &allocation) const;
      void throwIfNotBound(const Allocation &allocation) const;

      SIZE_T bindCount(LPVOID address) const;

      virtual LPVOID pool(SIZE_T size);
      virtual LPVOID repool(LPVOID address, SIZE_T newSize);
      virtual void unpool(LPVOID address);
      
      virtual Allocation &find(const LPVOID address);
      virtual Allocation &null(void);
      virtual Allocation &allocate(SIZE_T size);
      
      template <class Type> Allocation &allocate(void)
      {
         return this->allocate<Type>(sizeof(Type));
      }

      template <class Type> Allocation &allocate(SIZE_T size)
      {
         if (size > sizeof(Type))
            throw InsufficientSizeException(*this, size);

         return this->allocate(size);
      }
      
      virtual void reallocate(Allocation &allocation, SIZE_T size);
      template <class Type> void reallocate(Allocation &allocation)
      {
         if (size > sizeof(Type))
            throw InsufficientSizeException(*this, size);
         
         this->reallocate(allocation, sizeof(Type));
      }
      
      virtual void deallocate(Allocation &allocation);

      virtual Data read(const Allocation &allocation, const LPVOID address, SIZE_T size) const;
      virtual void write(Allocation &allocation, LPVOID address, const LPVOID pointer, SIZE_T size);
      
   protected:
      void bind(Allocation *allocation, LPVOID address);
      void rebind(Allocation *allocation, LPVOID newAddress);
      void unbind(Allocation *allocation);
   };

   template <class Type, SIZE_T PointerHint=0>
   using LocalObject = Allocator::Object<Type, PointerHint, Allocator::Instance>;
}
