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
      typedef typename std::remove_pointer<BaseType>::type UnpointedType;
      typedef typename std::remove_all_extents<BaseType>::type NoExtentType;
      typedef typename std::remove_extent<BaseType>::type ParentExtentType;
      typedef typename std::conditional<
         std::is_array<ParentExtentType>::value,
         ParentExtentType,
         typename std::conditional<
            std::rank<BaseType>::value == 1,
            NoExtentType,
            typename std::conditional<
               std::is_pointer<BaseType>::value && !std::is_void<UnpointedType>::value,
               UnpointedType,
               BaseType>::type>::type>::type IndexType;
      
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

      class NullCacheException : public Exception
      {
      public:
         NullCacheException(const Object &object)
            : Exception(object, EXCSTR(L"No cache is available."))
         {
         }
      };

      class DesyncedCacheException : public Exception
      {
      public:
         DesyncedCacheException(const Object &object)
            : Exception(object, EXCSTR(L"The cache is not synced with the allocation."))
         {
         }
      };

      class ObjectNotCachedException : public Exception
      {
      public:
         ObjectNotCachedException(const Object &object)
            : Exception(object, EXCSTR(L"The object is not cacheing."))
         {
         }
      };

      class NonLocalPointerException : public Exception
      {
      public:
         NonLocalPointerException(const Object &object)
            : Exception(object, EXCSTR(L"A local pointer was requested on an object allocated on an uncached remote allocation."))
         {
         }
      };
      
   protected:
      Allocator *allocator;
      Allocation allocation;
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
      
      Object(Object &object)
         : built(object.built)
         , cached(object.cached)
         , autoflush(object.autoflush)
         , allocator(object.allocator)
      {
         *this = object;
      }

      Object(Allocator *allocator, Allocation allocation, Data cache, bool built, bool cached, bool autoflush)
         : allocator(allocator)
         , allocation(allocation)
         , cache(cache)
         , built(built)
         , cached(cached)
         , autoflush(autoflush)
      {
      }

      ~Object(void)
      {
         if (this->cached)
            this->flush();
               
         if (this->built)
            this->destruct();
         
         if (this->allocation.isBound())
            this->allocation.deallocate();
      }

      template <typename... Args>
      static Object New(Args... args)
      {
         Object obj;
         obj.construct(args...);
         return obj;
      }
      
      void operator=(Object &object)
      {
         this->allocator = object.allocator;
         this->allocation.copy(object.allocation);
         this->built = object.built;
         this->cached = object.cached;
         this->cache = object.cache;
      }

      void operator=(const BaseType &type)
      {
         this->assign(type);
      }

      void operator=(const PointedType type)
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

      bool isBuilt(void) const noexcept
      {
         return this->built;
      }

      bool isCached(void) const noexcept
      {
         return this->cached;
      }

      bool willAutoflush(void) const noexcept
      {
         return this->autoflush;
      }

      void setAllocator(Allocator *allocator)
      {
         if (this->allocation.isBound())
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
         /* I hate const correctness so much... */
         this->assign(reinterpret_cast<LPVOID>(
                         const_cast<BaseType *>(&value)), sizeof(BaseType));
      }

      void assign(const PointedType pointer)
      {
         this->assign(reinterpret_cast<const LPVOID>(pointer), sizeof(Type));
      }

      void assign(const PointedType pointer, SIZE_T size)
      {
         this->assign(reinterpret_cast<const LPVOID>(pointer), size);
      }

      virtual void assign(const LPVOID pointer, SIZE_T size)
      {
         Data data;
         
         if (!this->allocation.isBound())
            this->allocate(size);

         data = BlockData(const_cast<LPVOID>(pointer), size);
         
         if (!this->built && std::is_copy_constructible<BaseType>::value)
            this->construct(*reinterpret_cast<PointedType>(data.data()));
         else if (this->cached)
            this->cache = data;
         else
            this->allocation.write(data);

         if (!this->built) // well that's wrong.
            this->built = true;

         if (this->cached && this->autoflush)
            this->flush();
      }

      void reassign(const BaseType &value)
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
         
         if (!this->allocation.isBound())
            return this->assign(pointer, size);
         
         this->reallocate(size);
         data = BlockData(const_cast<LPVOID>(pointer), size);
         
         if (this->cached)
            this->cache = data;
         else
            this->allocation.write(data);

         if (this->cached && this->autoflush)
            this->flush();
      }

      virtual PointedType pointer(void)
      {
         if (!this->cached)
         {
            this->allocation.throwIfNotBound();

            if (!this->allocation.isLocal())
               throw NonLocalPointerException(*this);
         
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
            this->allocation.throwIfNotBound();
         
            if (!this->allocation.isLocal())
               throw NonLocalPointerException(*this);

            return const_cast<const PointedType>(
               reinterpret_cast<PointedType>(this->allocation.address().pointer()));
         }
         
         if (this->cache.size() == 0 && this->allocation.size() == 0)
            throw NullCacheException(*this);

         if (this->cache.size() != this->allocation.size())
            throw DesyncedCacheException(*this);
            
         return const_cast<const PointedType>(
            reinterpret_cast<PointedType>(
               const_cast<LPBYTE>(this->cache.data())));
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
         /* nothing to be done for array types */
         if (std::is_array<BaseType>::value)
            return;
         
         if (this->built)
            throw AlreadyConstructedException(*this);

         /* perhaps you are constructing this prior to allocating it */
         if (!this->allocation.isBound())
            this->allocate();
         
         new(this->pointer()) BaseType(args...);
         this->built = true;
      }

      void destruct(void)
      {
         if (!this->built)
            throw NeverConstructedException(*this);
         
         this->pointer()->~BaseType();
         this->built = false;

         if (this->allocation.isBound())
            this->allocation.deallocate();

         this->cache.empty();
      }

      void allocate(void)
      {
         this->allocate(sizeof(BaseType));
      }

      void allocate(SIZE_T size)
      {
         this->allocation = this->allocator->allocate(size);
      }

      void reallocate(SIZE_T size)
      {
         if (!this->allocation.isBound())
            return this->allocate(size);

         this->allocation.reallocate(size);
      }
   };
}
