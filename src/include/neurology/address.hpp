#pragma once

#include <windows.h>

#include <cstdint>
#include <map>
#include <set>
#include <vector>

#include <neurology/exception.hpp>

#define BlockData(ptr, size) Neurology::Data(static_cast<LPBYTE>(ptr), static_cast<LPBYTE>(ptr)+(size))
#define PointerData(ptr) BlockData(ptr, sizeof(*ptr))
#define VarData(var) PointerData(&var)

namespace Neurology
{
   typedef std::vector<BYTE> Data;
   typedef std::uintptr_t Label;
   typedef Label *Identifier;

   LONG CopyData(LPVOID destination, const LPVOID source, SIZE_T size);

   /* my apologies for a completely pointer-hostile address object system. 
      the code is designed the way it is for a specific reason. it's intentionally
      less dynamic than std::shared_ptr, but it operates as if the pointers are 
      not only not typed, but merely numeric labels.

      imagine having multiple objects all referring to the same piece of memory
      in some way or another. now imagine that someone reallocates that memory
      and its address changes. how do you change all the underlying objects to
      point to the new region? how can you design it in such a way that makes it
      highly difficult to change the core pointer but still maintain pointer
      functionality? what about pointers to memory that's in another process
      entirely? how do you verify whether or not the pointer you have is valid?

      this address system is my answer to that. believe me, I like pointers, but
      since we're stuck in the object-oriented hellscape of Stoutrup's nightmares
      we might as well try to try to build some guardrails around the inherent
      "unsafety" of pointers. I love pointers and I love that freedom, but hey,
      if we have the tools to make it kind of safe, why not use them?
   */
   class Address;
   
   class AddressPool
   {
      friend Address;

   public:
      class Exception : public Neurology::Exception
      {
      public:
         AddressPool &pool;

         Exception(AddressPool &pool, LPWSTR message);
      };

      class NullIdentifierException : public Exception
      {
      public:
         NullIdentifierException(AddressPool &pool);
      };

      class AddressAlreadyBoundException : public Exception
      {
      public:
         Address &address;
         
         AddressAlreadyBoundException(AddressPool &pool, Address &address);
      };

      class IdentifierAlreadyLabeledException : public Exception
      {
      public:
         const Identifier identity;

         IdentifierAlreadyLabeledException(AddressPool &pool, const Identifier identity);
      };

      class IdentifierNotLabeledException : public Exception
      {
      public:
         const Identifier identity;

         IdentifierNotLabeledException(AddressPool &pool, const Identifier identity);
      };

      class NoSuchLabelException : public Exception
      {
      public:
         Label label;

         NoSuchLabelException(AddressPool &pool, Label label);
      };

      class AddressNotAssociatedException : public Exception
      {
      public:
         Address &address;
         
         AddressNotAssociatedException(AddressPool &pool, Address &address);
      };

      class AddressNotBoundException : public Exception
      {
      public:
         Address &address;
         
         AddressNotBoundException(AddressPool &pool, Address &address);
      };

      class NoSuchIdentifierException : public Exception
      {
      public:
         const Identifier identity;

         NoSuchIdentifierException(AddressPool &pool, const Identifier identity);
      };
      
   protected:
      /* a label can have multiple identities */
      typedef std::map<Label, std::set<Identifier> > LabelMap;

      /* an identity can have multiple address objects */
      typedef std::map<Identifier, std::set<Address *> > BindingMap;

      /* identities aren't directly accessible by address objects. the association map assists this. */
      typedef std::map<Address *, Identifier> AssociationMap;
      
   public:
      static AddressPool Instance;

   protected:
      std::set<Identifier> identities;
      LabelMap labels;
      BindingMap bindings;
      AssociationMap associations;

   public:
      AddressPool(void);
      ~AddressPool(void);

      bool hasLabel(const LPVOID pointer) const;
      bool hasLabel(Label label) const;
      bool isAssociated(const Address *address) const;
      bool isBound(const Address *address) const;

      void throwIfNoLabel(const LPVOID pointer) const;
      void throwIfNoLabel(Label label) const;
      void throwIfNotAssociated(const Address *address) const;
      void throwIfNotBound(const Address *address) const;

      Address address(const LPVOID pointer);
      Address address(Label label);
      Address newAddress(const LPVOID pointer);
      Address newAddress(Label label);

      LPVOID getPointer(const Address *address);
      const LPVOID getPointer(const Address *address) const;
      Label getLabel(const Address *address) const;

      void move(const Address *address, const LPVOID pointer);
      void move(const Address *address, Label label);
      void move(Label priorLabel, Label newLabel);

   protected:
      bool hasIdentifier(const Identifier pointer) const;

      void throwIfNoIdentifier(const Identifier identity) const;
      
      Identifier getIdentifier(const LPVOID pointer) const;
      Identifier getIdentifier(Label label) const;
      Identifier newIdentifier(const LPVOID pointer);
      Identifier newIdentifier(Label label);
      void releaseIdentifier(Identifier pointer);

      Address newAddress(Identifier pointer);
      Identifier getAssociation(const Address *address) const;

      void bind(const Address *address, const Identifier pointer);
      void rebind(const Address *address, const Identifier newIdentifier);
      void unbind(const Address *address);
      
      void identify(Identifier identity, Label label);
      void reidentify(Identifier identity, Label newLabel);
      void unidentify(Identifier identity);
   };

   class Address
   {
      friend AddressPool;

   public:
      class Exception : public Neurology::Exception
      {
      public:
         Address &address;

         Exception(Address &address, const LPWSTR message);
      };

      class NoPoolException : public Exception
      {
      public:
         NoPoolException(Address &address);
      };

      class NegativeMovementException : public Exception
      {
      public:
         std::intptr_t shift;

         NegativeMovementException(Address &address, std::intptr_t shift);
      };
      
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
      LPVOID operator->(void);
      const LPVOID operator->(void) const;

      Address operator+(std::intptr_t shift) const;
      Address operator-(std::intptr_t shift) const;
      std::intptr_t operator-(const Address &address) const;
      Address &operator+=(std::intptr_t shift);
      Address &operator-=(std::intptr_t shift);

      bool hasPool(void) const;

      void throwIfNoPool(void) const;
      
      Label label(void) const;
      LPVOID pointer(void);
      const LPVOID pointer(void) const;
      
      template <class Type> Type *cast(void)
      {
         return static_cast<Type *>(this->pointer());
      }
      
      template <class Type> const Type *cast(void) const
      {
         return const_cast<const Type *>(
            static_cast<Type *>(
               const_cast<LPVOID>(this->pointer())));
      }
      
      void move(const LPVOID pointer);
      void move(Label newLabel);
      void moveIdentifier(const LPVOID pointer);
      void moveIdentifier(Label label);

   protected:
      const Identifier getAssociation(void) const;
   };
}
