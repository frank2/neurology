#include <neurology/address.hpp>

using namespace Neurology;

AddressPool AddressPool::Instance;

AddressPool::Exception::Exception
(AddressPool &pool, LPWSTR message)
   : Neurology::Exception(message)
   , pool(pool)
{
}

AddressPool::NullIdentityException::NullIdentityException
(AddressPool &pool)
   : AddressPool::Exception(pool, EXCSTR(L"Identity is null."))
{
}

AddressPool::AddressAlreadyBoundException::AddressAlreadyBoundException
(AddressPool &pool, Address &address)
   : AddressPool::Exception(pool, EXCSTR(L"The provided address is already bound."))
   , address(address)
{
}

AddressPool::IdentityAlreadyLabeledException::IdentityAlreadyLabeledException
(AddressPool &pool, const Identity identity)
   : AddressPool::Exception(pool, EXCSTR(L"The identifier has already been labeled."))
   , identity(identity)
{
}

AddressPool::IdentityNotLabeledException::IdentityNotLabeledException
(AddressPool &pool, const Identity identity)
   : AddressPool::Exception(pool, EXCSTR(L"The identifier is not labeled."))
   , identity(identity)
{
}

AddressPool::NoSuchLabelException::NoSuchLabelException
(AddressPool &pool, Label label)
   : AddressPool::Exception(pool, EXCSTR(L"No such label being tracked by the given pool."))
   , label(label)
{
}

AddressPool::AddressNotAssociatedException::AddressNotAssociatedException
(AddressPool &pool, Address &address)
   : AddressPool::Exception(pool, EXCSTR(L"The provided address is not associated with the given pool."))
   , address(address)
{
}

AddressPool::AddressNotBoundException::AddressNotBoundException
(AddressPool &pool, Address &address)
   : AddressPool::Exception(pool, EXCSTR(L"The provided address is not bound to an identity."))
   , address(address)
{
}

AddressPool::NoSuchIdentityException::NoSuchIdentityException
(AddressPool &pool, const Identity identity)
   : AddressPool::Exception(pool, EXCSTR(L"The given identifier does not exist within the given address pool."))
   , identity(identity)
{
}

AddressPool::BadRangeException::BadRangeException
(AddressPool &pool, const Label minLabel, const Label maxLabel)
   : AddressPool::Exception(pool, EXCSTR(L"The address pool range is invalid."))
   , minLabel(minLabel)
   , maxLabel(maxLabel)
{
}

AddressPool::LabelNotInRangeException::LabelNotInRangeException
(AddressPool &pool, const Label label)
   : AddressPool::Exception(pool, EXCSTR(L"The provided label is not in range of the pool."))
   , label(label)
{
}

/* Windows is fucking rude basically */
#pragma push_macro("max")
#undef max

AddressPool::AddressPool
(void)
   : minLabel(0)
   , maxLabel(std::numeric_limits<Label>::max())
{
}

#pragma pop_macro("max")

AddressPool::AddressPool
(Label minLabel, Label maxLabel)
   : minLabel(minLabel)
   , maxLabel(maxLabel)
{
   if (this->minLabel > this->maxLabel)
      throw BadRangeException(*this, minLabel, maxLabel);
}

AddressPool::AddressPool
(AddressPool &pool)
{
   pool.drain(*this);
}

AddressPool::~AddressPool
(void)
{
   BindingMap::iterator bindingIter;
   LabelMap::iterator labelIter;
   AddressSet::iterator addressIter;
   IdentitySet::iterator identifierIter;
   
   /* unbind all addresses from their corresponding identifiers */

   /* the caveat with unbinding is that they will modify the corresponding
      map objects when we do so, so take a different approach to iterating
      over the objects. */
   while ((bindingIter = this->bindings.begin()) != this->bindings.end())
   {
      AddressSet::iterator addrIter = bindingIter->second.begin();
      this->unbind(*addrIter);
   }
   
   /* release all remaining identifiers from their corresponding labels */
   while ((labelIter = this->labels.begin()) != this->labels.end())
   {
      IdentitySet::iterator identifierIter = labelIter->second.begin();
      this->releaseIdentity(*identifierIter);
   }

   /* why are there still identities? delete them, this is probably some kind of
      memory leak. */
   while ((identifierIter = this->identities.begin()) != this->identities.end())
      delete *identifierIter;
}

