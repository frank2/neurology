#pragma once

#include <windows.h>

#include <map>
#include <set>
#include <vector>

#include <neurology/exception.hpp>
#include <neurology/object.hpp>

namespace Neurology
{
   typedef std::vector<BYTE> Data;
#define VarData(var) Data(static_cast<LPBYTE>(&(var)), static_cast<LPBYTE>((&(var))+1))
#define PointerData(ptr) Data(static_cast<LPBYTE>(ptr), static_cast<LPBYTE>((ptr)+1))
#define BlockData(ptr, size) Data(static_cast<LPBYTE>(ptr), static_cast<LPBYTE>(ptr)+size)

   bool CopyData(LPVOID destination, const LPVOID source, SIZE_T size);

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
      Allocator *allocator;
      LPVOID pointer;
      SIZE_T size;

   private:
      Allocation(Allocator *allocator, LPVOID pointer, SIZE_T size);
      
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
      bool inRange(const LPVOID address) const;
      bool inRange(const LPVOID address, SIZE_T size) const;

      void throwIfNoAllocator(void) const;
      void throwIfInvalid(void) const;
      void throwIfNotInRange(SIZE_T offset) const;
      void throwIfNotInRange(SIZE_T offset, SIZE_T size) const;
      void throwIfNotInRange(const LPVOID address) const;
      void throwIfNotInRange(const LPVOID address, SIZE_T size) const;
      
      LPVOID address(void);
      const LPVOID address(void) const;
      LPVOID address(SIZE_T offset);
      const LPVOID address(SIZE_T offset) const;
      LPVOID start(void);
      const LPVOID start(void) const;
      LPVOID end(void);
      const LPVOID end(void) const;

      template <class Type> Type *cast(void)
      {
         return this->cast<Type>(0);
      }

      template <class Type> const Type *cast(void) const
      {
         return this->cast<Type>(0);
      }

      template <class Type> Type *cast(SIZE_T offset)
      {
         if (sizeof(Type) > this->size)
            throw InsufficientSizeException(*this, sizeof(Type));
         else if (sizeof(Type)+offset > this->size)
            throw OffsetOutOfRangeException(*this, offset, sizeof(Type));

         return static_cast<Type *>(this->address(offset));
      }

      template <class Type> const Type *cast(SIZE_T offset) const
      {
         if (sizeof(Type) > this->size)
            throw InsufficientSizeException(*this, sizeof(Type));
         else if (sizeof(Type)+offset > this->size)
            throw OffsetOutOfRangeException(*this, offset, sizeof(Type));

         return const_cast<const Type *>(static_cast<Type *>(this->address(offset)));
      }

      SIZE_T getSize(void) const;

      void allocate(SIZE_T size);
      void reallocate(SIZE_T size);
      void deallocate(void);

      template <class Type> void allocate(SIZE_T size)
      {
         if (size < sizeof(Type))
            throw InsufficientSizeException(*this, size);

         this->allocate(size);
      }

      template <class Type> void allocate(void)
      {
         this->allocate<Type>(sizeof(Type));
      }

      template <class Type> void reallocate(SIZE_T size)
      {
         if (size < sizeof(Type))
            throw InsufficientSizeException(*this, size);

         this->reallocate(size);
      }

      template <class Type> void reallocate(void)
      {
         this->reallocate<Type>(sizeof(Type));
      }
      
      Data read(void) const;
      Data read(SIZE_T size) const;
      Data read(SIZE_T offset, SIZE_T size) const;
      Data read(const LPVOID address, SIZE_T size) const;
      void write(const Data data);
      void write(SIZE_T offset, const Data data);
      void write(LPVOID address, const Data data);
      void write(const LPVOID pointer, SIZE_T size);
      void write(SIZE_T offset, const LPVOID pointer, SIZE_T size);
      void write(LPVOID address, const LPVOID pointer, SIZE_T size);

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

      template <class Type, SIZE_T PointerHint=0, Allocator &AllocatorInstance=Allocator::Instance>
      class Object : public Neurology::Object<Type>
      {
      public:
         typedef Neurology::Object<Type> ObjectBase;
         
         class Exception : public ObjectBase::Exception
         {
         public:
            Exception(const ObjectBase &object, const LPWSTR message)
               : ObjectBase::Exception(object, message)
            {
            }
         };

         class BadPointerHintException : public Exception
         {
         public:
            BadPointerHintException(const ObjectBase &object)
               : Exception(object, EXCSTR(L"Pointer hint must be zero when constructing from a value type."))
            {
            }
         };
         
      protected:
         Allocation &allocation;
         
      public:
         Object(void)
            : ObjectBase()
            , allocation(AllocatorInstance.null())
         {
            if (PointerHint == 0)
               this->allocation.allocate<Type>();
            else
               this->allocation.allocate<Type>(PointerHint);
         }

