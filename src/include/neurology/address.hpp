#pragma once

#include <windows.h>

#include <cstdint>
#include <limits>
#include <map>
#include <set>
#include <vector>

#include <neurology/exception.hpp>

/* FIXME this doesn't belong here, but I feel like it will be useful later and
   just don't know where to put it right now. */
class Register
{
public:
#if REG_DWORD == REG_DWORD_LITTLE_ENDIAN
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
               std::uint8_t padding[7];
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
   typedef Label *Identity;

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

      class NullIdentityException : public Exception
      {
      public:
         NullIdentityException(AddressPool &pool);
      };

      class AddressAlreadyBoundException : public Exception
      {
      public:
         Address &address;
         
         AddressAlreadyBoundException(AddressPool &pool, Address &address);
      };

      class IdentityAlreadyLabeledException : public Exception
      {
      public:
         const Identity identity;

         IdentityAlreadyLabeledException(AddressPool &pool, const Identity identity);
      };

      class IdentityNotLabeledException : public Exception
      {
      public:
         const Identity identity;

         IdentityNotLabeledException(AddressPool &pool, const Identity identity);
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

      class NoSuchIdentityException : public Exception
      {
      public:
         const Identity identity;

         NoSuchIdentityException(AddressPool &pool, const Identity identity);
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
      typedef std::set<Address *> AddressSet;
      typedef std::set<Identity> IdentitySet;
      
      /* one label can have multiple identities */
      typedef std::map<const Label, IdentitySet> LabelMap;

      /* one identity can have multiple addresses */
      typedef std::map<const Identity, AddressSet> BindingMap;
      
   public:
      static AddressPool Instance;

   protected:
      Label minLabel, maxLabel;
      IdentitySet identities;
      LabelMap labels;
      BindingMap bindings;

   public:
      AddressPool(void);
      AddressPool(Label minLabel, Label maxLabel);
      AddressPool(AddressPool &pool);
      ~AddressPool(void);

      void drain(AddressPool &targetPool);
      std::set<Address *> pool(void) const;

      bool hasLabel(const LPVOID pointer) const;
      bool hasLabel(Label label) const;
      bool isBound(const Address &address) const;
      bool inRange(Label label) const;
      bool hasIdentity(const Identity identity) const;
      bool sharesIdentity(const Address &left, const Address &right) const;

      void throwIfNoLabel(const LPVOID pointer) const;
      void throwIfNoLabel(Label label) const;
      void throwIfNotBound(const Address &address) const;
      void throwIfNotInRange(Label label) const;

      Label minimum(void) const;
      Label maximum(void) const;
      std::pair<Label, Label> range(void) const;
      void setMin(Label label);
      void setMax(Label label);
      void setRange(Label minLabel, Label maxLabel);
      void setRange(std::pair<Label, Label> range);
      std::uintptr_t size(void) const;
      
      Address address(const LPVOID pointer);
      Address address(Label label);
      Address newAddress(const LPVOID pointer);
      Address newAddress(Label label);

      void move(Address &address, const LPVOID pointer);
      void move(Address &address, Label label);
      void move(Label priorLabel, Label newLabel);
      void shift(std::intptr_t shift);
      void rebase(Label newBase);

   protected:
      void throwIfNoIdentity(const Identity identity) const;
      
      Identity getIdentity(const LPVOID pointer) const;
      Identity getIdentity(Label label) const;
      Identity newIdentity(const LPVOID pointer);
      Identity newIdentity(Label label);
      void releaseIdentity(Identity identity);

      Address newAddress(Identity identity);

      void bind(Address *address, Identity identity);
      void rebind(Address *address, Identity newIdentity);
      void unbind(Address *address);
      void unbindOutOfBounds(void);
      
      void identify(Identity identity, Label label);
      void reidentify(Identity identity, Label newLabel);
      void unidentify(Identity identity);
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

      class NullIdentityException : public Exception
      {
      public:
         NullIdentityException(Address &address);
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
      Identity identity;

      /* constructors for AddressPool objects to use */
      Address(AddressPool *pool);

   public:
      Address(void);
      Address(const LPVOID pointer);
      Address(Label label);
      Address(unsigned int lowLabel);
      Address(const Address &address);
      ~Address(void);

      operator Label(void) const;

      void operator=(const Address &address);

      bool operator<(const Address &address) const;
      bool operator<(Label label) const;
      bool operator>(const Address &address) const;
      bool operator>(Label label) const;
      bool operator==(const Address &address) const;
      bool operator==(Label label) const;
      bool operator!=(const Address &address) const;
      bool operator!=(Label label) const;
      bool operator<=(const Address &address) const;
      bool operator<=(Label label) const;
      bool operator>=(const Address &address) const;
      bool operator>=(Label label) const;

      Address operator+(std::intptr_t shift) const;
      Address operator+(std::uintptr_t shift) const;
      Address operator+(int shift) const;
      Address operator-(std::intptr_t shift) const;
      Address operator-(std::uintptr_t shift) const;
      Address operator-(int shift) const;
      std::intptr_t operator-(const Address &address) const;
      Address &operator+=(std::intptr_t shift);
      Address &operator+=(std::uintptr_t shift);
      Address &operator+=(int shift);
      Address &operator-=(std::intptr_t shift);
      Address &operator-=(std::uintptr_t shift);
      Address &operator-=(int shift);

      LPVOID pointer(void);
      const LPVOID pointer(void) const;

      bool hasPool(void) const;
      bool isNull(void) const;
      bool usesPool(const AddressPool *pool) const;
      bool inRange(void) const;
      bool sharesIdentity(const Address &address) const;

      void throwIfNull(void) const;
      void throwIfNoPool(void) const;
      void throwIfNotInRange(void) const;

      AddressPool *getPool(void);
      const AddressPool *getPool(void) const;
      void setPool(AddressPool *pool);
      
      Label label(void) const;
      void move(const LPVOID pointer);
      void move(Label newLabel);
      void moveIdentity(const LPVOID pointer);
      void moveIdentity(Label label);

      Address copy(void) const;
   };

   class Offset : public Address
   {
   protected:
      std::uintptr_t offset;

   public:
      Offset(void);
      Offset(const Address &base);
      Offset(const Address &base, std::uintptr_t offset);
      Offset(const Offset &offset);

      void operator=(const Offset &offset);
      void operator=(const Address &address);

      bool operator<(const Offset &offset) const;
      bool operator>(const Offset &offset) const;
      bool operator==(const Offset &offset) const;
      bool operator!=(const Offset &offset) const;
      bool operator<=(const Offset &offset) const;
      bool operator>=(const Offset &offset) const;

      Offset operator+(std::intptr_t shift) const;
      Offset operator+(std::uintptr_t shift) const;
      Offset operator-(std::intptr_t shift) const;
      Offset operator-(std::uintptr_t shift) const;
      std::intptr_t operator-(const Offset &offset) const;
      Offset &operator+=(std::intptr_t shift);
      Offset &operator+=(std::uintptr_t shift);
      Offset &operator-=(std::intptr_t shift);
      Offset &operator-=(std::uintptr_t shift);

      Address operator*(void);
      const Address operator*(void) const;

      Address address(void);
      const Address address(void) const;
      Address &getBaseAddress(void);
      const Address &getBaseAddress(void) const;
      void setAddress(Address &address);
      std::uintptr_t getOffset(void) const;
      void setOffset(std::uintptr_t offset);
      void setOffset(Address &address, std::uintptr_t offset);

      virtual Label label(void) const;

      Offset copy(void) const;
   };
}
