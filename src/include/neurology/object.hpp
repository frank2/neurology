#pragma once

#include <windows.h>

#include <new>
#include <type_traits>

#include <neurology/allocators/local.hpp>
#include <neurology/allocators/void.hpp>
#include <neurology/exception.hpp>

namespace Neurology
{
   template <class Type>
   class Object
   {
   public:
      typedef typename std::conditional<std::is_void<Type>::value
                                        ,Type *
                                        ,Type>::type BaseType;
      typedef BaseType *PointedType;
      typedef typename std::remove_pointer<Type>::type UnpointedType;
      typedef typename std::remove_all_extents<BaseType>::type NoExtentType
      typedef typename std::remove_extent<BaseType>::type ParentExtentType;
      
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
      Offset allocationOffset;
      Data cache;
      bool built;
      bool cached;
      bool autoflush;

   public:
      Object(void)
         : built(false)
         , cached(false)
         , autoflush(false)
         , allocator(&LocalAllocator::Instance)
      {
      }

      Object(const BaseType &value)
         : built(true)
         , cached(false)
         , autoflush(false)
         , allocator(&LocalAllocator::Instance)
      {
         this->assign(value);
      }

      Object(const PointedType pointer)
         : built(true)
         , cached(false)
         , autoflush(false)
         , allocator(&LocalAllocator::Instance)
      {
         this->assign(pointer, sizeof(BaseType));
      }

      Object(const PointedType pointer, SIZE_T size)
         : built(true)
         , cached(false)
         , autoflush(false)
         , allocator(&LocalAllocator::Instance)
      {
         this->assign(pointer, size);
      }
      
      Object(const Object &object)
         : built(object.built)
         , cached(object.cached)
         , autoflush(object.autoflush)
         , allocator(object.allocator)
      {
         *this = object;
      }

      ~Object(void)
      {
         if (this->cached)
            this->flush();
               
         if (this->built)
            this->destruct();
         
         if (this->allocation.isValid())
            this->allocation.deallocate();
      }
      
      void operator=(const Object &object)
      {
         this->allocator = object.allocator;
         this->allocation.copy(object.allocation);
         this->built = object.built;
         this->cached = object.cached;
         this->cache = object.cache;
      }

      operator=(const BaseType &type)
      {
         this->assign(type);
      }

      operator=(const PointedType type)
      {
         this->assign(type);
      }

      BaseType &operator*(void)
      {
         return this->reference();
      }

      const BaseType &operator*(void) const
      {
         return this->reference();
      }
      
      PointedType operator->(void)
      {
         return this->pointer();
      }

      const PointedType operator->(void) const
      {
         return this->pointer();
      }

      /* hi welcome to hell, we'll start with the fourth layer, which is an
         array of arrays */
      typename std::enable_if<std::is_array<ParentExtentType>::value
                              ,Object<ParentExtentType> >::type
      operator[] (unsigned int index)
      {
         Object<ParentExtentType> result;
         SIZE_T parentExSize = sizeof(ParentExtentType);

         this->allocation.throwIfInvalid();

         result.allocation = this->allocation;
         result.offset = Offset(this->offset.address(), parentExSize*index);
         result.cached = this->cached;
         result.autoflush = this->autoflush;
         result.built = this->built;

         if (this->cached)
            result.cache = this->allocation.read(result.offset.address(), parentExSize);

         return result;
      }

      /* the next layer is an array to the base object */
      typename std::enable_if<std::rank<BaseType>::value == 1
                              ,Object<NoExtentType> >::type
      operator[] (unsigned int index)
      {
         Object<NoExtentType> result;
         SIZE_T noExSize = sizeof(NoExtentType);

         this->allocation.throwIfInvalid();

         result.allocation = this->allocation;
         result.offset = Offset(this->offset.address(), noExSize*index);
         result.cached = this->cached;
         result.autoflush = this->autoflush;
         result.built = this->built;

         if (this->cached)
            result.cache = this->allocation.read(result.offset.address(), noExSize);
      }

      /* the next layer is pointer scripting where the pointer is another pointer
         P O I N T C E P T I O N */
      typename std::enable_if<std::is_pointer<UnpointedType>::value
                              ,Object<UnpointedType> >::type
      operator[] (unsinged int index)
      {
         Object<UnpointedType> result;
         SIZE_T size = sizeof(UnpointedType);

         this->allocation.throwIfInvalid();

         result.allocation = this->allocation;
         result.offset = Offset(this->offset.address(), size*index);
         result.cached = this->cached;
         result.autoflush = this->autoflush;
         result.built = this->built;

         if (this->cached)
            result.cache = this->allocation.read(result.offset.address(), size);
      }

      /* the final layer is pointer arithmetic via array subscripting */
      typename std::enable_if<std::is_pointer<BaseType>::value &&
                              !std::is_pointer<UnpointedType>::value &&
                              !std::is_void<UnpointedType>::value
                              ,Object<UnpointedType> >::type
      operator[] (unsigned int index)
      {
         Object<UnpointedType> result;
         SIZE_T size = sizeof(UnpointedType);

         this->allocation.throwIfInvalid();

         result.allocation = this->allocation;
         result.offset = Offset(this->offset.address(), size*index);
         result.cached = this->cached;
         result.autoflush = this->autoflush;
         result.built = this->built;

         if (this->cached)
            result.cache = this->allocation.read(result.offset.address(), size);
      }

