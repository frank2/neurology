#pragma once

#include <windows.h>

#include <cstdint>
#include <map>
#include <set>

#include <neurology/exception.hpp>

namespace Neurology
{
   typedef std::uintptr_t Label;
   typedef Label *Pointer;

   class Address;
   
   class AddressPool
   {
      friend Address::Address(void);
      friend Address::Address(void *pointer);
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

      bool hasLabel(const void *pointer) const;
      bool hasLabel(Label label) const;
      bool isBound(const Address &address) const;

      Address &address(const void *pointer);
      Address &address(Label label);

      Address &newAddress(const void *pointer);
      Address &newAddress(Label label);

      void move(const Address &address, const void *pointer);
      void move(const Address &address, Label label);

   protected:
      bool hasPointer(const Pointer pointer) const;
      
      Pointer getPointer(const void *pointer) const;
      Pointer getPointer(Label label) const;
      Pointer newPointer(const void *pointer);
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
      Address(void *pointer);
      Address(Label label);
      Address(const Address &address);
      ~Address(void);

      void operator=(const Address &address);
      void operator=(const void *pointer);
      void operator=(Label label);

      Label label(void) const;
      void *address(void);
      const void *address(void) const;
      
      template <class Type> Type *cast(void)
      {
         return static_cast<Type *>(this->lpVoid());
      }
      
      template <class Type> const Type *cast(void) const
      {
         return const_cast<const Type *>(
            static_cast<Type *>(
               const_cast<void *>(this->lpVoid())));
      }
      
      void move(const void *pointer);
      void move(Label newLabel);
      void movePointer(const void *pointer);
      void movePointer(Label label);

   protected:
      const Pointer getAssociation(void) const;
   };
}
