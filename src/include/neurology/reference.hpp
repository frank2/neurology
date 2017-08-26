#pragma once

#include <windows.h>

#include <type_traits>
#include <vector>

#include <neurology/exception.hpp>

namespace Neurology
{
   typedef std::vector<BYTE> Data;
#define VarData(var) Data((LPBYTE)(&(var)), (LPBYTE)((&(var))+1))
#define PointerData(ptr) Data((LPBYTE)(ptr), (LPBYTE)((ptr)+1))
#define BlockData(ptr, size) Data((LPBYTE)(ptr), ((LPBYTE)(ptr))+size)

   class VoidReference
   {
   public:
      class Exception : public Neurology::Exception
      {
      public:
         const VoidReference &voidReference;

         Exception(const VoidReference &reference, const LPWSTR message);
      };

      class ReferenceStateException : public VoidReference::Exception
      {
      public:
         ReferenceStateException(const VoidReference &reference, const LPWSTR message);
      };

      class DeadReferenceException : public ReferenceStateException
      {
      public:
         OpenReferenceException(const VoidReference &reference);
      }
         
      class ActiveReferenceException : public ReferenceStateException
      {
      public:
         ActiveReferenceException(const VoidReference &reference);
      };

      class ShallowReferenceException : public VoidReference::Exception
      {
      public:
         ShallowReferenceException(const VoidReference &reference);
      };

      class SizeMismatchException : public VoidReference::Exception
      {
      public:
         const SIZE_T size;
         
         SizeMismatchException(const VoidReference &reference, SIZE_T size);
      };
      
   protected:
      LPVOID *data;
      SIZE_T *size;
      SIZE_T *referrals;

   private:
      bool shallow;

   public:
      VoidReference(void);
      VoidReference(VoidReference &reference);
      VoidReference(const VoidReference &reference);
      ~VoidReference(void);

      static LPVOID Allocate(SIZE_T size);
      static void Deallocate(LPVOID data);

      void operator=(VoidReference &reference);
      void operator=(const VoidReference &reference);

      void assign(const LPVOID pointer, SIZE_T size);
      void reassign(const LPVOID pointer, SIZE_T size);
      LPVOID deref(void);
      const LPVOID deref(void) const;

      SIZE_T getSize(void) const;
      
      bool hasData(void) const;
      bool isShallow(void) const;
      
      void throwIfShallow(void) const;
      void throwIfHasData(void) const;
      void throwIfNoData(void) const;

      virtual Data read(void) const;
      virtual Data read(SIZE_T size) const;
      virtual Data read(SIZE_T offset, SIZE_T size) const;
      virtual void write(const Data &data);
      virtual void write(SIZE_T offset, const Data &data);
      
      virtual void allocate(SIZE_T size);
      virtual void deallocate(void);
      virtual void resize(SIZE_T size);
   };

   template <class Type, SIZE_T PointerHint=0>
   class Reference : public VoidReference
   {
   public:
      class Exception : public VoidReference::Exception
      {
      public:
         const Reference<Type, PointerHint> &reference;

         Exception(const Reference<Type, PointerHint> &reference, const LPWSTR message)
            : VoidReference::Exception(reference, message)
         {
         }
      };

      class BadPointerHintException : public Exception
      {
      public:
         BadPointerHintException(const Reference<Type, PointerHint> &reference)
            : Exception(reference, EXCSTR(L"Size hint only relevant for pointer-based constructors."))
         {
         }
      };

      Reference(void)
         : VoidReference()
      {
         if (PointerHint == 0)
            this->allocate(sizeof(Type));
         else
            this->allocate(PointerHint);
      }
      
      Reference(const Type &value)
         : VoidReference()
      {
         if (PointerHint != 0)
            throw BadPointerHintException(*this);

         this->assign(value);
      }

      Reference(const Type *type)
         : VoidReference()
      {
         if (PointerHint == 0)
            this->assign(type, sizeof(Type));
         else
            this->assign(type, PointerHint);
      }

      virtual void operator=(const Type &value)
      {
         this->assign(value);
      }

      virtual void operator=(const Type *type)
      {
         this->assign(type);
      }

      virtual Type operator*(void)
      {
         return this->resolve();
      }

      virtual const Type operator*(void) const
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
}
