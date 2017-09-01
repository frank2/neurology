#include <neurology/address.hpp>

using namespace Neurology;

AddressPool AddressPool::Instance;

AddressPool::AddressPool
(void)
{
}

AddressPool::~AddressPool
(void)
{
   PointerMap::iterator bindingIter;
   LabelMap::iterator labelIter;
   
   /* unbind all addresses from their corresponding pointers */

   /* the caveat with unbinding is that they will modify the corresponding
      map objects when we do so, so take a different approach to iterating
      over the objects. */
   while ((bindingIter = this->bindings.begin()) != this->bindings.end())
   {
      std::set<Address *>::iterator addrIter = bindingIter->second.begin();
      this->unbind(*addrIter);
   }
   
   /* release all remaining pointers from their corresponding labels */
   while ((labelIter = this->labels.begin()) != this->labels.end())
   {
      std::set<Pointer>::iterator pointerIter = labelIter->second.begin();
      this->releasePointer(*pointerIter);
   }
}

bool
AddressPool::hasLabel
(const void *pointer) const
{
   return this->hasLabel(static_cast<Label>(
                            const_cast<void *>(pointer)));
}

bool
AddressPool::hasLabel
(Label label)
{
   return this->labels.find(label) != this->labels.end();
}

Address &
AddressPool::address
(void *pointer)
{
   return this->address(static_cast<Label>(pointer));
}

Address &
AddressPool::address
(Label label)
{
   Pointer pointer;

   if (!this->hasLabel(label))
      return this->newAddress(label);

   return this->newAddress(this->getPointer(label));
}

Address &
AddressPool::newAddress
(void *pointer)

{
   return this->newAddress(static_cast<Label>(pointer));
}

Address &
AddressPool::newAddress
(Label label)
{
   return this->newAddress(this->newPointer(label));
}

void
AddressPool::move
(const Address &address, const void *pointer)
{
   return this->move(address, static_cast<Label>(
                        const_cast<void *>(pointer)));
}
   
void
AddressPool::move
(const Address &address, Label newLabel)
{
   this->throwIfNoAssociation(&address);
   this->throwIfNotBound(address);

   if (this->hasLabel(newLabel))
      this->rebind(&address, this->getPointer(newLabel));
   else
      this->rebind(&address, this->newPointer(newLabel));
}

bool
AddressPool::hasPointer
(const Pointer pointer) const
{
   return this->pointers.find(const_cast<Pointer>(pointer)) != this->pointers.end();
}

Pointer
AddressPool::getPointer
(const void *pointer)
{
   return this->getPointer(static_cast<Label>(
                              const_cast<void *>(pointer)));
}

Pointer
AddressPool::getPointer
(Label label) const
{
   this->throwIfNoLabel(label);

   return *this->labels[label].begin();
}

Pointer
AddressPool::newPointer
(void *pointer)
{
   return this->newPointer(static_cast<Label>(pointer));
}

Pointer
AddressPool::newPointer
(Label label)
{
   Pointer newPointer;
   
   if (this->labels.find(label) == this->labels.end())
      this->labels[label] = std::set<Pointer>();

   newPointer = new Label(label);
   this->pointers.insert(newPointer);
   this->labels[label].insert(newPointer);
}

void
AddressPool::releasePointer
(Pointer pointer)
{
   PointerMap::iterator bindingIter;
   LabelMap::iterator labelIter;
   
   this->throwIfNoPointer(pointer);

   while ((bindingIter = this->bindings.find(pointer)) != this->bindings.end())
   {
      std::set<Address *>::iterator addrIter = bindingIter->second.begin();
      this->unbind(*addrIter);
   }

   /* if the unbinding of all its addresses killed the pointer, we're done here. */
   if (!this->hasPointer(pointer))
      return;

   /* unlabel this pointer if it's in there. it may not be-- unbinding/unlabeling it may have removed it
      from the pointer map as well as the label map, but did not destroy the pointer. */
   labelIter = this->labels.find(*pointer);

   if (labelIter != this->labels.end())
      this->unlabel(pointer);

   /* did the unlabelling kill us? we're done. */
   if (!this->hasPointer(pointer))
      return;
   
   /* pointer is still alive. destroy it. */
   this->pointers.erase(pointer);
   *pointer = 0;
   delete pointer;
}