      bool isBuilt(void) const
      {
         return this->built;
      }

      bool isCached(void) const
      {
         return this->cached;
      }

      bool willAutoflush(void) const
      {
         return this->autoflush;
      }

      void setAllocator(Allocator *allocator)
      {
         if (this->allocation.isValid())
            throw HotSwapException(*this);
         
         this->allocator = allocator;
      }

      void setCacheing(bool cached)
      {
         this->cached = cached;
      }

      void setAutoflush(bool autoflush)
      {
         this->autoflush = autoflush;
      }

      void assign(const BaseType &value)
      {
         this->assign(&value, sizeof(BaseType));
      }

      void assign(const PointedType pointer)
      {
         this->assign(pointer, sizeof(Type));
      }

      void assign(const PointedType pointer, SIZE_T size)
      {
         this->assign(reinterpret_castt<const LPVOID>(pointer), size);
      }

      virtual void assign(const LPVOID pointer, SIZE_T size)
      {
         Data data;
         
         if (!this->allocation.isValid())
            this->allocate(size);

         data = BlockData(const_cast<LPVOID>(pointer), size);
         
         if (!this->built && std::is_copy_constructible<BaseType>::value)
            this->construct(*reinterpret_cast<PointedType>(data.data()));
         else if (this->built && std::is_copy_assignable<BaseType>::value)
            **this = *reinterpret_cast<PointedType>(data.data());
         else if (this->cached)
            this->cache = data;
         else
            this->allocation.write(data);

         if (!this->built) // well that's wrong.
            this->built = true;

         if (this->cached && this->autoflush)
            this->flush();
      }

      reassign(const BaseType &value)
      {
         this->reassign(&value, sizeof(Type));
      }

      void reassign(const PointedType pointer)
      {
         this->reassign(pointer, sizeof(Type));
      }

      void reassign(const PointedType pointer, SIZE_T size)
      {
         this->reassign(const_cast<const LPVOID>(
                           static_cast<LPVOID>(
                              const_cast<PointedType>(pointer))), size);
      }

      virtual void reassign(const LPVOID pointer, SIZE_T size)
      {
         Data data;
         
         if (!this->allocation.isValid())
            return this->assign(pointer, size);
         
         this->reallocate(size);
         data = BlockData(const_cast<LPVOID>(pointer), size);
         
         if (std::is_copy_assignable<BaseType>::value)
            **this = *reinterpret_cast<PointedType>(data.data());
         else if (this->cached)
            this->cache = data;
         else
            this->allocation.write(data);

         if (this->cached && this->autoflush)
            this->flush();
      }

      virtual typename std::enable_if<!std::is_pointer<BaseType>::value
                                      ,PointedType>::type
      pointer(void)
      {
         if (!this->cached)
         {
            /* most of the time, we won't be cached */
            this->allocation.throwIfInvalid();
            return reinterpret_cast<PointedType>(this->allocation.address().pointer());
         }

         if (this->cache.size() == 0 && this->allocation.size() == 0)
            throw NullCacheException(*this);
         
         if (this->cache.size() != this->allocation.size())
            this->cache.resize(this->allocation.size());
         
         return reinterpret_cast<PointedType>(this->cache.data());
      }

      virtual const PointedType pointer(void) const
      {
         if (!this->cached)
         {
            this->allocation.throwIfInvalid();
         
            return const_cast<const PointedType>(
               reinterpret_cast<PointedType>(this->allocation.address().pointer()));
         }
         
         if (this->cache.size() == 0 && this->allocation.size() == 0)
            throw NullCacheException(*this);

         if (this->cache.size() != this->allocation.size())
            this->cache.resize(this->allocation.size());
            
         return reinterpret_cast<PointedType>(this->cache.data());
      }

      virtual BaseType &reference(void)
      {
         return *this->pointer();
      }

      virtual const BaseType &reference(void) const
      {
         return *this->pointer();
      }

      void flush(void)
      {
         if (!this->cached)
            throw ObjectNotCachedException(*this);

         if (this->cache.size() == 0)
            throw NullCacheException(*this);

         this->allocation.write(this->cache);
      }

      void update(void)
      {
         if (!this->cached)
            throw ObjectNotCachedException(*this);

         this->cache = this->allocation.read();
      }

      template <class ... Args>
      void construct(Args... args)
      {
         /* nothing to be done for array/pointer types */
         if (std::is_pointer<BaseType>::value || std::is_array<BaseType>::value)
            return;
         
         if (this->built)
            throw AlreadyConstructedException(*this);
         
         new(this->pointer()) BaseType(args...);
         this->built = true;
      }

      void destruct(void)
      {
         if (!this->built)
            throw NeverConstructedException(*this);
         
         this->pointer()->~BaseType();
         this->built = false;

         if (this->allocation.isValid())
            this->allocation.deallocate();

         this->cache.empty();
      }

   protected:
      void allocate(SIZE_T size)
      {
         this->allocation = this->allocator->allocate(size);
         this->offset.setAddress(this->allocation.address());
         this->offset.setOffset(0);
      }

      void reallocate(SIZE_T size)
      {
         if (!this->allocation.isValid())
            return this->allocate(size);

         this->allocation.reallocate(size);
         this->offset.setAddress(this->allocation.address());
         this->offset.setOffset(0);
      }
   };
}