void
AddressPool::drain
(AddressPool &targetPool)
{
   targetPool.identities = this->identities;
   targetPool.labels = this->labels;
   targetPool.bindings = this->bindings;

   for (BindingMap::iterator iter=this->bindings.begin();
        iter!=this->bindings.end();
        ++iter)
   {
      for (AddressSet::iterator setIter=iter->second.begin();
           setIter!=iter->second.end();
           ++setIter)
      {
         (*setIter)->pool = &targetPool;
      }
   }

   /* the copy constructors literally copy, and that's not what we want to do.
      so clear the pool we drained. */
   this->identities.clear();
   this->labels.clear();
   this->bindings.clear();
}

AddressPool::AddressSet
AddressPool::pool
(void) const
{
   BindingMap::const_iterator bindingIter;
   AddressSet result;

   for (bindingIter=this->bindings.begin(); bindingIter!=this->bindings.end(); ++bindingIter)
   {
      AddressSet::iterator addressIter;

      for (addressIter=bindingIter->second.begin(); addressIter!=bindingIter->second.end(); ++addressIter)
         result.insert(*addressIter);
   }

   return result;
}

bool
AddressPool::hasLabel
(const LPVOID pointer) const
{
   return this->hasLabel(reinterpret_cast<Label>(pointer));
}

bool
AddressPool::hasLabel
(Label label) const
{
   return this->labels.count(label) > 0;
}

bool
AddressPool::isBound
(const Address &address) const
{
   AddressPool::BindingMap::const_iterator bindingIter;

   bindingIter = this->bindings.find(address.identity);

   if (bindingIter == this->bindings.end())
      return false;

   return bindingIter->second.find(const_cast<Address *>(&address)) != bindingIter->second.end();
}

bool
AddressPool::inRange
(Label label) const
{
   return label >= this->minLabel && label <= this->maxLabel;
}

bool
AddressPool::hasIdentity
(const Identity identity) const
{
   return this->identities.find(const_cast<Identity>(identity)) != this->identities.end();
}

bool
AddressPool::sharesIdentity
(const Address &first, const Address &second) const
{
   if (!this->isBound(first) || !this->isBound(second))
      return false;

   return first.identity == second.identity;
}

void
AddressPool::throwIfNoLabel
(const LPVOID pointer) const
{
   return this->throwIfNoLabel(reinterpret_cast<Label>(pointer));
}

void
AddressPool::throwIfNoLabel
(Label label) const
{
   if (!this->hasLabel(label))
      throw NoSuchLabelException(*const_cast<AddressPool *>(this), label);
}

void
AddressPool::throwIfNotBound
(const Address &address) const
{
   if (!this->isBound(address))
      throw AddressNotBoundException(*const_cast<AddressPool *>(this), const_cast<Address &>(address));
}

void
AddressPool::throwIfNotInRange
(Label label) const
{
   if (!this->inRange(label))
      throw LabelNotInRangeException(*const_cast<AddressPool *>(this), label);
}

Label
AddressPool::minimum
(void) const
{
   return this->minLabel;
}

Label
AddressPool::maximum
(void) const
{
   return this->maxLabel;
}

std::pair<Label, Label>
AddressPool::range
(void) const
{
   return std::pair<Label, Label>(this->minLabel, this->maxLabel);
}

void
AddressPool::setMin
(Label label)
{
   if (label > this->maxLabel)
      throw BadRangeException(*this, label, this->maxLabel);
      
   this->minLabel = label;
   this->unbindOutOfBounds();
}

void
AddressPool::setMax
(Label label)
{
   if (label < this->minLabel)
      throw BadRangeException(*this, this->minLabel, label);

   this->maxLabel = label;
   this->unbindOutOfBounds();
}

void
AddressPool::setRange
(Label minLabel, Label maxLabel)
{
   this->setRange(std::pair<Label,Label>(minLabel, maxLabel));
}

void
AddressPool::setRange
(std::pair<Label, Label> range)
{
   if (range.first > range.second || range.second < range.first)
      throw BadRangeException(*this, range.first, range.second);

   this->minLabel = range.first;
   this->maxLabel = range.second;
   this->unbindOutOfBounds();
}

std::uintptr_t
AddressPool::size
(void) const
{
   return this->maxLabel - this->minLabel;
}