Address &
AddressPool::newAddress
(Pointer pointer)
{
   Address *newAddress;
   
   this->throwIfNoPointer(pointer);
   
   newAddress = new Address(this);
   this->addresses.insert(newAddress);
   this->bind(newAddress, pointer);

   return *newAddress;
}

void
AddressPool::bind
(Address *address, Pointer pointer)
{
   PointerMap::iterator bindingIter;
   
   this->throwIfNoPointer(pointer);

   bindingIter = this->bindings.find(pointer);

   if (bindingIter == this->bindings.end())
      this->bindings[pointer] = std::set<Address *>();
   else if (bindingIter->second.find(address) != bindingIter->second.end())
      throw AddressAlreadyBoundException(*this, address);

   this->bindings[pointer].insert(address);
   this->associations[address] = pointer;
}

void
AddressPool::rebind
(Address *address, Pointer pointer)
{
   PointerMap::iterator bindingIter;
   
   this->throwIfNoPointer(address->pointer);
   this->throwIfNoPointer(pointer);

   bindingIter = this->bindings.find(address->pointer);

   if (bindingIter != this->bindings.end() && bindingIter->second.find(address) != bindingIter->second.end())
   {
      bindingIter->second.erase(address);

      if (bindingIter->second.size() == 0)
         this->bindings.erase(address->pointer);
   }

   bindingIter = this->bindings.find(pointer);

   if (bindingIter == this->bindings.end())
      this->bindings[pointer] = std::set<Address *>();

   if (this->associations.find(address) != this->associations.end())
      this->associations.erase(address);
   
   this->bindings[pointer] = address;
   this->associations[address] = pointer;
}

void
AddressPool::unbind
(Address *address)
{
   AssocMap::iterator assocIter;
   PointerMap::iterator bindIter;
   std::set<Address *>::iterator addrIter;
   Pointer pointer;
   
   this->throwIfNoPointer(address->pointer);

   assocIter = this->associations.find(address);

   if (assocIter == this->associations.end())
      throw AddressNotAssociatedException(*this);

   pointer = *assocIter;
   bindIter = this->bindings.find(pointer);

   if (bindIter == this->bindings.end())
      throw AddressNotBoundException(*this);
   
   addrIter = bindIter->second.find(address);

   if (addrIter == bindIter->second.end())
      throw AddressNotBoundException(*this, address);

   this->associations.erase(address);
   bindIter->second.erase(address);

   if (bindIter->second.size() == 0)
   {
      pointer = bindIter->first;
      
      this->bindings.erase(pointer);
      this->releasePointer(pointer);
   }

   if (this->associations.find(address) != this->associations.end())
      this->associations.erase(address);

   /* if this address is actually one of ours, destroy it. */
   if (this->addresses.find(address))
   {
      this->addresses.erase(address);
      address->pool = NULL;
      delete address;
   }
}

void
AddressPool::label
(Pointer pointer, Label label)
{
   if (pointer == NULL)
      throw NullPointerException();

   this->throwIfNoPointer(pointer);
   
   if (this->labels.find(label) == this->labels.end())
      this->labels[label] = std::set<Pointer>();

   if (pointer != 0)
   {
      if (this->labels.find(*pointer) != this->labels.end() && this->labels[*pointer].find(pointer) != this->labels[*pointer].end())
         throw PointerAlreadyLabeledException(*this, pointer);

      if (this->labels[label].find(pointer))
         throw PointerAlreadyLabeledException(*this, pointer);
   }

   this->labels[label].insert(pointer);
   *pointer = label;
}

