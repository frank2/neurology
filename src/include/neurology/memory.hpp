#pragma once

#include <windows.h>

#include <vector>

#include <neurology/exception.hpp>
#include <neurology/reference.hpp>

namespace Neurology
{
   typedef std::vector<BYTE> Data;
#define VarData(var) Data((LPBYTE)(&(var)), (LPBYTE)((&(var))+1))
#define PointerData(ptr) Data((LPBYTE)(ptr), (LPBYTE)((ptr)+1))
#define BlockData(ptr, size) Data((LPBYTE)(ptr), ((LPBYTE)(ptr))+size)

   class BadReferenceStateException : public ReferenceException
   {
   public:
      BadReferenceStateException(void);
   };

   class MemoryException : public NeurologyException
   {
   public:
      Memory *memory;

      MemoryException(Memory *memory, const LPWSTR message);
      MemoryException(MemoryException &exception);
      ~MemoryException(void);
   };

   class BadModeException : public MemoryException
   {
   public:
      Memory::Mode mode;

      BadModeException(Memory *memory, Memory::Mode mode);
      BadModeException(BadModeException &exception);
   };

   class BadPointerException : public MemoryException
   {
   public:
      LPVOID pointer;
      SIZE_T size;
      Memory::Mode op;

      BadPointerException(Memory *memory, LPVOID pointer, SIZE_T size, Memory::Mode operation);
      BadPointerException(BadPointerException &exception);
   };

   class PointerOutOfBoundsException : public MemoryException
   {
   public:
      LPVOID pointer;
      SIZE_T size;

      PointerOutOfBoundsException(Memory *memory, LPVOID pointer, SIZE_T size);
      PointerOutOfBoundsException(PointerOutOfBoundsException &exception);
   };

   class OffsetOutOfBoundsException : public MemoryException
   {
   public:
      SIZE_T offset, size;
      
      OffsetOutOfBoundsException(Memory *memory, SIZE_T offset, SIZE_T size);
      OffsetOutOfBoundsException(OffsetOutOfBoundsException &exception);
   };

   class PointerWithParentException : public MemoryException
   {
   public:
      PointerWithParentException(Memory *memory);
      PointerWithParentException(PointerWithParentException &exception);
   };

   class OffsetWithoutParentException : public MemoryException
   {
   public:
      OffsetWithoutParentException(Memory *memory);
      OffsetWithoutParentException(OffsetWithoutParentException &exception);
   };

   class NegativeOffsetException : public MemoryException
   {
   public:
      __int64 offset;

      NegativeOffsetException(Memory *memory, __int64 offset);
      NegativeOffsetException(NegativeOffsetException &exception);
   };
   
   class Memory
   {
   public:
      class Mode
      {
      public:
         const static BYTE READ = 0x4;
         const static BYTE WRITE = 0x2;
         const static BYTE EXECUTE = 0x1;

         union
         {
            struct
            {
               BYTE padding:5, read:1, write:1, execute:1;
            };
            BYTE flags;
         };

      public:
         Mode(void);
         Mode(bool read, bool write, bool execute);
         Mode(BYTE flags);
         Mode(Mode &mode);

         void operator=(BYTE flags);
         
         Mode operator&(BYTE mask);
         Mode operator&(Mode mask);
         Mode operator|(BYTE mask);
         Mode operator|(Mode mask);
         Mode operator^(BYTE mask);
         Mode operator^(Mode mask);
         
         void operator&=(BYTE mask);
         void operator&=(Mode mask);
         void operator|=(BYTE mask);
         void operator|=(Mode mask);
         void operator^=(BYTE mask);
         void operator^=(Mode mask);

         static BYTE Flags(bool read, bool write, bool execute);
         BYTE getFlags(void);
         void setFlags(BYTE flags);

         void markReadable(void);
         void markWritable(void);
         void markExecutable(void);

         void unmarkReadable(void);
         void unmarkWritable(void);
         void unmarkExecutable(void);
      };

      class Reference : public Neurology::Reference
      {
      public:
         const static bool STATE_POINTER = false;
         const static bool STATE_OFFSET = true;
         
      protected:
         bool state;
         
         union
         {
            LPVOID *pointerRef;
            SIZE_T *offsetRef;
         };

         SIZE_T *sizeRef;
         Mode *modeRef;

      public:
         Reference(void);
         Reference(LPVOID pointer, SIZE_T size, Mode mode);
         Reference(SIZE_T offset, SIZE_T size, Mode mode);
         Reference(Reference &reference);

         bool isNull(void);
         bool state(void);
         void setState(bool state);
         LPVOID pointer(void);
         void setPointer(LPVOID pointer);
         SIZE_T offset(void);
         void setOffset(SIZE_T offset);
         SIZE_T size(void);
         void setSize(SIZE_T size);
         Mode mode(void);
         void setMode(Mode mode);

      protected:
         void allocate(void);
         void release(void);
      };

   protected:
      Memory *parent;
      Memory::Reference reference;

   public:
      Memory(void);
      Memory(LPVOID pointer, SIZE_T size, Mode mode);
      Memory(Memory *parent, SIZE_T offset, SIZE_T size, Mode mode);
      Memory(Memory &memory);

      Memory *getParent(void);
      
      /* reference data */
      void ref(void);
      void deref(void);
      DWORD refs(void);
      bool isNull(void);
      bool isValid(void);
      virtual void invalidate(void);

      /* mode data */
      Mode mode(void);
      bool readable(void);
      bool writable(void);
      bool executable(void);
      void markReadable(void);
      void markWritable(void);
      void markExecutable(void);
      void unmarkReadable(void);
      void unmarkWritable(void);
      void unmarkExecutable(void);

      SIZE_T size(void);
      Mode mode(void);
      virtual Address address(void);
      virtual Address address(SIZE_T offset);
      LPVOID pointer(void);
      LPVOID pointer(SIZE_T offset);
      SIZE_T offset(void);
      SIZE_T offset(LPVOID address);
      LPVOID start(void);
      LPVOID end(void);
      bool inRange(SIZE_T offset, SIZE_T size);
      bool inRange(SIZE_T offset);
      bool inRange(LPVOID address, SIZE_T size);
      virtual bool inRange(LPVOID address);
      virtual void setPointer(LPVOID base);
      virtual void setOffset(SIZE_T offset);
      virtual void setSize(SIZE_T size);
      virtual void setMode(Mode mode);
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

      Address operator+(__int64 offset);
      Address operator-(__int64 offset);
      void operator+=(__int64 offset);
      void operator-=(__int64 offset);

      LPVOID pointer(void);
      LPVOID pointer(SIZE_T offset);
      SIZE_T getOffset(void);
      Data read(SIZE_T size);
      Data read(SIZE_T offset, SIZE_T size);
      void write(Data data);
      void write(SIZE_T offset, Data data);
      void shift(__int64 offset);
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
