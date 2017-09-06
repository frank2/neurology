#pragma once

#include <windows.h>

#include <cstdint>
#include <map>
#include <set>
#include <vector>

#include <neurology/exception.hpp>

/* FIXME this doesn't belong here, but I feel like it will be useful later and
   just don't know where to put it right now. */
struct Register
{
#ifdef REG_DWORD == REG_DWORD_LITTLE_ENDIAN
   union
   {
      struct
      {
         std::uint64_t value;
      } qword;
      struct
      {
         union
         {
            std::uint32_t lowDword;
            std::uint32_t value;
         };
         std::uint32_t highDword;
      } dword;
      struct
      {
         union
         {
            std::uint16_t lowWord;
            std::uint16_t value;
         };
         std::uint16_t highWord, middleWord, upperWord;
      } word;
      struct
      {
         union
         {
            struct
            {
               std::uint8_t value;
               std::uint8_t __padding[7];
            };
            std::uint8_t bytes[8];
         };
      } byte;
   };
#else
   union
   {
      struct
      {
         std::uint64_t value;
      } qword;
      struct
      {
         std::uint32_t highDword;
         union
         {
            std::uint32_t lowDword;
            std::uint32_t value;
         };
      } dword;
      struct
      {
         std::uint16_t upperWord, middleWord, highWord;
         union
         {
            std::uint16_t lowWord;
            std::uint16_t value;
         };
      } word;
      struct
      {
         union
         {
            std::uint8_t bytes[8];
            struct
            {
               std::uint8_t __padding[7];
               std::uint8_t value;
            };
         };
      } byte;
   };
#endif

   Register(void) { this->qword.value = 0; }
   Register(std::uint64_t value) { this->qword.value = value; }
   Register(std::uint32_t value) { this->dword.value = value; }
   Register(std::uint16_t value) { this->word.value = value; }
   Register(std::uint8_t value) { this->byte.value = value; }
   operator std::uint64_t(void) const { return this->qword.value; }
   operator std::uint32_t(void) const { return this->dword.value; }
   operator std::uint16_t(void) const { return this->word.value; }
   operator std::uint8_t(void) const { return this->byte.value; }
   void operator=(std::uint64_t value) { this->qword.value = value; }
   void operator=(std::uint32_t value) { this->dword.value = value; }
   void operator=(std::uint16_t value) { this->word.value = value; }
   void operator=(std::uint8_t value) { this->byte.value = value; }
};

namespace Neurology
{
   typedef std::uintptr_t Label;
   typedef Label *Identifier;

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

      class BadRangeException : public Exception
      {
      public:
         const Label minLabel, maxLabel;

         BadRangeException(AddressPool &pool, const Label minLabel, const Label maxLabel);
      };

      class LabelNotInRangeException : public Exception
      {
      public:
         const Label label;

         LabelNotInRangeException(AddressPool &pool, const Label label);
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
      Label minLabel, maxLabel;
      std::set<Identifier> identities;
      LabelMap labels;
      BindingMap bindings;
      AssociationMap associations;

   public:
      AddressPool(void);
      AddressPool(Label minLabel, Label maxLabel);
      AddressPool(AddressPool &pool);
      ~AddressPool(void);

      void drain(AddressPool &targetPool);
      std::set<Address> pool(void);

      bool hasLabel(const LPVOID pointer) const;
      bool hasLabel(Label label) const;
      bool isAssociated(const Address *address) const;
      bool isBound(const Address *address) const;
      bool inRange(Label label) const;

      void throwIfNoLabel(const LPVOID pointer) const;
      void throwIfNoLabel(Label label) const;
      void throwIfNotAssociated(const Address *address) const;
      void throwIfNotBound(const Address *address) const;
      void throwIfNotInRange(Label label) const;

      Label minimum(void) const;
      Label maximum(void) const;
      std::pair<Label, Label> range(void) const;
      void setMin(Label label);
      void setMax(Label label);
      void setRange(std::pair<Label, Label> range);
      SIZE_T size(void) const;
      
      Address address(const LPVOID pointer);
      Address address(Label label);
      Address newAddress(const LPVOID pointer);
      Address newAddress(Label label);

      Label getLabel(const Address *address) const;

      void move(const Address *address, const LPVOID pointer);
      void move(const Address *address, Label label);
      void move(Label priorLabel, Label newLabel);
      void shift(std::intptr_t shift);
      void rebase(Label newBase);

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
      void unbindOutOfBounds(void);
      
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

      class AddressUnderflowException : public Exception
      {
      public:
         const std::intptr_t shift;

         AddressUnderflowException(Address &address, const std::intptr_t shift);
      };

      class AddressOverflowException : public Exception
      {
      public:
         const std::intptr_t shift;

         AddressOverflowException(Address &address, const std::intptr_t shift);
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

      operator Label(void) const;

      void operator=(const Address &address);

      bool operator<(const Address &address);
      bool operator>(const Address &address);
      bool operator==(const Address &address);
      bool operator!=(const Address &address);
      bool operator<=(const Address &address);
      bool operator>=(const Address &address);

      Address operator+(std::intptr_t shift) const;
      Address operator-(std::intptr_t shift) const;
      std::intptr_t operator-(const Address &address) const;
      Address &operator+=(std::intptr_t shift);
      Address &operator-=(std::intptr_t shift);

      bool hasPool(void) const;
      bool isNull(void) const;

      void throwIfNoPool(void) const;

      AddressPool *getPool(void);
      const AddressPool *getPool(void) const;
      void setPool(AddressPool *pool);
      
      Label label(void) const;
      void move(const LPVOID pointer);
      void move(Label newLabel);
      void moveIdentifier(const LPVOID pointer);
      void moveIdentifier(Label label);

      Address copy(void) const;

   protected:
      const Identifier getAssociation(void) const;
   };
}
