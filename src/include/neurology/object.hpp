#pragma once

#include <windows.h>

#include <new>
#include <type_traits>

#include <neurology/allocators/local.hpp>
#include <neurology/allocators/void.hpp>
#include <neurology/exception.hpp>

namespace Neurology
{
   template <class Type, Allocator *AllocatorClass = &LocalAllocator::Instance>
   class Object
   {
   public:
      typedef Type BaseType;
      typedef Type *PointedType;
      typedef typename std::remove_pointer<Type>::type UnpointedType;
      typedef typename std::remove_extent<Type>::type ParentExtentType;
      typedef typename std::remove_all_extents<Type>::type ArrayBaseType;
      
      class Exception : public Neurology::Exception
      {
      public:
         const Object &object;

         Exception(const Object &object, const LPWSTR message)
            : Neurology::Exception(message)
            , object(object)
         {
         }
      };

      class VoidObjectException : public Exception
      {
      public:
         VoidObjectException(const Object &object)
            : Exception(object, EXCSTR(L"The object is void."))
         {
         }
      };

      class ConstructionException : public Exception
      {
      public:
         ConstructionException(const Object &object, const LPWSTR message)
            : Exception(object, message)
         {
         }
      };

      class AlreadyConstructedException : public ConstructionException
      {
      public:
         AlreadyConstructedException(const Object &object)
            : ConstructionException(object, EXCSTR(L"Object already constructed."))
         {
         }
      };

      class NeverConstructedException : public ConstructionException
      {
      public:
         NeverConstructedException(const Object &object)
            : ConstructionException(object, EXCSTR(L"Object never constructed."))
         {
         }
      };

   protected:
      Allocator *allocator;
      Allocation allocation;
      bool built;

   public:
      Object(void)
         : built(false)
         , allocator(AllocatorClass)
      {
      }

      Object(const Type value)
         : built(true)
         , allocator(AllocatorClass)
      {
         this->assign(value);
      }

      Object(const Type &value)
         : built(true)
         , allocator(AllocatorClass)
      {
         this->assign(value);
      }

      Object(const Type *pointer)
         : built(true)
         , allocator(AllocatorClass)
      {
         this->assign(pointer, sizeof(Type));
      }

      Object(const Type *pointer, SIZE_T size)
         : built(true)
         , allocator(AllocatorClass)
      {
         this->assign(pointer, size);
      }
      
      Object(Object &object)
         : built(object.built)
         , allocator(AllocatorClass)
      {
         *this = object;
      }

      Object(const Object *object)
         : built(object->built)
         , allocator(AllocatorClass)
      {
         *this = object;
      }

      ~Object(void)
      {
         if (this->built)
            this->destruct();
         
         if (this->allocation.isValid())
            this->allocation.deallocate();
      }
      
      void operator=(Object &object)
      {
         this->allocator = object.allocator;
         this->allocation.copy(object.allocation);
         this->built = object.built;
      }

      void operator=(const Object *object)
      {
         this->allocator = object->allocator;

         if (this->allocation.isValid())
            this->allocation.deallocate();

         if (object->allocation.isValid())
         {
            this->allocation = this->allocator->null();
            this->allocation.clone(object->allocation);
         }
      }

      void operator=(Type type)
      {
         this->assign(type);
      }

      void operator=(Type &type)
      {
         this->assign(type);
      }

      void operator=(Type *type)
      {
         this->assign(type);
      }

      Type &operator*(void)
      {
         return this->reference();
      }

      const Type &operator*(void) const
      {
         return this->reference();
      }

      Type *operator->(void)
      {
         return this->pointer();
      }

      const Type *operator->(void) const
      {
         return this->pointer();
      }
      
      virtual void assign(const Type &value)
      {
         this->assign(&value, sizeof(Type));
      }

      virtual void assign(const Type *pointer)
      {
         this->assign(pointer, sizeof(Type));
      }

      virtual void assign(const Type *pointer, SIZE_T size)
      {
         this->assign(const_cast<const LPVOID>(
                         static_cast<LPVOID>(
                            const_cast<Type *>(pointer))), size);
      }

      virtual void assign(const LPVOID pointer, SIZE_T size)
      {
         Data data;
         
         if (!this->allocation.isValid())
            this->allocation = this->allocator.allocate(size);

         data = BlockData(const_cast<LPVOID>(pointer), size);
         
         if (!this->built)
         {
            try
            {
               /* should pass by value or pass by reference, depending on which is there */
               this->construct(*reinterpret_cast<Type *>(data.data()));
            }
            catch (std::exception &exception)
            {
               UNUSED(exception);

               /* maybe it accepts a pointer for copy? try that and throw if it fails. */
               this->construct(reinterpret_cast<Type *>(data.data()));
            }
         }
         else
            this->allocation.write(BlockData(const_cast<LPVOID>(pointer), size));
      }

      virtual void reassign(const Type &value)
      {
         this->reassign(&value, sizeof(Type));
      }

      virtual void reassign(const Type *pointer)
      {
         this->reassign(pointer, sizeof(Type));
      }

      virtual void reassign(const Type *pointer, SIZE_T size)
      {
         this->reassign(const_cast<const LPVOID>(
                           static_cast<LPVOID>(
                              const_cast<Type *>(pointer))), size);
      }

      virtual void reassign(const LPVOID pointer, SIZE_T size)
      {
         if (!this->allocation.isValid())
            this->allocation = this->allocator.allocate(size);
         else
            this->allocation.reallocate(size);
         
         this->allocation.write(BlockData(const_cast<LPVOID>(pointer), size));
      }

      virtual Type *pointer(void)
      {
         this->allocation.throwIfInvalid();
         return reinterpret_cast<Type *>(this->allocation.address().label());
      }

      virtual const Type *pointer(void) const
      {
         this->allocation.throwIfInvalid();
         
         return const_cast<const Type *>(
            reinterpret_cast<Type *>(this->allocation.address().label()));
      }

      virtual Type &reference(void)
      {
         return *this->pointer();
      }

      virtual const Type &reference(void) const
      {
         return *this->pointer();
      }

      template <class ... Args>
      void construct(Args... args)
      {
         if (this->built)
            throw AlreadyConstructedException(*this);
         
         new (this->pointer()) Type(args...);
         this->built = true;
      }

      void destruct(void)
      {
         if (!this->built)
            throw NeverConstructedException(*this);
         
         this->pointer()->~Type();
         this->built = false;

         if (this->allocation.isValid())
            this->allocation.deallocate();
      }
   };
}
