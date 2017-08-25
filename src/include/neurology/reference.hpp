#pragma once

#include <windows.h>

#include <vector>

#include <neurology/exception.hpp>

namespace Neurology
{
   class Frame
   {
   public:
      class Exception : public Neurology::Exception
      {
      public:
         const Frame &frame;

         Exception(const Frame &frame, const LPWSTR message);
      };

      class ChargeException : public Frame::Exception
      {
      public:
         ChargeException(const Frame &frame, const LPWSTR message);
      };

      class AlreadyChargedException : public ChargeException
      {
      public:
         AlreadyChargedException(const Frame &frame);
      };

      class NoChargeException : public ChargeException
      {
      public:
         NoChargeException(const Frame &frame);
      };

      class ShallowFrameException : public Frame::Exception
      {
      public:
         ShallowFrameException(const Frame &frame);
      };

      class SizeMismatchException : public Frame::Exception
      {
      public:
         const SIZE_T volts;
         
         SizeMismatchException(const Frame &frame, SIZE_T volts);
      };

      class UnknownSizeException : public Frame::Exception
      {
      public:
         const LPVOID address;

         UnknownSizeException(const Frame &frame, LPVOID address);
      };
      
   protected:
      LPVOID *data;
      SIZE_T size;

   private:
      bool shallow;

   public:
      Frame(void);
      {
         this->data = NULL;
         this->size = 0;
         this->shallow = false;
      }
      
      Frame(Frame &frame);
      {
         *this = frame;
      }
      
      Frame(const Frame &frame)
      {
         *this = frame;
      }

      ~Frame(void);
      {
         if (this->shallow)
            this->discharge();
      }

      static LPVOID Allocate(SIZE_T size);
      {
         return reinterpret_cast<LPVOID>(new BYTE[size]);
      }
      
      static void Deallocate(LPVOID data);
      {
         delete[] data;
      }

      void operator=(Frame &frame);
      {
         this->data = frame.data;
         this->size = frame.size;
         this->shallow = false;
      }

      void operator=(const Frame &frame);
      {
         this->throwIfShallow();
         frame.throwIfNoData();

         if (!this->hasData)
            this->charge(size);
         else if (this->size != frame.size)
            this->adjustSize(frame.size);
         
         this->assign(*frame.data, frame.size);
         this->shallow = true;
      }

      void assign(LPVOID pointer, SIZE_T size);
      {
         if (!this->hasData())
            this->charge(size);
         else if (this->size != size)
            throw SizeMismatchException(*this, size);

         CopyMemory(*this->data, pointer, size);
      }

      void reassign(LPVOID pointer, SIZE_T size);
      {
         this->throwIfShallow();
         
         if (this->hasData() && this->size != size)
            this->adjustSize(size);

         this->assign(pointer, size);
      }

      SIZE_T voltage(void) const;
      {
         return this->size;
      }

      bool hasData(void) const;
      {
         return this->data != NULL && *this->data != NULL && this->size > 0;
      }
      
      bool isShallow(void) const;
      {
         return this->shallow;
      }

      void throwIfShallow(void) const;
      {
         if (this->shallow)
            throw ShallowFrameException(*this);
      }

      void throwIfHasData(void) const;
      {
         if (this->hasData())
            throw ChargedFrameException(*this);
      }

      void throwIfNoData(void) const;
      {
         if (!this->hasData())
            throw DischargedFrameException(*this);
      }
      
      void charge(SIZE_T size);
      {
         this->throwIfHasData();

         this->size = size;

         if (this->data == NULL)
            this->data = new LPVOID;
         
         *this->data = Frame::Allocate(this->size);
      }
      
      void discharge(void);
      {
         this->throwIfNoData();

         Frame::Deallocate(*this->data);
         *this->data = NULL;

         delete this->data;
         this->data = NULL;
      }
      
      void adjustSize(SIZE_T size);
      {
         LPVOID newData;
         
         this->throwIfShallow();
         this->throwIfNoData();

         if (min(size, this->size) != 0)
         {
            newData = Frame::Allocate(size);
            CopyMemory(newData, *this->data, min(size, this->size));
         }
         else
            newData = NULL

         Frame::Deallocate(*this->data);
         this->data = newData;
         this->size = size;
      }
   };

   template <class Type>
   class TypedFrame : public Frame
   {
      TypedFrame(Type value)
         : Frame()
      {
         this->assign(value);
      }

      TypedFrame(Type *type)
         : Frame()
      {
         this->assign(type);
      }

      TypedFrame(Type *type, SIZE_T size)
         : Frame()
      {
         this->assign(type, size);
      }

      void operator=(Type value)
      {
         this->assign(value);
      }

      void operator=(Type *type)
      {
         this->assign(type);
      }

      Type operator*(void)
      {
         return this->resolve();
      }

      void assign(Type value)
      {
         this->assign((LPVOID)&value, sizeof(Type));
      }

      void assign(Type *pointer)
      {
         this->assign((LPVOID)type, sizeof(Type));
      }

      void reassign(Type value)
      {
         this->reassign((LPVOID)&value, sizeof(Type));
      }

      void reassign(Type *pointer)
      {
         this->reassign((LPVOID)pointer, sizeof(Type));
      }

      Type *cast(void) const
      {
         this->throwIfNoData();
         
         return reinterpret_cast<Type *>(*this->data);
      }

      Type resolve(void) const
      {
         return *this->cast();
      }
   };

   class Reference
   {
   public:
      class Exception : public Neurology::Exception
      {
      public:
         const Reference &reference;

         Exception(const Reference &reference, const LPWSTR message);
      };
      
   protected:
      TypedFrame<SIZE_T> referrals;

   private:
      bool shallow;
      
   public:
      Reference(void);
      Reference(Reference &master);
      Reference(const Reference &master);
      ~Reference(void);

      virtual void operator=(Reference &master);
      virtual void operator=(const Reference &master);

      SIZE_T getReferrals(void) const;
      virtual void reference(void);
      virtual void dereference(void);
      bool isDead(void) const;
      bool isWaiting(void) const;
      bool isActive(void) const;
      bool isShallow(void) const;
      void throwIfActive(void) const;
      void throwIfShallow(void) const;

   protected:
      virtual void solder(void);
      virtual void shutdown(void);
   };
}