Address
AddressPool::address
(const LPVOID pointer)
{
   return this->address(reinterpret_cast<Label>(pointer));
}

Address
AddressPool::address
(Label label)
{
   std::set<Identity>::iterator identifierIter;

   if (this->hasLabel(label))
      return this->newAddress(this->getIdentity(label));
   
   return this->newAddress(label);
}

Address
AddressPool::newAddress
(const LPVOID pointer)
{
   return this->newAddress(reinterpret_cast<Label>(pointer));
}

Address
AddressPool::newAddress
(Label label)
{
   return this->newAddress(this->newIdentity(label));
}

void
AddressPool::move
(Address &address, const LPVOID pointer)
{
   return this->move(address, reinterpret_cast<Label>(pointer));
}
   
void
AddressPool::move
(Address &address, Label newLabel)
{
   this->throwIfNotBound(address);

   if (this->hasLabel(newLabel))
      this->rebind(&address, this->getIdentity(newLabel));
   else
      this->rebind(&address, this->newIdentity(newLabel));
}

void
AddressPool::move
(Label priorLabel, Label newLabel)
{
   std::set<Identity>::iterator identIter;
   
   this->throwIfNoLabel(priorLabel);

   /* reidentifying will slowly wipe out this list of labels */
   while ((identIter = this->labels[priorLabel].begin()) != this->labels[priorLabel].end())
   {
      this->reidentify(*identIter, newLabel);
      
      if (this->labels.count(priorLabel) == 0)
         break;
   }
}

void
AddressPool::shift
(std::intptr_t shift)
{
   IdentitySet::iterator identIter;

   if (shift == 0)
      return;

   for (identIter=this->identities.begin(); identIter!=this->identities.end(); ++identIter)
      this->reidentify(*identIter, **identIter+shift);
}

void
AddressPool::rebase
(Label newBase)
{
   std::uintptr_t currentSize = this->size();
   Label newMax = newBase + currentSize;
   std::intptr_t delta = newBase - this->minLabel;

   if (newBase > newMax)
      throw BadRangeException(*this, newBase, newMax);

   if (delta == 0)
      return;
   
   /* create a temporary range that will include the new base as well as the
      old base, allowing everything to be moved freely. */
   this->minLabel = min(newBase, this->minLabel);
   this->maxLabel = max(newMax, this->maxLabel);

   this->shift(delta);

   this->minLabel = newBase;
   this->maxLabel = newMax;
}

void
AddressPool::throwIfNoIdentity
(const Identity identity) const
{
   if (!this->hasIdentity(identity))
      throw NoSuchIdentityException(*const_cast<AddressPool *>(this), identity);
}

Identity
AddressPool::getIdentity
(const LPVOID pointer) const
{
   return this->getIdentity(reinterpret_cast<Label>(pointer));
}

Identity
AddressPool::getIdentity
(Label label) const
{
   this->throwIfNotInRange(label);
   this->throwIfNoLabel(label);

   /* douchebag C++:
      >wants you to enforce const correctness
      >won't properly enforce const correctness on the stl

      buddy why can't you just return a const reference for std::map::operator[]() const
    */
   return *this->labels.at(label).begin();
}

Identity
AddressPool::newIdentity
(const LPVOID pointer)
{
   return this->newIdentity(reinterpret_cast<Label>(pointer));
}

Identity
AddressPool::newIdentity
(Label label)
{
   Identity newIdentity;

   this->throwIfNotInRange(label);

   newIdentity = new Label(0);
   this->identities.insert(newIdentity);
   this->identify(newIdentity, label);

   return newIdentity;
}

void
AddressPool::releaseIdentity
(Identity identity)
{
   BindingMap::iterator bindingIter;
   LabelMap::iterator labelIter;
   
   this->throwIfNoIdentity(identity);

   while ((bindingIter = this->bindings.find(identity)) != this->bindings.end())
   {
      AddressSet::iterator addrIter = bindingIter->second.begin();
      this->unbind(*addrIter);
   }

   /* if the unbinding of all its addresses killed the identity, we're done here. */
   if (!this->hasIdentity(identity))
      return;

   if (this->labels.count(*identity) > 0)
      this->unidentify(identity);

   delete identity;
}

