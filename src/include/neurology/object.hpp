#pragma once

#include <windows.h>

#include <new>

#include <neurology/exception.hpp>

namespace Neurology
{
   template <class Type>
   class Object
   {
   public:
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
      
      Object(const Object &object)
         : built(object.built)
      {
         *this = object;
      }

      ~Object(void)
      {
         if (this->built)
            this->destruct();
      }
      
      virtual void operator=(const Object &object)
      {
         *this = *object;
      }

      virtual void operator=(const Type &value)
      {
         **this = value;
      }

      virtual void operator=(const Type *pointer)
      {
         **this = *pointer;
      }

      virtual Type &operator*(void)
      {
         return this->resolve();
      }

      virtual const Type &operator*(void) const
      {
         return this->resolve();
      }
      
      virtual Type operator+(const Type &right)
      {
         return **this + right;
      }

      virtual const Type operator+(const Type &right) const
      {
         return **this + right;
      }

      virtual Type operator-(const Type &right)
      {
         return **this - right;
      }

      virtual const Type operator-(const Type &right) const
      {
         return **this - right;
      }

      virtual Type operator*(const Type &right)
      {
         return **this * right;
      }

      virtual const Type operator*(const Type &right) const
      {
         return **this * right;
      }

      virtual Type operator/(const Type &right)
      {
         return **this / right;
      }

      virtual const Type operator/(const Type &right) const
      {
         return **this / right;
      }

      virtual Type operator%(const Type &right)
      {
         return **this % right;
      }

      virtual const Type operator%(const Type &right) const
      {
         return **this % right;
      }

      virtual Type &operator++(void)
      {
         **this = **this + 1;
         return **this;
      }

      virtual Type operator++(int dummy)
      {
         **this = **this + 1;
         return **this;
      }

      virtual Type &operator--(void)
      {
         **this = **this - 1;
         return **this;
      }

      virtual Type operator--(int dummy)
      {
         **this = **this - 1;
         return **this;
      }

      virtual bool operator==(const Type &right) const
      {
         return **this == right;
      }

      virtual bool operator!=(const Type &right) const
      {
         return **this != right;
      }

      virtual bool operator>(const Type &right) const
      {
         return **this > right;
      }

      virtual bool operator<(const Type &right) const
      {
         return **this < right;
      }

      virtual bool operator>=(const Type &right) const
      {
         return **this >= right;
      }

      virtual bool operator<=(const Type right) const
      {
         return **this <= right;
      }

      virtual bool operator!(void) const
      {
         return !**this;
      }

      virtual bool operator&&(const Type &right) const
      {
         return **this && right;
      }

      virtual bool operator||(const Type right) const
      {
         return **this || right;
      }

      virtual Type operator~(void)
      {
         return ~**this;
      }

      virtual const Type operator~(void) const
      {
         return ~**this;
      }

      virtual Type operator&(const Type &right)
      {
         return **this & right;
      }

      virtual const Type operator&(const Type &right) const
      {
         return **this & right;
      }

      virtual Type operator|(const Type &right)
      {
         return **this | right;
      }

      virtual const Type operator|(const Type &right) const
      {
         return **this | right;
      }

      virtual Type operator^(const Type &right)
      {
         return **this ^ right;
      }

      virtual const Type operator^(const Type &right) const
      {
         return **this ^ right;
      }

      virtual Type operator<<(const Type &right)
      {
         return **this << right;
      }

      virtual const Type operator<<(const Type &right) const
      {
         return **this << right;
      }

      virtual Type operator>>(const Type &right)
      {
         return **this >> right;
      }

      virtual const Type operator>>(const Type &right) const
      {
         return **this >> right;
      }

      virtual Type &operator+=(const Type &right)
      {
         **this = **this + right;
         return **this;
      }

      virtual Type &operator-=(const Type &right)
      {
         **this = **this - right;
         return **this;
      }

      virtual Type &operator*=(const Type &right)
      {
         **this = **this * right;
         return **this;
      }

      virtual Type &operator/=(const Type &right)
      {
         **this = **this / right;
         return **this;
      }

      virtual Type &operator%=(const Type &right)
      {
         **this = **this % right;
         return **this;
      }

      virtual Type &operator&=(const Type &right)
      {
         **this = **this & right;
         return **this;
      }

      virtual Type &operator|=(const Type &right)
      {
         **this = **this | right;
         return **this;
      }

      virtual Type &operator^=(const Type &right)
      {
         **this = **this ^ right;
         return **this;
      }

      virtual Type &operator<<=(const Type &right)
      {
         **this = **this << right;
         return **this;
      }

      virtual Type &operator>>=(const Type &right)
      {
         **this = **this >> right;
         return **this;
      }

      virtual Type *operator->(void)
      {
         return this->pointer();
      }

      virtual const Type *operator->(void) const
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
