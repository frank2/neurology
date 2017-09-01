#pragma once

#include <windows.h>

#include <cstdint>
#include <map>
#include <set>
#include <vector>

#include <neurology/exception.hpp>

#define VarData(var) Neurology::Data(static_cast<LPBYTE>(&(var)), static_cast<LPBYTE>((&(var))+1))
#define PointerData(ptr) Neurology::Data(static_cast<LPBYTE>(ptr), static_cast<LPBYTE>((ptr)+1))
#define BlockData(ptr, size) Neurology::Data(static_cast<LPBYTE>(ptr), static_cast<LPBYTE>(ptr)+size)

namespace Neurology
{
   typedef std::vector<BYTE> Data;
   typedef std::uintptr_t Label;
   typedef Label *Pointer;

   NTSTATUS CopyData(LPVOID destination, const LPVOID source, SIZE_T size);

   class Address;

   /* my apologies for a completely pointer-hostile address object system. 
      the code is designed the way it is for a specific reason.

      imagine having multiple objects all referring to the same piece of memory
      in some way or another. now imagine that someone reallocates that memory
      and its address changes. how do you change all the underlying objects to
      point to the new region?

      this pointer system is my answer to that. believe me, I like pointers, but
      since we're stuck in the object-oriented hellscape of Stoutrup's nightmares
      we might as well try to try to build some guardrails around the inherent
      unsafety of pointers. I love pointers and I love that freedom, but hey,
      if we have the tools to make it kind of safe, why not use them?
   */
   class AddressPool
   {
      friend Address::Address(void);
      friend Address::Address(LPVOID pointer);
      friend Address::Address(Label label);
      friend Address::~Address(void);
      friend const Pointer Address::getAssociation(void) const;
      friend void Address::movePointer(Label newLabel);
      
   public:
      static AddressPool Instance;

      /* a label can have multiple pointers */
      typedef std::map<Label, std::set<Pointer> > LabelMap;

      /* a pointer can have multiple address objects */
      typedef std::map<Pointer, std::set<Address *> > PointerMap;

      /* pointers aren't directly accessible by address objects. the association map assists this. */
      typedef std::map<Address *, Pointer> > AssocMap;
      
   protected:
      std::set<Pointer> pointers;
      std::set<Address *> addresses;
      LabelMap labels;
      PointerMap bindings;
      AssocMap associations;

   public:
      AddressPool(void);
      ~AddressPool(void);

      bool hasLabel(const LPVOID pointer) const;
      bool hasLabel(Label label) const;
      bool isAssociated(const Address &address) const;
      bool isBound(const Address &address) const;

      Address &address(const LPVOID pointer);
      Address &address(Label label);

      Address &newAddress(const LPVOID pointer);
      Address &newAddress(Label label);

      void move(const Address &address, const LPVOID pointer);
      void move(const Address &address, Label label);

   protected:
      bool hasPointer(const Pointer pointer) const;
      
      Pointer getPointer(const LPVOID pointer) const;
      Pointer getPointer(Label label) const;
      Pointer newPointer(const LPVOID pointer);
      Pointer newPointer(Label label);
      void releasePointer(Pointer pointer);

      Address &newAddress(Pointer pointer);
      Pointer getAssociation(const Address *address) const;

      void bind(const Address *address, const Pointer pointer);
      void rebind(const Address *address, const Pointer newPointer);
      void unbind(Address *address);

      void label(Pointer pointer, Label label);
      void relabel(Pointer pointer, Label newLabel);
      void unlabel(Pointer pointer);

      void move(Pointer pointer, Label newLabel);
      void move(Label priorLabel, Label newLabel);
   };

   class Address
   {
      friend AddressPool;

   protected:
      AddressPool *pool;

      /* constructors for AddressPool objects to use */
      Address(AddressPool *pool);

   public:
      Address(void);
      Address(const LPVOID pointer);
      Address(Label label);
      Address(const Address &address);
      ~Address(void);

      void operator=(const Address &address);
      void operator=(const LPVOID pointer);
      void operator=(Label label);
      LPVOID operator*(void);
      const LPVOID operator*(void) const;

      Address &operator+(std::intptr_t shift);
      const Address &operator+(std::intptr_t shift) const;
      std::intptr_t operator+(const Address &address) const;
      Address &operator-(std::intptr_t shift);
      const Address &operator-(std::intptr_t shift) const;
      std::intptr_t operator-(const Address &address) const;
      Address &operator+=(std::intptr_t shift);
      Address &operator-=(std::intptr_t shift);

      Label label(void) const;
      LPVOID address(void);
      const LPVOID address(void) const;
      
      template <class Type> Type *cast(void)
      {
         return static_cast<Type *>(this->address());
      }
      
      template <class Type> const Type *cast(void) const
      {
         return const_cast<const Type *>(
            static_cast<Type *>(
               const_cast<void *>(this->address())));
      }
      
      void move(const LPVOID pointer);
      void move(Label newLabel);
      void movePointer(const LPVOID pointer);
      void movePointer(Label label);

   protected:
      const Pointer getAssociation(void) const;
   };
}