Address
AddressPool::newAddress
(Identity identity)
{
   this->throwIfNoIdentity(identity);

   Address newAddress(this);
   this->bind(&newAddress, identity);
   return newAddress;
}

void
AddressPool::bind
(Address *address, Identity identity)
{
   this->throwIfNoIdentity(identity);

   if (this->bindings.count(identity) > 0 && this->bindings[identity].find(address) != this->bindings[identity].end())
      throw AddressAlreadyBoundException(*this, *address);

   this->bindings[identity].insert(address);
   address->identity = identity;
}

void
AddressPool::rebind
(Address *address, Identity identity)
{
   Identity assoc;

   this->throwIfNoIdentity(identity);

   assoc = address->identity;

   if (this->bindings.count(assoc) > 0 && this->bindings[assoc].find(address) != this->bindings[assoc].end())
   {
      this->bindings[assoc].erase(address);

      if (this->bindings[assoc].size() == 0)
         this->bindings.erase(assoc);
   }

   this->bindings[identity].insert(address);
   address->identity = identity;
}

void
AddressPool::unbind
(Address *address)
{
   BindingMap::iterator bindIter;
   AddressSet::iterator addrIter;

   this->throwIfNotBound(*address);

   this->bindings[address->identity].erase(address);

   if (this->bindings[address->identity].size() == 0)
   {
      this->bindings.erase(address->identity);
      this->releaseIdentity(address->identity);
   }

   address->pool = NULL;
   address->identity = NULL;
}

void
AddressPool::unbindOutOfBounds
(void)
{
   BindingMap::iterator bindIter;

   bindIter = this->bindings.begin();

   while (bindIter != this->bindings.end())
   {
      if (this->inRange(*bindIter->first))
      {
         ++bindIter;
         continue;
      }

      /* if we unbind the address, we lose our iterator. compensate for this. */
      const Identity prev = bindIter->first;
      ++bindIter;
      const Identity next = (bindIter != this->bindings.end()) ? bindIter->first : NULL;
      
      /* this one's not in range, kill it. */
      this->unidentify(const_cast<Identity>(bindIter->first));

      if (next != NULL)
         bindIter = this->bindings.find(next);
   }
}

void
AddressPool::identify
(Identity identity, Label label)
{
   if (identity == NULL)
      throw NullIdentityException(*this);

   this->throwIfNoIdentity(identity);
   this->throwIfNotInRange(label);
   
   if (this->labels.count(label) == 0)
      this->labels[label] = std::set<Identity>();

   if (*identity != 0)
   {
      if (this->labels.count(*identity) > 0 && this->labels[*identity].find(identity) != this->labels[*identity].end())
         throw IdentityAlreadyLabeledException(*this, identity);

      if (this->labels[label].find(identity) != this->labels[label].end())
         throw IdentityAlreadyLabeledException(*this, identity);
   }

   this->labels[label].insert(identity);
   *identity = label;
}

void
AddressPool::reidentify
(Identity identity, Label newLabel)
{
   LabelMap::iterator labelIter;
   
   if (identity == NULL)
      throw NullIdentityException(*this);

   this->throwIfNoIdentity(identity);
   this->throwIfNotInRange(newLabel);

   /* this identity hasn't been labeled. label it and leave. */
   if (*identity == 0)
      return this->identify(identity, newLabel);

   /* find the identity set associated with this identity's label. if it's found,
      find if the identity is in the identity set. if it is, erase it. */
   if (this->labels.count(*identity) > 0)
   {
      std::set<Identity>::iterator identityIter;
      identityIter = this->labels[*identity].find(identity);
      
      if (identityIter != this->labels[*identity].end())
      {
         this->labels[*identity].erase(identity);

         if (this->labels[*identity].size() == 0)
            this->labels.erase(*identity);
      }
   }

   if (this->labels.count(newLabel) == 0)
      this->labels[newLabel] = std::set<Identity>();

   this->labels[newLabel].insert(identity);
   *identity = newLabel;
}

void
AddressPool::unidentify
(Identity identity)
{
   LabelMap::iterator labelIter;
   
   if (identity == NULL)
      throw NullIdentityException(*this);

   this->throwIfNoIdentity(identity);

   labelIter = this->labels.find(*identity);

   if (labelIter == this->labels.end())
      throw IdentityNotLabeledException(*this, identity);

   if (labelIter->second.find(identity) == labelIter->second.end())
      throw IdentityNotLabeledException(*this, identity);

   this->labels[*identity].erase(identity);

   if (this->labels[*identity].size() == 0)
      this->labels.erase(*identity);

   this->identities.erase(identity);
   *identity = 0;
}

