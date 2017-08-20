#pragma once

#include <windows.h>

#include <vector>

#include <neurology/exception.hpp>

namespace Neurology
{
   typedef std::vector<BYTE> Data;
#define VarData(var) Data((LPBYTE)(&(var)), (LPBYTE)((&(var))+1))
#define PointerData(ptr) Data((LPBYTE)(ptr), (LPBYTE)((ptr)+1))
#define BlockData(ptr, size) Data((LPBYTE)(ptr), ((LPBYTE)(ptr))+size)

   class NullPointerException : public NeurologyException
   {
   public:
      NullPointerException(void);
      NullPointerException(NullPointerException &exception);
   };

   class BadPointerException : public NeurologyException
   {
   public:
      LPVOID address;
      SIZE_T size;

      BadPointerException(LPVOID address, SIZE_T size);
      BadPointerException(BadPointerException &exception);
   };

   class AddressOutOfBoundsException : public NeurologyException
   {
   public:
      Memory *memory;
      LPVOID address;
      SIZE_T size;

      AddressOutOfBoundsException(Memory *memory, LPVOID address, SIZE_T size);
      AddressOutOfBoundsException(AddressOutOfBoundsException &exception);
   };

   class OffsetOutOfBoundsException : public NeurologyException
   {
   public:
      Memory *memory;
      SIZE_T offset, size;
      
      OffsetOutOfBoundsException(Memory *memory, SIZE_T offset, SIZE_T size);
      OffsetOutOfBoundsException(OffsetOutOfBoundsException &exception);
   };
   
   class Memory
   {
   protected:
      Memory *parent;
      
      union
      {
         LPVOID *pointerRef;
         SIZE_T *offsetRef;
      };
      
      SIZE_T *sizeRef;
      LPDWORD refCount;

   public:
      Memory(void);
      Memory(LPVOID pointer, SIZE_T size);
      Memory(Memory *parent, SIZE_T offset, SIZE_T size);
      Memory(Memory &memory);
      ~Memory(void);

      void ref(void);
      void deref(void);
      DWORD refs(void);
      bool isNull(void);
      bool isValid(void);
      void invalidate(void);

      SIZE_T size(void);
      Address address(void);
      Address address(SIZE_T offset);
      LPVOID pointer(void);
      LPVOID pointer(SIZE_T offset);
      SIZE_T offset(LPVOID address);
      LPVOID start(void);
      LPVOID end(void);
      bool inRange(LPVOID address);
      bool inRange(LPVOID address, SIZE_T size);
      virtual void setPointer(LPVOID base);
      virtual void setOffset(SIZE_T offset);
      virtual void setSize(SIZE_T size);
      Data read(void);
      Data read(SIZE_T size);
      Data read(SIZE_T offset, SIZE_T size);
      virtual Data read(LPVOID address, SIZE_T size);
      void write(Memory *region);
      void write(Data data);
      void write(SIZE_T offset, Memory *region);
      void write(SIZE_T offset, Data data);
      void write(LPVOID address, Memory *region);
      virtual void write(LPVOID address, Data data);
   };

   class Address
   {
   protected:
      Memory *memory;
      SIZE_T offset;

   public:
      Address(void);
      Address(Memory *memory);
      Address(Memory *memory, SIZE_T offset);
      Address(Address &address);
      ~Address(void);

      LPVOID pointer(void);
      LPVOID pointer(SIZE_T offset);
      Data read(SIZE_T size);
      Data read(SIZE_T offset, SIZE_T size);
      void write(Data data);
      void write(SIZE_T offset, Data data);
   };

   template <class Type>
   class Object
   {
   protected:
      Address *address;
      SIZE_T size;
      Data cache;

   public:
      Object(void)
      {
         this->address = NULL;
         this->size = 0;
      }
      
      Object(Address *address)
      {
         this->address = address;
         this->size = sizeof(Type);
         this->cache = Data(this->size);
         ZeroMemory(this->cache.data(), this->size);
      }

      Object(Address *address, SIZE_T size)
      {
         this->address = address;
         this->size = size;
         this->cache = Data(this->size);
         ZeroMemory(this->cache.data(), this->size);
      }
      
      Object(Object &pointer)
      {
         this->address = pointer.address;
         this->size = pointer.size;
         this->cache = pointer.cache;
      }

      ~Object(void)
      {
         try
         {
            this->save();
         }
         except (NeurologyException &exception)
         {
         }

         this->address = NULL;
         this->size = 0;
         this->data = Data();
      }

      void save(void)
      {
         if (this->address == NULL)
            throw NullPointerException();
         
         this->address->write(this->cache);
      }

      virtual Type *resolve(void) const
      {
         if (this->address == NULL)
            throw NullPointerException();
         
         this->cache = this->address->read(this->size);

         if (this->cache.size() != this->size)
            this->cache.resize(this->size);
         
         return (Type *)this->cache.data();
      }

      virtual void assign(Type object)
      {
         if (this->address == NULL)
            throw NullPointerException();
         
         this->address->write(VarData(object));
      }

      virtual void assign(Type *object)
      {
         if (this->address == NULL)
            throw NUllPointerException();
         
         this->address->write(BlockData(object, this->size));
      }

      Type *operator*(void) const
      {
         return this->resolve();
      }
      
      virtual void operator=(Type object)
      {
         this->assign(object);
      }

      virtual void operator=(Type *object)
      {
         this->assign(object);
      }
   };
}
