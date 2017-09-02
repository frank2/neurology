#include <neurology/address.hpp>

using namespace Neurology;

AddressPool AddressPool::Instance;

LONG
Neurology::CopyData
(LPVOID destination, const LPVOID source, SIZE_T size)
{
   __try
   {
      CopyMemory(destination, source, size);
      return 0; // STATUS_SUCCESS
   }
   __except (EXCEPTION_EXECUTE_HANDLER)
   {
      return GetExceptionCode();
   }
}

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

AddressPool::AddressPool
(void)
{
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

bool
AddressPool::hasLabel
(const LPVOID pointer) const
{
   return this->hasLabel(reinterpret_cast<Label>(
                            const_cast<LPVOID>(pointer)));
}

bool
AddressPool::hasLabel
(Label label) const
{
   return this->labels.find(label) != this->labels.end();
}

bool
AddressPool::isAssociated
(const Address *address) const
{
   return this->associations.find(const_cast<Address *>(address)) != this->associations.end();
}

bool
AddressPool::isBound
(const Address *address) const
{
   AssociationMap::const_iterator assocIter;
   BindingMap::const_iterator bindingIter;
  
   if (!this->isAssociated(address))
      return false;

   /* I hate how the STL doesn't adhere to const correctness arbitrarily... */
   assocIter = this->associations.find(const_cast<Address *>(address));
   bindingIter = this->bindings.find(assocIter->second);

   if (bindingIter == this->bindings.end())
      return false;

   return bindingIter->second.find(assocIter->first) != bindingIter->second.end();
}

void
AddressPool::throwIfNoLabel
(const LPVOID pointer) const
{
   return this->throwIfNoLabel(reinterpret_cast<Label>(
                                  const_cast<LPVOID>(pointer)));
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
(const Address *address) const
{
   if (!this->isAssociated(address))
      throw AddressNotAssociatedException(*const_cast<AddressPool *>(this), *const_cast<Address *>(address));
}

void
AddressPool::throwIfNotBound
(const Address *address) const
{
   if (!this->isBound(address))
      throw AddressNotBoundException(*const_cast<AddressPool *>(this), *const_cast<Address *>(address));
}

Address
AddressPool::address
(const LPVOID pointer)
{
   return this->address(reinterpret_cast<Label>(
                           const_cast<LPVOID>(pointer)));
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
   return this->newAddress(reinterpret_cast<Label>(
                              const_cast<LPVOID>(pointer)));
}

Address
AddressPool::newAddress
(Label label)
{
   return this->newAddress(this->newIdentifier(label));
}

LPVOID
AddressPool::getPointer
(const Address *address)
{
   return reinterpret_cast<LPVOID>(this->getLabel(address));
}

const LPVOID
AddressPool::getPointer
(const Address *address) const
{
   return const_cast<const LPVOID>(
      reinterpret_cast<LPVOID>(this->getLabel(address)));
}

Label
AddressPool::getLabel
(const Address *address) const
{
   this->throwIfNotAssociated(address);

   /* why the STL won't have a const interface to the object you provide for find, idk.
      ask Stoustrup. leave a flaming bag of poop on his doorstep. */
   return *this->associations.find(const_cast<Address *>(address))->second;
}

void
AddressPool::move
(const Address *address, const LPVOID pointer)
{
   return this->move(address
                     ,reinterpret_cast<Label>(
                        const_cast<LPVOID>(pointer)));
}
   
void
AddressPool::move
(const Address *address, Label newLabel)
{
   this->throwIfNotBound(address);

   if (this->hasLabel(newLabel))
      this->rebind(address, this->getIdentifier(newLabel));
   else
      this->rebind(address, this->newIdentifier(newLabel));
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
   return this->getIdentifier(reinterpret_cast<Label>(
                                 const_cast<LPVOID>(pointer)));
}

Identifier
AddressPool::getIdentifier
(Label label) const
{
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
   return this->newIdentifier(reinterpret_cast<Label>(
                                 const_cast<LPVOID>(pointer)));
}

Identifier
AddressPool::newIdentifier
(Label label)
{
   Identifier newIdentifier;
   
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

   /* unlabel this identifier if it's in there. it may not be-- unbinding/unlabeling it may have removed it
      from the identifier map as well as the label map, but did not destroy the identifier. */
   labelIter = this->labels.find(*identifier);

   if (labelIter != this->labels.end())
      this->unidentify(identifier);

   /* did the unlabelling kill us? we're done. */
   if (!this->hasIdentifier(identifier))
      return;
   
   /* identifier is still alive. destroy it. */
   this->identities.erase(identifier);
   *identifier = 0;
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
   this->throwIfNotAssociated(address);

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

   this->throwIfNotAssociated(address);
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

   this->throwIfNotAssociated(address);
   this->throwIfNotBound(address);

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
AddressPool::identify
(Identifier identifier, Label label)
{
   if (identifier == NULL)
      throw NullIdentifierException(*this);

   this->throwIfNoIdentifier(identifier);
   
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

   /* when you unlabel a identifier, you're also destroying it. release the identifier if it's
      one of ours. although I don't know why it wouldn't... but just in case. */
   if (this->identities.find(identifier) != this->identities.end())
      this->releaseIdentifier(identifier);
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

Address::NegativeMovementException::NegativeMovementException
(Address &address, std::intptr_t)
   : Address::Exception(address, EXCSTR(L"Address has no pool."))
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
   : pool(&AddressPool::Instance)
{
}

Address::Address
(LPVOID pointer)
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
(const Address &address)
{
   *this = address;
}

Address::~Address
(void)
{
   if (this->hasPool() && this->pool->isBound(this))
      this->pool->unbind(this);
}

void
Address::operator=
(const Address &address)
{
   address.pool->throwIfNotAssociated(&address);
   this->pool = address.pool;

   if (this->pool->isBound(this))
      this->pool->rebind(this, address.getAssociation());
   else
      this->pool->bind(this, address.getAssociation());
}

void
Address::operator=
(const LPVOID pointer)
{
   this->move(pointer);
}

void
Address::operator=
(Label label)
{
   this->move(label);
}

LPVOID
Address::operator*
(void)
{
   return this->pointer();
}

const LPVOID
Address::operator*
(void) const
{
   return this->pointer();
}

LPVOID
Address::operator->
(void)
{
   return this->pointer();
}

const LPVOID
Address::operator->
(void) const
{
   return this->pointer();
}

Address
Address::operator+
(std::intptr_t shift) const
{
   return this->pool->address(this->label() + shift);
}

Address
Address::operator-
(std::intptr_t shift) const
{
   return this->pool->address(this->label() - shift);
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
   this->move(this->label() + shift);
   return *this;
}

Address &
Address::operator-=
(std::intptr_t shift)
{
   std::intptr_t result = this->label() - shift;

   if (result < 0)
      throw NegativeMovementException(*this, shift);
   
   this->move(result);
   return *this;
}

bool
Address::hasPool
(void) const
{
   return this->pool != NULL;
}

void
Address::throwIfNoPool
(void) const
{
   if (!this->hasPool())
      throw NoPoolException(*const_cast<Address *>(this));
}

Label
Address::label
(void) const
{
   return *this->getAssociation();
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

void
Address::move
(const LPVOID pointer)
{
   this->move(reinterpret_cast<Label>(
                 const_cast<LPVOID>(pointer)));
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
   this->moveIdentifier(reinterpret_cast<Label>(
                           const_cast<LPVOID>(pointer)));
}

void
Address::moveIdentifier
(Label newLabel)
{
   this->throwIfNoPool();

   this->pool->reidentify(this->pool->getAssociation(this), newLabel);
}

const Identifier
Address::getAssociation
(void) const
{
   this->throwIfNoPool();
   
   return this->pool->getAssociation(this);
}