Address::Exception::Exception
(Address &address, LPWSTR message)
   : Neurology::Exception(message)
   , address(address)
{
}

Address::NoPoolException::NoPoolException
(Address &address)
   : Address::Exception(address, EXCSTR(L"Address has no pool."))
{
}

Address::NullIdentityException::NullIdentityException
(Address &address)
   : Address::Exception(address, EXCSTR(L"Address identity is null."))
{
}

Address::AddressUnderflowException::AddressUnderflowException
(Address &address, const std::intptr_t shift)
   : Address::Exception(address, EXCSTR(L"Shift value underflowed the address."))
   , shift(shift)
{
}

Address::AddressOverflowException::AddressOverflowException
(Address &address, const std::intptr_t shift)
   : Address::Exception(address, EXCSTR(L"Shift value overflowed the address."))
   , shift(shift)
{
}

Address::Address
(AddressPool *pool)
   : pool(pool)
   , identity(NULL)
{
   this->throwIfNoPool();
}

Address::Address
(void)
   : pool(NULL)
   , identity(NULL)
{
}

Address::Address
(const LPVOID pointer)
   : pool(&AddressPool::Instance)
{
   if (this->pool->hasLabel(pointer))
      this->identity = this->pool->getIdentity(pointer);
   else
      this->identity = this->pool->newIdentity(pointer);

   this->pool->bind(this, this->identity);
}

Address::Address
(Label label)
   : pool(&AddressPool::Instance)
{
   if (this->pool->hasLabel(label))
      this->identity = this->pool->getIdentity(label);
   else
      this->identity = this->pool->newIdentity(label);

   this->pool->bind(this, this->identity);
}

Address::Address
(unsigned int lowLabel)
   : pool(&AddressPool::Instance)
{
   if (this->pool->hasLabel(lowLabel))
      this->identity = this->pool->getIdentity(lowLabel);
   else
      this->identity = this->pool->newIdentity(lowLabel);

   this->pool->bind(this, this->identity);
}

Address::Address
(const Address &address)
   : pool(NULL)
{
   *this = address;
}

Address::~Address
(void)
{
   if (this->hasPool() && this->pool->isBound(*this))
      this->pool->unbind(this);
}

Address::operator Label
(void) const
{
   return this->label();
}

void
Address::operator=
(const Address &address)
{
   if (!address.hasPool())
   {
      if (!this->hasPool())
         return;

      if (this->pool->isBound(*this))
         this->pool->unbind(this);

      this->pool = NULL;
      return;
   }
   
   address.pool->throwIfNotBound(address);

   if (this->hasPool() && this->pool->isBound(*this))
      this->pool->unbind(this);

   this->pool = address.pool;
   this->pool->bind(this, address.identity);
}

bool
Address::operator<
(const Address &address) const
{
   return *this < address.label();
}

bool
Address::operator<
(Label label) const
{
   return this->label() < label;
}

bool
Address::operator>
(const Address &address) const
{
   return address < this->label();
}

bool
Address::operator>
(Label label) const
{
   return label < this->label();
}

bool
Address::operator==
(const Address &address) const
{
   return !(*this < address) && !(address < *this);
}

bool
Address::operator==
(Label label) const
{
   return !(*this < label) && !(label < *this);
}

bool
Address::operator!=
(const Address &address) const
{
   return !(*this == address);
}

bool
Address::operator!=
(Label label) const
{
   return !(*this == label);
}

bool
Address::operator<=
(const Address &address) const
{
   return *this < address || *this == address;
}

bool
Address::operator<=
(Label label) const
{
   return *this < label || *this == label;
}

bool
Address::operator>=
(const Address &address) const
{
   return *this > address || *this == address;
}

bool
Address::operator>=
(Label label) const
{
   return *this > label || *this == label;
}

