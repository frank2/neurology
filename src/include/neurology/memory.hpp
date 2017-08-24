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
      BadReferenceStateException(const Memory::Reference &reference);
   };

   class MemoryException : public NeurologyException
   {
   public:
      Memory memory;

      MemoryException(Memory &memory, const LPWSTR message);
      MemoryException(MemoryException &exception);
   };

   class BadModeException : public MemoryException
   {
   public:
      Memory::Mode mode;

      BadModeException(Memory &memory, Memory::Mode mode);
      BadModeException(BadModeException &exception);
   };

   class BadPointerException : public MemoryException
   {
   public:
      LPVOID pointer;
      SIZE_T size;
      Memory::Mode op;

      BadPointerException(Memory &memory, LPVOID pointer, SIZE_T size, Memory::Mode operation);
      BadPointerException(BadPointerException &exception);
   };

   class PointerOutOfBoundsException : public MemoryException
   {
   public:
      LPVOID pointer;
      SIZE_T size;

      PointerOutOfBoundsException(Memory &memory, LPVOID pointer, SIZE_T size);
      PointerOutOfBoundsException(PointerOutOfBoundsException &exception);
   };

   class OffsetOutOfBoundsException : public MemoryException
   {
   public:
      SIZE_T offset, size;
      
      OffsetOutOfBoundsException(Memory &memory, SIZE_T offset, SIZE_T size);
      OffsetOutOfBoundsException(OffsetOutOfBoundsException &exception);
   };

   class PointerWithParentException : public MemoryException
   {
   public:
      PointerWithParentException(Memory &memory);
      PointerWithParentException(PointerWithParentException &exception);
   };

   class OffsetWithoutParentException : public MemoryException
   {
   public:
      OffsetWithoutParentException(Memory &memory);
      OffsetWithoutParentException(OffsetWithoutParentException &exception);
   };

   class NegativeOffsetException : public MemoryException
   {
   public:
      __int64 offset;

      NegativeOffsetException(Memory &memory, __int64 offset);
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
            BYTE modeFlags;
         };

      public:
         Mode(void);
         Mode(bool read, bool write, bool execute);
         Mode(BYTE flags);
         Mode(Mode &mode);
         Mode(const Mode &mode);

         operator int(void) const;

         void operator=(const Mode &mode);
         void operator=(BYTE flags);
         
         Mode operator&(BYTE mask) const;
         Mode operator&(const Mode mask) const;
         Mode operator|(BYTE mask) const;
         Mode operator|(const Mode mask) const;
         Mode operator^(BYTE mask) const;
         Mode operator^(const Mode mask) const;
         
         void operator&=(BYTE mask);
         void operator&=(const Mode mask);
         void operator|=(BYTE mask);
         void operator|=(const Mode mask);
         void operator^=(BYTE mask);
         void operator^=(const Mode mask);

         static BYTE MakeFlags(bool read, bool write, bool execute);
         BYTE flags(void) const;
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

         virtual void operator=(Reference &reference);
         virtual void operator=(const Reference &reference);

         virtual bool isNull(void) const;
         bool isNullAddress(void) const;
         bool state(void) const;
         void setState(bool state);
         LPVOID pointer(void) const;
         void setPointer(LPVOID pointer);
         SIZE_T offset(void) const;
         void setOffset(SIZE_T offset);
         SIZE_T size(void) const;
         void setSize(SIZE_T size);
         Mode mode(void) const;
         void setMode(Mode mode);

      protected:
         virtual void allocate(void);
         virtual void release(void);
      };

   protected:
      Memory *parent;
      Memory::Reference reference;

   public:
      Memory(void);
      Memory(LPVOID pointer, SIZE_T size, Mode mode);
      Memory(Memory *parent, SIZE_T offset, SIZE_T size, Mode mode);
      Memory(Memory &memory);
      Memory(const Memory &memory);

      void operator=(Memory &memory);
      void operator=(const Memory &memory);

      Memory *getParent(void);
      const Memory *getParent(void) const;
      
      /* reference data */
      void ref(void);
      void deref(void);
      DWORD refs(void) const;
      bool isNull(void) const;
      bool isNullAddress(void) const;
      bool isValid(void) const;
      virtual void invalidate(void);

      /* mode data */
      Mode mode(void) const;
      bool readable(void) const;
      bool writable(void) const;
      bool executable(void) const;
      void markReadable(void);
      void markWritable(void);
      void markExecutable(void);
      void unmarkReadable(void);
      void unmarkWritable(void);
      void unmarkExecutable(void);

      SIZE_T size(void) const;
      Address address(void);
      const Address address(void) const;
      Address address(LPVOID pointer);
      const Address address(LPVOID pointer) const;
      virtual Address address(SIZE_T offset);
      virtual const Address address(SIZE_T offset) const;
      LPVOID pointer(void) const;
      LPVOID pointer(SIZE_T offset) const;
      SIZE_T offset(void) const;
      SIZE_T offset(LPVOID address) const;
      LPVOID start(void) const;
      LPVOID end(void) const;
      bool inRange(SIZE_T offset, SIZE_T size) const;
      bool inRange(SIZE_T offset) const;
      bool inRange(LPVOID address, SIZE_T size) const;
      virtual bool inRange(LPVOID address) const;
      virtual void setPointer(LPVOID base);
      virtual void setOffset(SIZE_T offset);
      virtual void setSize(SIZE_T size);
      virtual void setMode(Mode mode);
      Data read(void) const;
      Data read(SIZE_T size) const;
      Data read(SIZE_T offset, SIZE_T size) const;
      virtual Data read(LPVOID address, SIZE_T size) const;
      void write(Memory region);
      void write(Data data);
      void write(SIZE_T offset, const Memory region);
      void write(SIZE_T offset, Data data);
      void write(LPVOID address, const Memory region);
      virtual void write(LPVOID address, Data data);

      Memory slice(SIZE_T start, SIZE_T end);
      const Memory slice(SIZE_T start, SIZE_T end) const;
      Memory slice(LPVOID start, LPVOID end);
      const Memory slice(LPVOID start, LPVOID end) const;
   };

   class Address
   {
   protected:
      union
      {
         Memory memory;
         const Memory constMemory;
      };
      
      SIZE_T offset;

   public:
      Address(void);
      Address(Memory memory);
      Address(const Memory memory);
      Address(Memory memory, SIZE_T offset);
      Address(const Memory memory, SIZE_T offset);
      Address(Address &address);
      Address(const Address &address);
      ~Address(void);

      void operator=(Address &address);
      void operator=(const Address &address);

      bool operator==(const Address &address) const;
      bool operator==(LPVOID pointer) const;
      
      Address operator+(__int64 offset) const;
      Address operator-(__int64 offset) const;
      void operator+=(__int64 offset);
      void operator-=(__int64 offset);

      bool isNull(void) const;
      LPVOID pointer(void);
      LPVOID pointer(SIZE_T offset);
      SIZE_T getOffset(void) const;
      Data read(SIZE_T size) const;
      Data read(SIZE_T offset, SIZE_T size) const;
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

      Object(Address *address, Type type)
      {
         this->address = address;
         this->size = sizeof(Type);
         this->cache = Data(this->size);
         CopyMemory(this->cache.data(), &type, this->size);
      }

      Object(Address *address, Type *type)
      {
         this->address = address;
         this->size = sizeof(Type);
         this->cache = Data(this->size);
         CopyMemory(this->cache.data(), type, this->size);
      }

      Object(Address *address, Type *type, SIZE_T size)
      {
         this->address = address;
         this->size = size;
         this->cache = Data(this->size);
         CopyMemory(this->cache.data(), type, this->size);
      }
      
      Object(Object &object)
      {
         *this = object;
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

      virtual void save(void)
      {
         if (this->address == NULL)
            throw NullPointerException();
         
         this->address->write(this->cache);
      }

      virtual Type *resolve(void)
      {
         if (this->address == NULL)
            throw NullPointerException();
         
         this->cache = this->address->read(this->size);

         if (this->cache.size() != this->size)
            this->cache.resize(this->size);
         
         return (Type *)this->cache.data();
      }

      virtual const Type *resolve(void) const
      {
         if (this->address == NULL)
            throw NullPointerException();
         
         return (const Type *)this->cache.data();
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

      virtual void assign(Type *object, SIZE_T size)
      {
         if (this->address == NULL)
            throw NUllPointerException();

         this->size = size;
         this->address->write(BlockData(object, this->size));
      }

      Type *operator*(void)
      {
         return this->resolve();
      }

      const Type *operator*(void) const
      {
         return this->resolve();
      }

      virtual void operator=(Object &object)
      {
         this->address = object.address;
         this->size = object.size;
         this->cache = object.cache;
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
