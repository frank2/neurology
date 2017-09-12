#include <neurology/address.hpp>

using namespace Neurology;

AddressPool AddressPool::Instance;

AddressPool::Exception::Exception
(AddressPool &pool, LPWSTR message)
   : Neurology::Exception(message)
   , pool(pool)
{
}

AddressPool::NullIdentifierException::NullIdentifierException
(AddressPool &pool)
   : AddressPool::Exception(pool, EXCSTR(L"Identifier is null."))
{
}

AddressPool::AddressAlreadyBoundException::AddressAlreadyBoundException
(AddressPool &pool, Address &address)
   : AddressPool::Exception(pool, EXCSTR(L"The provided address is already bound."))
   , address(address)
{
}

AddressPool::IdentifierAlreadyLabeledException::IdentifierAlreadyLabeledException
(AddressPool &pool, const Identifier identity)
   : AddressPool::Exception(pool, EXCSTR(L"The identifier has already been labeled."))
   , identity(identity)
{
}

AddressPool::IdentifierNotLabeledException::IdentifierNotLabeledException
(AddressPool &pool, const Identifier identity)
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

AddressPool::NoSuchIdentifierException::NoSuchIdentifierException
(AddressPool &pool, const Identifier identity)
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
   std::set<Address *>::iterator addressIter;
   std::set<Identifier>::iterator identifierIter;
   
   /* unbind all addresses from their corresponding identifiers */

   /* the caveat with unbinding is that they will modify the corresponding
      map objects when we do so, so take a different approach to iterating
      over the objects. */
   while ((bindingIter = this->bindings.begin()) != this->bindings.end())
   {
      std::set<Address *>::iterator addrIter = bindingIter->second.begin();
      this->unbind(*addrIter);
   }
   
   /* release all remaining identifiers from their corresponding labels */
   while ((labelIter = this->labels.begin()) != this->labels.end())
   {
      std::set<Identifier>::iterator identifierIter = labelIter->second.begin();
      this->releaseIdentifier(*identifierIter);
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
   targetPool.associations = this->associations;

   for (AssociationMap::iterator iter=this->associations.begin();
        iter!=this->associations.end();
        ++iter)
   {
      /* do this in lieu of setPool to avoid rebinding */
      iter->first->pool = &targetPool;
   }
}

std::set<Address>
AddressPool::pool
(void)
{
   LabelMap::iterator labelIter;
   std::set<Address> result;

   for (labelIter=this->labels.begin(); labelIter!=this->labels.end(); ++labelIter)
      result.insert(this->address(labelIter->first));

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
   return this->labels.find(label) != this->labels.end();
}

bool
AddressPool::isAssociated
(const Address &address) const
{
   return this->associations.find(const_cast<Address *>(&address)) != this->associations.end();
}

bool
AddressPool::isBound
(const Address &address) const
{
   AssociationMap::const_iterator assocIter;
   BindingMap::const_iterator bindingIter;
  
   if (!this->isAssociated(address))
      return false;

   /* I hate how the STL doesn't adhere to const correctness arbitrarily... */
   assocIter = this->associations.find(const_cast<Address *>(&address));
   bindingIter = this->bindings.find(assocIter->second);

   if (bindingIter == this->bindings.end())
      return false;

   return bindingIter->second.find(assocIter->first) != bindingIter->second.end();
}

bool
AddressPool::inRange
(Label label) const
{
   return label >= this->minLabel && label <= this->maxLabel;
}