Address
Address::operator+
(std::intptr_t shift) const
{
   std::uintptr_t newValue = this->label() + shift;

   if (shift > 0 && newValue < this->label())
      throw AddressOverflowException(*const_cast<Address *>(this), shift);
   else if (shift < 0 && newValue > this->label())
      throw AddressUnderflowException(*const_cast<Address *>(this), shift);
   
   return this->pool->address(newValue);
}

Address
Address::operator+
(std::uintptr_t shift) const
{
   std::uintptr_t newValue = this->label() + shift;

   if (newValue < this->label())
      throw AddressOverflowException(*const_cast<Address *>(this), shift);
   
   return this->pool->address(newValue);
}

Address
Address::operator+
(int shift) const
{
   /* merely here because C++ doesn't know what to do with a basic int... sigh. 
      you would think it would upcast the int to std::intptr_t and extend the
      signage, but no, it just gets confused. */
   return this->operator+(static_cast<std::intptr_t>(shift));
}

Address
Address::operator-
(std::intptr_t shift) const
{
   std::uintptr_t newValue = this->label() - shift;

   if (shift > 0 && newValue > this->label())
      throw AddressUnderflowException(*const_cast<Address *>(this), shift);
   else if (shift < 0 && newValue < this->label())
      throw AddressOverflowException(*const_cast<Address *>(this), shift);
   
   return this->pool->address(this->label() - shift);
}

Address
Address::operator-
(std::uintptr_t shift) const
{
   std::uintptr_t newValue = this->label() - shift;

   if (newValue > this->label())
      throw AddressUnderflowException(*const_cast<Address *>(this), shift);
   
   return this->pool->address(this->label() - shift);
}

Address
Address::operator-
(int shift) const
{
   return this->operator-(static_cast<std::intptr_t>(shift));
}

std::intptr_t
Address::operator-
(const Address &address) const
{
   return this->label() - address.label();
}

Address &
Address::operator+=
(std::intptr_t shift)
{
   std::uintptr_t newValue = this->label() + shift;

   if (shift > 0 && newValue < this->label())
      throw AddressOverflowException(*const_cast<Address *>(this), shift);
   else if (shift < 0 && newValue > this->label())
      throw AddressUnderflowException(*const_cast<Address *>(this), shift);

   this->move(newValue);
   
   return *this;
}

Address &
Address::operator+=
(std::uintptr_t shift)
{
   std::uintptr_t newValue = this->label() + shift;

   if (newValue < this->label())
      throw AddressOverflowException(*const_cast<Address *>(this), shift);

   this->move(newValue);
   
   return *this;
}

Address &
Address::operator+=
(int shift)
{
   return this->operator+=(static_cast<std::intptr_t>(shift));
}

Address &
Address::operator-=
(std::intptr_t shift)
{
   std::uintptr_t newValue = this->label() - shift;

   if (shift > 0 && newValue > this->label())
      throw AddressUnderflowException(*this, shift);
   else if (shift < 0 && newValue < this->label())
      throw AddressOverflowException(*this, shift);

   this->move(newValue);
   return *this;
}

Address &
Address::operator-=
(std::uintptr_t shift)
{
   std::uintptr_t newValue = this->label() - shift;

   if (newValue > this->label())
      throw AddressUnderflowException(*this, shift);

   this->move(newValue);
   return *this;
}

Address &
Address::operator-=
(int shift)
{
   return this->operator-=(static_cast<std::intptr_t>(shift));
}

LPVOID
Address::pointer
(void)
{
   return reinterpret_cast<LPVOID>(this->label());
}

const LPVOID
Address::pointer
(void) const
{
   return const_cast<const LPVOID>(
      reinterpret_cast<LPVOID>(this->label()));
}

bool
Address::hasPool
(void) const
{
   return this->pool != NULL;
}

bool
Address::isNull
(void) const
{
   return !this->hasPool() || this->identity == NULL;
}

bool
Address::usesPool
(const AddressPool *pool) const
{
   return this->pool == pool;
}

bool
Address::inRange
(void) const
{
   return this->hasPool() && !this->isNull() && this->pool->inRange(this->label());
}

bool
Address::sharesIdentity
(const Address &address) const
{
   if (!this->hasPool() || !address.hasPool())
      return false;

   return this->pool->sharesIdentity(*this, address);
}

void
Address::throwIfNull
(void) const
{
   if (!this->isNull())
      return;

   this->throwIfNoPool();

   if (this->identity == NULL)
      throw NullIdentityException(const_cast<Address &>(*this));
}

