#pragma once

#include <windows.h>

#include <map>
#include <set>
#include <vector>

#include <neurology/exception.hpp>

namespace Neurology
{
   typedef std::vector<BYTE> Data;
#define VarData(var) Data(static_cast<LPBYTE>(&(var)), static_cast<LPBYTE>((&(var))+1))
#define PointerData(ptr) Data(static_cast<LPBYTE>(ptr), static_cast<LPBYTE>((ptr)+1))
#define BlockData(ptr, size) Data(static_cast<LPBYTE>(ptr), static_cast<LPBYTE>(ptr)+size)

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

      class BadPointerException : public Exception
      {
      public:
         const LPVOID pointer;
         const SIZE_T size;

         BadPointerException(const Allocation &allocation, const LPVOID pointer, SIZE_T size);
      };

   protected:
      Allocator *allocator;
      LPVOID pointer;
      SIZE_T size;

   private:
      Allocation(Allocator *allocator, LPVOID pointer, SIZE_T size);
      
   public:
      Allocation(void);
      Allocation(Allocator *allocator);
      Allocation(Allocation &allocation);
      Allocation(const Allocation &allocation);
      ~Allocation(void);

      void operator=(Allocation &allocation);
      void operator=(const Allocation &allocation);
      LPVOID operator*(void);
      const LPVOID operator*(void) const;

      bool isValid(void) const;
      bool isBound(void) const;
      bool isNull(void) const;
      bool inRange(const LPVOID address) const;
      bool inRange(const LPVOID address, SIZE_T size) const;

      void throwIfNoAllocator(void) const;
      void throwIfInvalid(void) const;
      
      virtual LPVOID address(void);
      virtual const LPVOID address(void) const;
      virtual LPVOID address(SIZE_T offset);
      virtual const LPVOID address(SIZE_T offset) const;
      LPVOID start(void);
      const LPVOID start(void) const;
      LPVOID end(void);
      const LPVOID end(void) const;

      template <class Type> virtual Type *cast(void)
      {
         return this->cast<Type>(0);
      }

      template <class Type> virtual const Type *cast(void) const
      {
         return this->cast<Type>(0);
      }

      template <class Type> Type *cast(SIZE_T offset)
      {
         if (sizeof(Type) > this->size)
            throw InsufficientSizeException(*this, sizeof(Type));
         else if (sizeof(Type)+offset > this->size)
            throw OffsetOutOfBoundsException(*this, offset, sizeof(Type));

         return static_cast<Type *>(this->address(offset));
      }

      template <class Type> const Type *cast(SIZE_T offset) const
      {
         if (sizeof(Type) > this->size)
            throw InsufficientSizeException(*this, sizeof(Type));
         else if (sizeof(Type)+offset > this->size)
            throw OffsetOutOfBoundsException(*this, offset, sizeof(Type));

         return const_cast<const Type *>(static_cast<Type *>(this->address(offset)));
      }

      SIZE_T getSize(void) const;

      virtual void allocate(SIZE_T size);
      virtual void reallocate(SIZE_T size);
      virtual void deallocate(void);

      template <class Type> void allocate(void)
      {
         this->allocate(sizeof(Type));
      }

      template <class Type> void reallocate(void)
      {
         this->reallocate(sizeof(Type));
      }
      
      Data read(void) const;
      Data read(SIZE_T size) const;
      Data read(SIZE_T offset, SIZE_T size) const;
      virtual Data read(LPVOID address, SIZE_T size) const;
      void write(const Data data);
      void write(SIZE_T offset, const Data data);
      void write(const LPVOID pointer, SIZE_T size);
      void write(SIZE_T offset, const LPVOID pointer, SIZE_T size);
      virtual void write(LPVOID address, const LPVOID pointer, SIZE_T size);
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

      class UnpooledAddressException : public Exception
      {
      public:
         const LPVOID address;
         
         UnpooledAddressException(Allocator &allocator, const LPVOID address);
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

   protected:
      std::map<LPVOID, SIZE_T> memoryPool;
      std::set<Allocation *> allocations;
      std::map<LPVOID, std::set<Allocation *> > bindings;

   public:
      Allocator(void);
      ~Allocator(void);
      
      bool isPooled(const LPVOID pointer) const;
      bool isAllocated(const Allocation &allocation) const;
      bool isBound(const Allocation &allocation) const;

      void throwIfNotPooled(const LPVOID pointer) const;
      void throwIfNotAllocated(const Allocation &allocation) const;
      void throwIfBound(const Allocation &allocation) const;
      void throwIfNotBound(const Allocation &allocation) const;

      virtual LPVOID pool(SIZE_T size);
      virtual LPVOID repool(LPVOID address, SIZE_T newSize);
      virtual void unpool(LPVOID address);
      
      virtual Allocation &find(const LPVOID address);
      virtual Allocation &null(void);
      virtual Allocation &allocate(SIZE_T size);
      virtual void reallocate(Allocation &allocation, SIZE_T size);
      virtual void deallocate(Allocation &allocation);
      
   protected:
      void bind(Allocation *allocation, LPVOID address);
      void rebind(Allocation *allocation, LPVOID newAddress);
      void unbind(Allocation *allocation);
   };
}