         Object(SIZE_T size)
            : ObjectBase()
            , allocation(AllocatorInstance.allocate(size))
         {
         }

         Object(Object &object)
            : ObjectBase()
            , allocation(AllocatorInstance.null())
         {
            *this = object;
         }

         Object(const Object *object)
            : ObjectBase()
            , allocation(AllocatorInstance.null())
         {
            *this = object;
         }

         Object(const Type &value)
            : ObjectBase()
            , allocation(AllocatorInstance.null())
         {
            if (PointerHint != 0)
               throw BadPointerHintException(*this);

            this->allocation.allocate<Type>();
            this->construct(value);
         }

         Object(const Type *value)
            : ObjectBase()
            , allocation(AllocatorInstance.null())
         {
            if (PointerHint == 0)
               this->allocation.allocate<Type>();
            else
               this->allocation.allocate<Type>(PointerHint);

            this->construct(*value);
         }

         Object(const Type *value, SIZE_T size)
            : ObjectBase()
            , allocation(AllocatorInstance.null())
         {
            this->allocation.allocate<Type>(size);
            this->assign(value, size);
            
            /* we assume that if you're passing a pointer with a size, it's been constructed. */
            this->built = true;
         }

         ~Object(void)
         {
            if (!this->allocation.isValid())
               return;

            /* this allocation is the allocator's last binding. prepare to die. */
            if (this->built && AllocatorInstance.bindCount(this->allocation.address()) == 1)
               this->destruct();

            this->allocation.deallocate();
         }

         /* I dunno why but C++ wants to point at Neurology::Object when returning these types... probably some kind of
            scoping issue where it doesn't look to the internal class lol */
         template <class ... Args> static Allocator::Object<Type, PointerHint, AllocatorInstance> New(Args... args)
         {
            Allocator::Object<Type, PointerHint, AllocatorInstance> newObject;
            newObject.construct(args...);
            return newObject;
         }
         
         template <SIZE_T AllocationSize, class ... Args> static Allocator::Object<Type, PointerHint, AllocatorInstance> New(Args... args)
         {
            Allocator::Object<Type, PointerHint, AllocatorInstance> newObject(AllocationSize);
            newObject.construct(args...);
            return newObject;
         }

         void operator=(Object &object)
         {
            this->allocation.copy(object.allocation);
            this->built = object.built;
         }

         void operator=(const Object *object)
         {
            this->allocation.clone(object->allocation);
            this->built = object->built;
         }

         void operator=(const Type &value)
         {
            if (!this->built)
            {
               if (!this->allocation.isValid())
                  this->allocation.allocate<Type>();
               
               this->construct(value);
            }
            else
               this->reassign(&value);
         }

         void operator=(const Type *value)
         {
            if (!this->built)
            {
               if (!this->allocation.isValid())
                  this->allocation.allocate<Type>();
               
               this->construct(*value);
            }
            else
               this->reassign(*value, sizeof(Type));
         }

         void assign(const Type *pointer)
         {
            if (this->allocation.isValid())
               ObjectBase::assign(pointer, this->allocation.getSize());
            else
            {
               if (PointerHint == 0)
                  ObjectBase::assign(pointer, sizeof(Type));
               else
                  ObjectBase::assign(pointer, PointerHint);
            }
         }
         
         void assign(const LPVOID pointer, SIZE_T size)
         {
            this->allocation.throwIfInvalid();
            
            if (size > this->allocation.getSize())
               throw Allocation::InsufficientSizeException(this->allocation, size);

            this->allocation.write(pointer, size);
         }

         virtual void reassign(const Type *pointer)
         {
            /* C++ doesn't recognize that it's fucking inhereted the functions
               of its parent for some reason... sigh. */
            if (this->allocation.isValid())
               ObjectBase::reassign(pointer, this->allocation.getSize());
            else
            {
               if (PointerHint == 0)
                  ObjectBase::reassign(pointer, sizeof(Type));
               else
                  ObjectBase::reassign(pointer, PointerHint);
            }
         }

         virtual void reassign(const LPVOID pointer, SIZE_T size)
         {
            if (!this->allocation.isValid())
               this->allocation = AllocatorInstance.allocate<Type>(size);
            else if (this->allocation.getSize() != size)
               this->allocation.reallocate<Type>(size);

            this->allocation.write(pointer, size);
         }

         virtual Type *pointer(void)
         {
            this->allocation.throwIfInvalid();

            return this->allocation.cast<Type>();
         }

         virtual const Type *pointer(void) const
         {
            this->allocation.throwIfInvalid();

            return this->allocation.cast<Type>();
         }
      };
         
      static Allocator Instance;
      
   protected:
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