void
Address::throwIfNoPool
(void) const
{
   if (!this->hasPool())
      throw NoPoolException(*const_cast<Address *>(this));
}

void
Address::throwIfNotInRange
(void) const
{
   this->throwIfNoPool();
   this->pool->throwIfNotInRange(this->label());
}

AddressPool *
Address::getPool
(void)
{
   return this->pool;
}

const AddressPool *
Address::getPool
(void) const
{
   return this->pool;
}

void
Address::setPool
(AddressPool *pool)
{
   Label label;
   
   if (this->hasPool() && this->pool->isBound(*this))
   {
      label = this->label();
      this->pool->unbind(this);
   }
   else
      label = 0;

   this->pool = pool;

   if (pool == NULL)
      return;

   if (label != 0)
   {
      if (pool->hasLabel(label))
         pool->bind(this, pool->getIdentity(label));
      else
         pool->bind(this, pool->newIdentity(label));
   }
}         

Label
Address::label
(void) const
{
   this->throwIfNull();
   
   return *this->identity;
}

void
Address::move
(const LPVOID pointer)
{
   this->move(reinterpret_cast<Label>(pointer));
}

void
Address::move
(Label newLabel)
{
   this->throwIfNoPool();
   
   this->pool->move(*this, newLabel);
}

void
Address::moveIdentity
(const LPVOID pointer)
{
   this->moveIdentity(reinterpret_cast<Label>(pointer));
}

void
Address::moveIdentity
(Label newLabel)
{
   this->throwIfNoPool();

   this->pool->reidentify(this->identity, newLabel);
}

Address
Address::copy
(void) const
{
   this->throwIfNoPool();

   return this->pool->address(this->label());
}

Offset::Offset
(void)
   : offset(0)
{
}

Offset::Offset
(const Address &base)
   : Address(base)
   , offset(0)
{
}

Offset::Offset
(const Address &base, std::uintptr_t offset)
   : Address(base)
   , offset(offset)
{
   this->throwIfNotInRange();
}

Offset::Offset
(const Offset &offset)
{
   *this = offset;
}

void
Offset::operator=
(const Offset &offset)
{
   *static_cast<Address *>(this) = *reinterpret_cast<const Address *>(&offset);
   this->offset = offset.offset;
}

void
Offset::operator=
(const Address &address)
{
   *static_cast<Address *>(this) = address;
   this->offset = 0;
}

bool
Offset::operator<
(const Offset &offset) const
{
   return this->label() < offset.label();
}

bool
Offset::operator>
(const Offset &offset) const
{
   return offset < *this;
}

bool
Offset::operator==
(const Offset &offset) const
{
   return !(*this < offset) && !(offset < *this);
}

bool
Offset::operator!=
(const Offset &offset) const
{
   return !(*this == offset);
}

bool
Offset::operator<=
(const Offset &offset) const
{
   return *this < offset || *this == offset;
}

bool
Offset::operator>=
(const Offset &offset) const
{
   return *this > offset || *this == offset;
}

Offset
Offset::operator+
(std::intptr_t shift) const
{
   std::intptr_t newOffset = this->offset + shift;

   if (shift > 0 && newOffset < 0)
      throw Address::AddressOverflowException(*const_cast<Offset *>(this), shift);
   if (shift < 0 && newOffset > 0)
      throw Address::AddressUnderflowException(*const_cast<Offset *>(this), shift);

   if (newOffset < 0)
      return Offset((*reinterpret_cast<const Address *>(this))+newOffset);
   else
      return Offset(reinterpret_cast<const Address *>(this)->copy(), newOffset);
}

Offset
Offset::operator+
(std::uintptr_t shift) const
{
   std::uintptr_t newOffset = this->offset + shift;

   if (newOffset < this->offset)
      throw Address::AddressOverflowException(*const_cast<Offset *>(this), shift);

   return Offset(reinterpret_cast<const Address *>(this)->copy(), newOffset);
}

Offset
Offset::operator-
(std::intptr_t shift) const
{
   std::intptr_t newOffset = this->offset - shift;

   if (shift > 0 && newOffset > 0)
      throw Address::AddressOverflowException(*const_cast<Offset *>(this), shift);
   if (shift < 0 && newOffset < 0)
      throw Address::AddressUnderflowException(*const_cast<Offset *>(this), shift);

   if (newOffset < 0)
      return Offset((*reinterpret_cast<const Address *>(this))+newOffset);
   else
      return Offset(reinterpret_cast<const Address *>(this)->copy(), newOffset);
}

