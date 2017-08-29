#pragma once

#include <windows.h>

#include <neurology/allocators/allocator.hpp>
#include <neurology/exception.hpp>

namespace Neurology
{
   template <class Type, SIZE_T PointerHint=0, Allocator &AllocatorObject = Allocator::Instance>
   class Object
   {
   protected:
      Allocation allocation;

   public:
      Object(void)
      {
         if (PointerHint == 0)
            this->allocation = AllocatorObject.allocate<Type>();
         else
            this->allocation = AllocatorObject.allocate<Type>(PointerHint);
      }

      Object(Type &value)
      {
         if (PointerHint != 0)
            throw BadPointerHintException(*this);

         this->assign(value);
      }

      Object(const Type &value)
      {
         if (PointerHint != 0)
            throw BadPointerHintException(*this);

         this->assign(value);
      }

      Object(const Type *pointer)
      {
         if (PointerHint == 0)
            this->assign(pointer, sizeof(Type));
         else
            this->assign(pointer, PointerHint);
      }

      Object(const Type *pointer, SIZE_T size)
      {
         this->assign(pointer, size);
      }
      
      Object(Object &object)
      {
         *this = object;
      }
      
      Object(const Object &object)
      {
         *this = object;
      }

      virtual void operator=(Object &object)
      {
         this->allocation.copy(object.allocation);
      }
      
      virtual void operator=(const Object &object)
      {
         this->allocation.clone(object.allocation);
      }

      virtual void operator=(Type &value)
      {
         this->assign(value);
      }

      virtual void operator=(const Type &value)
      {
         this->assign(value);
      }

      virtual void operator=(const Type *pointer)
      {
         this->assign(pointer);
      }

      virtual Type operator*(void)
      {
         return this->resolve();
      }

      virtual const Type operator*(void)
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

      virtual bool operator==(const Type right) const
      {
         return **this == right;
      }

      virtual bool operator!=(const Type right) const
      {
         return **this != right;
      }

      virtual bool operator>(const Type right) const
      {
         return **this > right;
      }

      virtual bool operator<(const Type right) const
      {
         return **this < right;
      }

      virtual bool operator>=(const Type right) const
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

      virtual bool operator&&(const Type right) const
      {
         return **this && right;
      }

      virtual bool operator||(const Type right) const
      {
         return **this || right;
      }

      virtual Type operator~(void) const
      {
         return ~**this;
      }

      virtual Type operator&(const Type right) const
      {
         return **this & right;
      }

      virtual Type operator|(const Type right) const
      {
         return **this | right;
      }

      virtual Type operator^(const Type right) const
      {
         return **this ^ right;
      }

      virtual Type operator<<(const Type right) const
      {
         return **this << right;
      }

      virtual Type operator>>(const Type right) const
      {
         return **this >> right;
      }

      virtual Type &operator+=(const Type right)
      {
         **this = **this + right;
         return **this;
      }

      virtual Type &operator-=(const Type right)
      {
         **this = **this - right;
         return **this;
      }

      virtual Type &operator*=(const Type right)
      {
         **this = **this * right;
         return **this;
      }

      virtual Type &operator/=(const Type right)
      {
         **this = **this / right;
         return **this;
      }

      virtual Type &operator%=(const Type right)
      {
         **this = **this % right;
         return **this;
      }

      virtual Type &operator&=(const Type right)
      {
         **this = **this & right;
         return **this;
      }

      virtual Type &operator|=(const Type right)
      {
         **this = **this | right;
         return **this;
      }

      virtual Type &operator^=(const Type right)
      {
         **this = **this ^ right;
         return **this;
      }

      virtual Type &operator<<=(const Type right)
      {
         **this = **this << right;
         return **this;
      }

      virtual Type &operator>>=(const Type right)
      {
         **this = **this >> right;
         return **this;
      }

      virtual Type &operator[](const int index)
      {
         return **this[index];
      }

      virtual const Type &operator[](const int index) const
      {
         return **this[index];
      }

      virtual Type *operator->(void)
      {
         return this->pointer();
      }

      virtual const Type *operator->(void) const
      {
         return this->pointer();
      }
      
      void assign(const Type &value)
      {
         this->assign(const_cast<LPVOID>(&value), sizeof(Type));
      }

      void assign(const Type *pointer)
      {
         if (!this->hasData())
         {
            if (PointerHint == 0)
               this->assign(const_cast<LPVOID>(pointer), sizeof(Type));
            else
               this->assign(const_cast<LPVOID>(pointer), PointerHint);
         }
         else
            this->assign(const_cast<LPVOID>(pointer), this->size);
      }

      void reassign(const Type &value)
      {
         this->reassign(const_cast<LPVOID>(&value), sizeof(Type));
      }

      void reassign(const Type *pointer)
      {
         if (!this->hasData())
         {
            if (PointerHint == 0)
               this->reassign(const_cast<LPVOID>(pointer), sizeof(Type));
            else
               this->reassign(const_cast<LPVOID>(pointer), PointerHint);
         }
         else
            this->reassign(const_cast<LPVOID>(pointer), this->size);
      }

      Type *pointer(void)
      {
         this->throwIfNoData();
         
         return static_cast<Type *>(this->deref());
      }

      const Type *pointer(void) const
      {
         this->throwIfNoData();

         return const_cast<Type *>(this->deref());
      }

      Type resolve(void)
      {
         return *this->pointer();
      }

      const Type resolve(void) const
      {
         return *this->pointer();
      }
   };

   template <class Type, SIZE_T PointerHint=0>
   class LocalObject : public Object<Type, PointerHint, Allocator::Instance>
   {
   };
}