void
AddressPool::relabel
(Pointer pointer, Label newLabel)
{
   LabelMap::iterator labelIter;
   
   if (pointer == NULL)
      throw NullPointerException();

   this->throwIfNoPointer(pointer);

   /* this pointer hasn't been labeled. label it and leave. */
   if (*pointer == 0)
      return this->label(pointer, newLabel);
   
   labelIter = this->labels.find(*pointer);

   if (labelIter != this->labels.end())
   {
      std::set<Pointer>::iterator = pointerIter;

      pointerIter = labelIter->second.find(pointer);
      
      if (pointerIter != labelIter->second.end())
      {
         labelIter->second.erase(pointer);

         if (labelIter->second.size() == 0)
            this->labels.erase(*pointer);
      }
   }

   if (this->labels.find(newLabel) == this->labels.end())
      this->labels[newLabel] = std::set<Pointer>();

   this->labels[newLabel].insert(pointer);
   *pointer = newLabel;
}

void
AddressPool::unlabel
(Pointer pointer)
{
   LabelMap::iterator labelIter;
   
   if (pointer == NULL)
      throw NullPointerException();

   this->throwIfNoPointer(pointer);

   labelIter = this->labels.find(*pointer);

   if (labelIter == this->labels.end())
      throw PointerNotLabelledException(*this, pointer);

   if (labelIter->second.find(pointer) == labelIter->second.end())
      throw PointerNotLabelledException(*this, pointer);

   this->labels[*pointer].erase(pointer);

   if (this->labels[*pointer].size() == 0)
      this->labels.erase(*pointer);

   /* when you unlabel a pointer, you're also destroying it. release the pointer if it's
      one of ours. */
   if (this->pointers.find(pointer) != this->pointers.end())
      this->releasePointer(pointer);
}

Address::Address
(AddressPool *pool)
   : pool(pool)
{
}

Address::Address
(void)
   : pool(AddressPool::Instance)
{
}

Address::Address
(void *pointer)
   : pool(AddressPool::Instance)
{
   Pointer newPointer;
   
   if (this->pool == NULL)
      throw NullPointerException();

   if (this->pool->hasLabel(pointer))
      newPointer = this->pool->getPointer(pointer);
   else
      newPointer = this->pool->newPointer(pointer);

   this->pool->bind(this, newPointer);
}

Address::Address
(Label label)
   : pool(AddressPool::Instance)
{
   Pointer newPointer;
   
   if (this->pool == NULL)
      throw NullPointerException();

   if (this->pool->hasLabel(label))
      newPointer = this->pool->getPointer(label);
   else
      newPointer = this->pool->newPointer(label);

   this->pool->bind(this, newPointer);
}

Address::Address
(const Address &address)
{
   *this = address;
}

Address::~Address
(void)
{
   if (this->pool != NULL && this->isBound(*this))
      this->unbind(this);
}

void
Address::operator=
(const Address &address)
{
   this->pool = address.pool;

   if (this->isBound())
      this->pool->rebind(this, address.getAssociation());
   else
      this->pool->bind(this, address.getAssociation());
}

void
Address::operator=
(const void *pointer)
{
   this->move(pointer);
}

void
Address::operator=
(Label label)
{
   this->move(label);
}

Label
Address::label
(void) const
{
   return *this->getAssociation();
}

void *
Address::address
(void)
{
   return static_cast<void *>(*this->getAssociation());
}

const void *
Address::address
(void) const
{
   return const_cast<const void *>(
      static_cast<void *>(*this->getAssociation()));
}

void
Address::move
(const void *pointer)
{
   this->move(static_cast<Label>(
                 const_cast<void *>(pointer)));
}

void
Address::move
(Label newLabel)
{
   this->throwIfNoPool();
   
   this->pool->move(*this, label);
}

void
Address::movePointer
(const void *pointer)
{
   this->movePointer(static_cast<Label>(
                        const_cast<void *>(pointer)));
}

void
Address::movePointer
(Label newLabel)
{
   this->throwIfNoPool();

   this->move(this->pool->getAssociation(this), newLabel);
}

const Pointer
Address::getAssociation
(void) const
{
   this->throwIfNoPool();
   
   return this->pool->getAssociation(this);
}