Offset
Offset::operator-
(std::uintptr_t shift) const
{
   std::intptr_t newOffset = this->offset - shift;

   if (newOffset > this->offset)
      throw Address::AddressUnderflowException(*const_cast<Offset *>(this), shift);

   if (newOffset < 0)
      return Offset((*reinterpret_cast<const Address *>(this))+newOffset);
   else
      return Offset(reinterpret_cast<const Address *>(this)->copy(), newOffset);
}

std::intptr_t
Offset::operator-
(const Offset &offset) const
{
   return this->label() - offset.label();
}

Offset &
Offset::operator+=
(std::intptr_t shift)
{
   std::intptr_t newOffset = this->offset + shift;

   if (shift > 0 && newOffset < 0)
      throw Address::AddressOverflowException(*const_cast<Offset *>(this), shift);
   if (shift < 0 && newOffset > 0)
      throw Address::AddressUnderflowException(*const_cast<Offset *>(this), shift);

   if (newOffset < 0)
   {
      const Address *self = reinterpret_cast<const Address *>(this);
      this->move((*self+newOffset).label());
      this->offset = 0;
   }
   else
   {
      this->offset = newOffset;
      this->throwIfNotInRange();
   }

   return *this;
}

Offset &
Offset::operator+=
(std::uintptr_t shift)
{
   std::uintptr_t newOffset = this->offset + shift;

   if (newOffset < this->offset)
      throw Address::AddressOverflowException(*const_cast<Offset *>(this), shift);

   this->offset = newOffset;
   this->throwIfNotInRange();

   return *this;
}

Offset &
Offset::operator-=
(std::intptr_t shift)
{
   std::intptr_t newOffset = this->offset - shift;

   if (shift > 0 && newOffset > 0)
      throw Address::AddressOverflowException(*const_cast<Offset *>(this), shift);
   if (shift < 0 && newOffset < 0)
      throw Address::AddressUnderflowException(*const_cast<Offset *>(this), shift);

   if (newOffset < 0)
   {
      const Address *self = reinterpret_cast<const Address *>(this);
      this->move((*self+newOffset).label());
      this->offset = 0;
   }
   else
   {
      this->offset = newOffset;
      this->throwIfNotInRange();
   }

   return *this;
}

Offset &
Offset::operator-=
(std::uintptr_t shift)
{
   std::intptr_t newOffset = this->offset - shift;

   if (newOffset > this->offset)
      throw Address::AddressUnderflowException(*const_cast<Offset *>(this), shift);

   if (newOffset < 0)
   {
      const Address *self = reinterpret_cast<const Address *>(this);
      this->move((*self+newOffset).label());
      this->offset = 0;
   }
   else
   {
      this->offset = newOffset;
      this->throwIfNotInRange();
   }

   return *this;
}

Address
Offset::operator*
(void)
{
   return this->address();
}

const Address
Offset::operator*
(void) const
{
   return this->address();
}

Address
Offset::address
(void)
{
   this->throwIfNoPool();

   return this->pool->address(this->label());
}

const Address
Offset::address
(void) const
{
   this->throwIfNoPool();

   return this->pool->address(this->label());
}

Address &
Offset::getBaseAddress
(void)
{
   return *static_cast<Address *>(this);
}

const Address &
Offset::getBaseAddress
(void) const
{
   return *reinterpret_cast<const Address *>(this);
}

void
Offset::setAddress
(Address &address)
{
   *static_cast<Address *>(this) = address;
}

std::uintptr_t
Offset::getOffset
(void) const
{
   return this->offset;
}

void
Offset::setOffset
(std::uintptr_t offset)
{
   this->offset = offset;
}

void
Offset::setOffset
(Address &address, std::uintptr_t offset)
{
   this->setAddress(address);
   this->setOffset(offset);
}

Label
Offset::label
(void) const
{
   return Address::label() + this->offset;
}

Offset
Offset::copy
(void) const
{
   return Offset(*const_cast<Address *>(reinterpret_cast<const Address *>(this)), this->offset);
}