bool
AddressPool::sharesIdentifier
(const Address &first, const Address &second) const
{
   if (!this->isAssociated(first) || !this->isAssociated(second))
      return false;

   return this->getAssociation(&first) == this->getAssociation(&second);
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
AddressPool::throwIfNotAssociated
(const Address &address) const
{
   if (!this->isAssociated(address))
      throw AddressNotAssociatedException(*const_cast<AddressPool *>(this), const_cast<Address &>(address));
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
   std::set<Identifier>::iterator identifierIter;

   if (this->hasLabel(label))
      return this->newAddress(this->getIdentifier(label));
   
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
   return this->newAddress(this->newIdentifier(label));
}

Label
AddressPool::getLabel
(const Address &address) const
{
   this->throwIfNotAssociated(address);

   /* why the STL won't have a const interface to the object you provide for find, idk.
      ask Stoustrup. leave a flaming bag of poop on his doorstep. */
   return *this->associations.find(const_cast<Address *>(&address))->second;
}

void
AddressPool::move
(const Address &address, const LPVOID pointer)
{
   return this->move(address, reinterpret_cast<Label>(pointer));
}
   
void
AddressPool::move
(const Address &address, Label newLabel)
{
   this->throwIfNotBound(address);

   if (this->hasLabel(newLabel))
      this->rebind(&address, this->getIdentifier(newLabel));
   else
      this->rebind(&address, this->newIdentifier(newLabel));
}

void
AddressPool::move
(Label priorLabel, Label newLabel)
{
   std::set<Identifier>::iterator identIter;
   
   this->throwIfNoLabel(priorLabel);

   /* reidentifying will slowly wipe out this list of labels */
   while ((identIter = this->labels[priorLabel].begin()) != this->labels[priorLabel].end())
   {
      this->reidentify(*identIter, newLabel);
      
      if (this->labels.find(priorLabel) == this->labels.end())
         break;
   }
}

void
AddressPool::shift
(std::intptr_t shift)
{
   std::set<Address> snapshot;
   std::set<Address>::iterator snapIter;

   snapshot = this->pool();

   for (snapIter=snapshot.begin(); snapIter!=snapshot.end(); ++snapIter)
   {
      std::set<Identifier>::iterator identIter;
      Label label = snapIter->label();

      while ((identIter = this->labels[label].begin()) != this->labels[label].end())
         this->reidentify(*identIter, label+shift);
   }
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

   /* create a temporary range that will include the new base as well as the
      old base, allowing everything to be moved freely. */
   this->minLabel = min(newBase, this->minLabel);
   this->maxLabel = max(newMax, this->maxLabel);

   this->shift(delta);

   this->minLabel = newBase;
   this->maxLabel = newMax;
}
   
bool
AddressPool::hasIdentifier
(const Identifier identifier) const
{
   return this->identities.find(identifier) != this->identities.end();
}

void
AddressPool::throwIfNoIdentifier
(const Identifier identifier) const
{
   if (!this->hasIdentifier(identifier))
      throw NoSuchIdentifierException(*const_cast<AddressPool *>(this), identifier);
}

Identifier
AddressPool::getIdentifier
(const LPVOID pointer) const
{
   return this->getIdentifier(reinterpret_cast<Label>(pointer));
}

Identifier
AddressPool::getIdentifier
(Label label) const
{
   this->throwIfNotInRange(label);
   this->throwIfNoLabel(label);

   /* douchebag C++:
      >wants you to enforce const correctness
      >won't properly enforce const correctness on the stl

      buddy why can't you just return a const reference for std::map::operator[]() const
    */
   return *this->labels.find(label)->second.begin();
}

Identifier
AddressPool::newIdentifier
(const LPVOID pointer)
{
   return this->newIdentifier(reinterpret_cast<Label>(pointer));
}

Identifier
AddressPool::newIdentifier
(Label label)
{
   Identifier newIdentifier;

   this->throwIfNotInRange(label);
   
   if (this->labels.find(label) == this->labels.end())
      this->labels[label] = std::set<Identifier>();

   newIdentifier = new Label(0);
   this->identities.insert(newIdentifier);
   this->identify(newIdentifier, label);

   return newIdentifier;
}

void
AddressPool::releaseIdentifier
(Identifier identifier)
{
   BindingMap::iterator bindingIter;
   LabelMap::iterator labelIter;
   
   this->throwIfNoIdentifier(identifier);

   while ((bindingIter = this->bindings.find(identifier)) != this->bindings.end())
   {
      std::set<Address *>::iterator addrIter = bindingIter->second.begin();
      this->unbind(*addrIter);
   }

   /* if the unbinding of all its addresses killed the identifier, we're done here. */
   if (!this->hasIdentifier(identifier))
      return;

   labelIter = this->labels.find(*identifier);

   if (labelIter != this->labels.end())
      this->unidentify(identifier);

   delete identifier;
}

Address
AddressPool::newAddress
(Identifier identifier)
{
   this->throwIfNoIdentifier(identifier);

   Address newAddress(this);
   this->bind(&newAddress, identifier);
   return newAddress;
}

Identifier
AddressPool::getAssociation
(const Address *address) const
{
   this->throwIfNotAssociated(*address);

   /* I don't have anything witty to say here. I'm sure you can find my stupid childish commentary
      on this earlier in the file. <3 */
   return this->associations.find(const_cast<Address *>(address))->second;
}

void
AddressPool::bind
(const Address *address, const Identifier identifier)
{
   BindingMap::iterator bindingIter;
   /* STOUSTRUP WHY HAVE YOU BETRAYED ME */
   Address *deconst = const_cast<Address *>(address);
   
   this->throwIfNoIdentifier(identifier);

   bindingIter = this->bindings.find(identifier);

   if (bindingIter == this->bindings.end())
      this->bindings[identifier] = std::set<Address *>();
   else if (bindingIter->second.find(deconst) != bindingIter->second.end())
      throw AddressAlreadyBoundException(*this, *const_cast<Address *>(address));

   this->bindings[identifier].insert(deconst);
   this->associations[deconst] = identifier;
}

void
AddressPool::rebind
(const Address *address, const Identifier identifier)
{
   Identifier assoc;
   BindingMap::iterator bindingIter;
   Address *deconst = const_cast<Address *>(address);

   this->throwIfNotAssociated(*address);
   this->throwIfNoIdentifier(identifier);

   assoc = this->associations[deconst];
   bindingIter = this->bindings.find(assoc);

   if (bindingIter != this->bindings.end() && bindingIter->second.find(deconst) != bindingIter->second.end())
   {
      bindingIter->second.erase(deconst);

      if (bindingIter->second.size() == 0)
         this->bindings.erase(assoc);
   }

   bindingIter = this->bindings.find(identifier);

   if (bindingIter == this->bindings.end())
      this->bindings[identifier] = std::set<Address *>();

   this->bindings[identifier].insert(deconst);
   this->associations[deconst] = identifier;
}

void
AddressPool::unbind
(const Address *address)
{
   Identifier assoc;
   BindingMap::iterator bindIter;
   std::set<Address *>::iterator addrIter;
   Address *deconst = const_cast<Address *>(address);

   this->throwIfNotAssociated(*address);
   this->throwIfNotBound(*address);

   assoc = this->associations[deconst];
   this->associations.erase(deconst);
   this->bindings[assoc].erase(deconst);

   if (this->bindings[assoc].size() == 0)
   {
      this->bindings.erase(assoc);
      this->releaseIdentifier(assoc);
   }
}

void
AddressPool::unbindOutOfBounds
(void)
{
   AssociationMap::iterator assocIter;

   assocIter = this->associations.begin();

   while (assocIter != this->associations.end())
   {
      Address *prev, *next;
      if (this->inRange(assocIter->first->label()))
      {
         ++assocIter;
         continue;
      }

      /* if we unbind the address, we lose our iterator. compensate for this. */
      prev = assocIter->first;
      ++assocIter;

      if (assocIter != this->associations.end())
         next = assocIter->first;
      else
         next = NULL;
      
      /* this one's not in range, kill it. */
      this->unbind(assocIter->first);

      if (next != NULL)
         assocIter = this->associations.find(next);
   }
}

void
AddressPool::identify
(Identifier identifier, Label label)
{
   if (identifier == NULL)
      throw NullIdentifierException(*this);

   this->throwIfNoIdentifier(identifier);
   this->throwIfNotInRange(label);
   
   if (this->labels.find(label) == this->labels.end())
      this->labels[label] = std::set<Identifier>();

   if (identifier != 0)
   {
      if (this->labels.find(*identifier) != this->labels.end() && this->labels[*identifier].find(identifier) != this->labels[*identifier].end())
         throw IdentifierAlreadyLabeledException(*this, identifier);

      if (this->labels[label].find(identifier) != this->labels[label].end())
         throw IdentifierAlreadyLabeledException(*this, identifier);
   }

   this->labels[label].insert(identifier);
   *identifier = label;
}

void
AddressPool::reidentify
(Identifier identifier, Label newLabel)
{
   LabelMap::iterator labelIter;
   
   if (identifier == NULL)
      throw NullIdentifierException(*this);

   this->throwIfNoIdentifier(identifier);
   this->throwIfNotInRange(newLabel);

   /* this identifier hasn't been labeled. label it and leave. */
   if (*identifier == 0)
      return this->identify(identifier, newLabel);

   /* find the identifier set associated with this identifier's label. if it's found,
      find if the identifier is in the identifier set. if it is, erase it. */
   labelIter = this->labels.find(*identifier);

   if (labelIter != this->labels.end())
   {
      std::set<Identifier>::iterator identifierIter;
      identifierIter = labelIter->second.find(identifier);
      
      if (identifierIter != labelIter->second.end())
      {
         labelIter->second.erase(identifier);

         if (labelIter->second.size() == 0)
            this->labels.erase(*identifier);
      }
   }

   if (this->labels.find(newLabel) == this->labels.end())
      this->labels[newLabel] = std::set<Identifier>();

   this->labels[newLabel].insert(identifier);
   *identifier = newLabel;
}

void
AddressPool::unidentify
(Identifier identifier)
{
   LabelMap::iterator labelIter;
   
   if (identifier == NULL)
      throw NullIdentifierException(*this);

   this->throwIfNoIdentifier(identifier);

   labelIter = this->labels.find(*identifier);

   if (labelIter == this->labels.end())
      throw IdentifierNotLabeledException(*this, identifier);

   if (labelIter->second.find(identifier) == labelIter->second.end())
      throw IdentifierNotLabeledException(*this, identifier);

   this->labels[*identifier].erase(identifier);

   if (this->labels[*identifier].size() == 0)
      this->labels.erase(*identifier);

   this->identities.erase(identifier);
   *identifier = 0;
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
{
   this->throwIfNoPool();
}

Address::Address
(void)
   : pool(NULL)
{
}

Address::Address
(const LPVOID pointer)
   : pool(&AddressPool::Instance)
{
   Identifier newIdentifier;

   if (this->pool->hasLabel(pointer))
      newIdentifier = this->pool->getIdentifier(pointer);
   else
      newIdentifier = this->pool->newIdentifier(pointer);

   this->pool->bind(this, newIdentifier);
}

Address::Address
(Label label)
   : pool(&AddressPool::Instance)
{
   Identifier newIdentifier;

   if (this->pool->hasLabel(label))
      newIdentifier = this->pool->getIdentifier(label);
   else
      newIdentifier = this->pool->newIdentifier(label);

   this->pool->bind(this, newIdentifier);
}

Address::Address
(unsigned int lowLabel)
   : pool(&AddressPool::Instance)
{
   Identifier newIdentifier;

   if (this->pool->hasLabel(lowLabel))
      newIdentifier = this->pool->getIdentifier(lowLabel);
   else
      newIdentifier = this->pool->newIdentifier(lowLabel);

   this->pool->bind(this, newIdentifier);
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
   
   address.pool->throwIfNotAssociated(address);

   if (this->hasPool() && this->pool->isBound(*this))
      this->pool->unbind(this);

   this->pool = address.pool;
   this->pool->bind(this, address.getAssociation());
}

bool
Address::operator<
(const Address &address) const
{
   return this->label() < address.label();
}

bool
Address::operator>
(const Address &address) const
{
   return address < *this;
}

bool
Address::operator==
(const Address &address) const
{
   return !(*this < address) && !(address < *this);
}

bool
Address::operator!=
(const Address &address) const
{
   return this->label() != address.label();
}

bool
Address::operator<=
(const Address &address) const
{
   return this->label() <= address.label();
}

bool
Address::operator>=
(const Address &address) const
{
   return this->label() >= address.label();
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
   return !this->hasPool() || !this->pool->isAssociated(*this);
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
Address::sharesIdentifier
(const Address &address) const
{
   if (!this->hasPool() || !address.hasPool())
      return false;

   return this->pool->sharesIdentifier(*this, address);
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
         pool->bind(this, pool->getIdentifier(label));
      else
         pool->bind(this, pool->newIdentifier(label));
   }
}         

Label
Address::label
(void) const
{
   return *this->getAssociation();
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
   
   this->pool->move(this, newLabel);
}

void
Address::moveIdentifier
(const LPVOID pointer)
{
   this->moveIdentifier(reinterpret_cast<Label>(pointer));
}

void
Address::moveIdentifier
(Label newLabel)
{
   this->throwIfNoPool();

   this->pool->reidentify(this->pool->getAssociation(this), newLabel);
}

Address
Address::copy
(void) const
{
   this->throwIfNoPool();

   return this->pool->address(this->label());
}

const Identifier
Address::getAssociation
(void) const
{
   this->throwIfNoPool();
   
   return this->pool->getAssociation(this);
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
