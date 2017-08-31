#pragma once

#include <windows.h>

#include <new>
#include <type_traits>

#include <neurology/exception.hpp>

namespace Neurology
{
   template<class Type, class ReturnType=Type, class IndexType=int>
   struct has_index_op
   {
   public:
      template<typename TestType, ReturnType & (TestType::*)(IndexType)> struct SFINAE {};
      template<typename U> static char Test(SFINAE<U, &U::operator[]>*);
      template<typename U> static int Test(...);
      static const bool value = sizeof(Test<Type>(0)) == sizeof(char);
   };
   
   template <class Type>
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
      bool built;

   public:
      Object(void)
         : built(false)
      {
      }

      Object(SIZE_T size)
         : built(false)
      {
         throw VoidObjectException(*this);
      }

      Object(const Type value)
         : built(true)
      {
         this->reassign(value);
      }

      Object(const Type &value)
         : built(true)
      {
         this->reassign(value);
      }

      Object(const Type *pointer)
         : built(true)
      {
         this->reassign(pointer, sizeof(Type));
      }

      Object(const Type *pointer, SIZE_T size)
         : built(true)
      {
         this->reassign(pointer, size);
      }
      
      Object(Object &object)
         : built(object.built)
      {
         *this = object;
      }

      Object(const Object *object)
         : built(object->built)
      {
         *this = object;
      }

      ~Object(void)
      {
      }
      
      void operator=(Object &object)
      {
         throw VoidObjectException(*this);
      }

      void operator=(const Object *object)
      {
         throw VoidObjectException(*this);
      }

      Type &operator*(void)
      {
         return this->resolve();
      }

      const Type &operator*(void) const
      {
         return this->resolve();
      }
      
      Type operator+(const Type &right)
      {
         return **this + right;
      }

      const Type operator+(const Type &right) const
      {
         return **this + right;
      }

      Type operator-(const Type &right)
      {
         return **this - right;
      }

      const Type operator-(const Type &right) const
      {
         return **this - right;
      }

      Type operator*(const Type &right)
      {
         return **this * right;
      }

      const Type operator*(const Type &right) const
      {
         return **this * right;
      }

      Type operator/(const Type &right)
      {
         return **this / right;
      }

      const Type operator/(const Type &right) const
      {
         return **this / right;
      }

      Type operator%(const Type &right)
      {
         return **this % right;
      }

      const Type operator%(const Type &right) const
      {
         return **this % right;
      }

      Type &operator++(void)
      {
         **this = **this + 1;
         return **this;
      }

      Type operator++(int dummy)
      {
         **this = **this + 1;
         return **this;
      }

      Type &operator--(void)
      {
         **this = **this - 1;
         return **this;
      }

      Type operator--(int dummy)
      {
         **this = **this - 1;
         return **this;
      }

      bool operator==(const Type &right) const
      {
         return **this == right;
      }

      bool operator!=(const Type &right) const
      {
         return **this != right;
      }

      bool operator>(const Type &right) const
      {
         return **this > right;
      }

      bool operator<(const Type &right) const
      {
         return **this < right;
      }

      bool operator>=(const Type &right) const
      {
         return **this >= right;
      }

      bool operator<=(const Type right) const
      {
         return **this <= right;
      }

      bool operator!(void) const
      {
         return !**this;
      }

      bool operator&&(const Type &right) const
      {
         return **this && right;
      }

      bool operator||(const Type right) const
      {
         return **this || right;
      }

      Type operator~(void)
      {
         return ~**this;
      }

      const Type operator~(void) const
      {
         return ~**this;
      }

      Type operator&(const Type &right)
      {
         return **this & right;
      }

      const Type operator&(const Type &right) const
      {
         return **this & right;
      }

      Type operator|(const Type &right)
      {
         return **this | right;
      }

      const Type operator|(const Type &right) const
      {
         return **this | right;
      }

      Type operator^(const Type &right)
      {
         return **this ^ right;
      }

      const Type operator^(const Type &right) const
      {
         return **this ^ right;
      }

      Type operator<<(const Type &right)
      {
         return **this << right;
      }

      const Type operator<<(const Type &right) const
      {
         return **this << right;
      }

      Type operator>>(const Type &right)
      {
         return **this >> right;
      }

      const Type operator>>(const Type &right) const
      {
         return **this >> right;
      }

      Type &operator+=(const Type &right)
      {
         **this = **this + right;
         return **this;
      }

      Type &operator-=(const Type &right)
      {
         **this = **this - right;
         return **this;
      }

      Type &operator*=(const Type &right)
      {
         **this = **this * right;
         return **this;
      }

      Type &operator/=(const Type &right)
      {
         **this = **this / right;
         return **this;
      }

      Type &operator%=(const Type &right)
      {
         **this = **this % right;
         return **this;
      }

      Type &operator&=(const Type &right)
      {
         **this = **this & right;
         return **this;
      }

      Type &operator|=(const Type &right)
      {
         **this = **this | right;
         return **this;
      }

      Type &operator^=(const Type &right)
      {
         **this = **this ^ right;
         return **this;
      }

      Type &operator<<=(const Type &right)
      {
         **this = **this << right;
         return **this;
      }

      Type &operator>>=(const Type &right)
      {
         **this = **this >> right;
         return **this;
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
         throw VoidObjectException(*this);
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
         throw VoidObjectException(*this);
      }

      virtual Type *pointer(void)
      {
         throw VoidObjectException(*this);
      }

      virtual const Type *pointer(void) const
      {
         throw VoidObjectException(*this);
      }

      virtual Type &resolve(void)
      {
         return *this->pointer();
      }

      virtual const Type &resolve(void) const
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
      }
   };
}
