#include <neurology/memory.hpp>

using namespace Neurology;

NullPointerException::NullPointerException
(void)
   : NeurologyException("pointer pointer is null")
{
}

NullPointerException::NullPointerException
(NullPointerException &exception)
   : NeurologyException(exception)
{
}

BadPointerException::BadPointerException
(LPVOID address, SIZE_T size)
   : NeurologyException("provided pointer is bad")
{
   this->address = address;
   this->size = size;
}

BadPointerException::BadPointerException
(BadPointerException &exception)
   : NeurologyException(exception)
{
   this->address = exception.address;
   this->size = exception.size;
}

AddressOutOfBoundsException::AddressOutOfBoundsException
(Memory *memory, LPVOID address, SIZE_T size)
   : NeurologyException("offset and size are out of bounds in region")
{
   this->memory = memory;
   this->address = address;
   this->size = size;
}

AddressOutOfBoundsException::AddressOutOfBoundsException
(AddressOutOfBoundsException &exception)
   : NeurologyException(exception)
{
   this->memory = exception.memory;
   this->address = exception.address;
   this->size = exception.size;
}

OffsetOutOfBoundsException::OffsetOutOfBoundsException
(Memory *memory, SIZE_T offset, SIZE_T size)
   : NeurologyException("offset and size are out of bounds in region")
{
   this->memory = memory;
   this->offset = offset;
   this->size = size;
}

OffsetOutOfBoundsException::OffsetOutOfBoundsException
(OffsetOutOfBoundsException &exception)
   : NeurologyException(exception)
{
   this->memory = exception.memory;
   this->offset = exception.offset;
   this->size = exception.size;
}
   
Memory::Memory
(void)
{
   this->parent = NULL;
   this->pointerRef = NULL;
   this->sizeRef = NULL;
   this->refCount = NULL;
}

Memory::Memory
(LPVOID pointer, SIZE_T size)
{
   this->parent = NULL;
   this->setPointer(pointer);
   this->setSize(size);
   this->ref();
}

Memory::Memory
(Memory *parent, SIZE_T offset, SIZE_T size)
{
   this->parent = parent;

   if (parent == NULL)
      throw NullPointerException();
   
   this->setOffset(offset);
   this->setSize(size);
   this->ref();
}

Memory::Memory
(Memory &memory)
{
   this->deref();

   this->parent = memory.parent;
   this->pointerRef = memory.pointerRef;
   this->sizeRef = memory.sizeRef;
   this->refCount = memory.refCount;

   this->ref();
}

Memory::~Memory
(void)
{
   this->deref();
}

void
Memory::ref
(void)
{
   if (this->refCount == NULL)
   {
      this->refCount = new DWORD;
      *this->refCount = 1;
   }
   else
      ++*this->refCount;

   if (this->parent != NULL)
      this->parent->ref();
}

void
Memory::deref
(void)
{
   if (this->refCount == NULL)
      return;

   --*this->refCount;

   if (*this->refCount <= 0)
   {
      if (this->pointerRef != NULL)
      {
         delete this->pointerRef;
         this->pointerRef = NULL;
      }

      if (this->sizeRef != NULL)
      {
         delete this->sizeRef;
         this->sizeRef = NULL;
      }

      delete this->refCount;
      this->refCount = NULL;
   }
}

DWORD
Memory::refs
(void)
{
   if (this->refCount == NULL)
      return 0;

   return *this->refCount;
}

bool
Memory::isValid
(void)
{
   bool result = this->pointerRef != NULL;

   if (this->parent.refs() > 0)
      result &= this->parent.isValid();
   else if (result)
      result &= *this->pointerRef != NULL;

   return result && this->size() != 0;
}

void
Memory::invalidate
(void)
{
   if (this->pointerRef == NULL)
      throw NullPointerException();

   if (this->sizeRef == NULL)
      throw NullPointerException();

   *this->pointerRef = NULL;
   *this->sizeRef = 0;
}

SIZE_T
Memory::size
(void)
{
   if (this->size == NULL)
      return 0;
   
   return *this->size;
}

Address
Memory::address
(void)
{
   if (this->parent != NULL)
   {
      if (this->offsetRef == NULL)
         throw NullPointerException();
      
      return this->parent->address(*this->offsetRef);
   }
   
   return this->address(0);
}

Address
Memory::address
(SIZE_T offset)
{
   return Address(this, offset);
}

LPVOID
Memory::pointer
(void)
{
   if (this->parent != NULL)
   {
      if (this->offsetRef == NULL)
         throw NullPointerException();
      
      return this->parent->pointer(*this->offsetRef);
   }

   return this->pointer(0);
}

LPVOID
Memory::pointer
(SIZE_T offset)
{
   if (this->pointerRef == NULL || this->sizeRef == NULL)
      throw NullPointerException();

   if (this->parent != NULL)
      return this->parent->pointer(offset + *this->offsetRef);

   if (offset > *this->sizeRef)
      throw OffsetOutOfBoundsException(this, offset, 0);

   return (LPVOID)((LPBYTE)*this->pointerRef + offset);
}

SIZE_T
Memory::offset
(LPVOID address)
{
   if (!this->inRange(address))
      throw AddressOutOfBoundsException(this, address, 0);

   return (SIZE_T)address - (SIZE_T)this->pointer();
}
   
LPVOID
Memory::start
(void)
{
   return this->pointer();
}

LPVOID
Memory::end
(void)
{
   if (this->sizeRef == NULL)
      throw NullPointerException();
   
   if (*this->sizeRef == 0)
      return this->start();

   return this->pointer(*this->sizeRef);
}

bool
Memory::inRange
(LPVOID address)
{
   return this->pointerRef != NULL && address >= this->start() && address <= this->end();
}

bool
Memory::inRange
(LPVOID address, SIZE_T size)
{
   LPBYTE calcAddress = (LPBYTE)address+size;
   
   return this->inRange(address) && calcAddress >= this->start() && calcAddress <= this->end();
}

void
Memory::setPointer
(LPVOID base)
{
   if (this->pointerRef == NULL)
      this->pointerRef = new LPVOID;

   *this->pointer = base;
}

void
Memory::setSize
(SIZE_T size)
{
   if (this->sizeRef == NULL)
      this->sizeRef = new SIZE_T;

   *this->sizeRef = size;
}

Data
Memory::read
(void)
{
   if (this->sizeRef == NULL)
      throw NullPointerException();
   
   return this->read(*this->sizeRef);
}

Data
Memory::read
(SIZE_T size)
{
   return this->read((SIZE_T)0, size);
}

Data
Memory::read
(SIZE_T offset, SIZE_T size)
{
   try
   {
      return this->read(this->pointer(offset), size);
   }
   catch (AddressOutOfBoundsException &exception)
   {
      throw OffsetOutOfBoundsException(exception.memory, offset, exception.size);
   }
}

Data
Memory::read
(LPVOID address, SIZE_T size)
{
   Data result;
   
   if (this->pointerRef == NULL || *this->pointerRef == NULL)
      throw NullPointerException();

   if (!this->inRange(address, size))
      throw AddressOutOfBoundsException(this, address, size);

   if (size == 0)
      return result;

   if (this->parent != NULL)
      return this->parent->read(address, size);

   __try
   {
      result = BlockData(address, size);
   }
   __except(EXCEPTION_EXECUTE_HANDLER)
   {
      throw BadPointerException(address, size);
   }

   return result;
}

void
Memory::write
(Memory *region)
{
   this->write(region->read());
}

void
Memory::write
(Data data)
{
   this->write((SIZE_T)0, data);
}

void
Memory::write
(SIZE_T offset, Memory *region)
{
   this->write(offset, region->read());
}

void
Memory::write
(SIZE_T offset, Data data)
{
   try
   {
      this->write(this->pointerAddress(offset), data);
   }
   catch (AddressOutOfBoundsException &exception)
   {
      throw OffsetOutOfBoundsException(exception.memory, offset, exception.size);
   }
}

void
Memory::write
(LPVOID address, Memory *region)
{
   this->write(address, region->read());
}

void
Memory::write
(LPVOID address, Data data)
{
   if (this->pointerRef == NULL || *this->pointerRef == NULL)
      throw NullPointerException();

   if (!this->inRange(address, data.size()))
      throw AddressOutOfBoundsException(this, address, data.size());

   if (data.size() == 0)
      return;

   if (this->parent != NULL)
      return this->parent->write(address, data);
   
   __try
   {
      CopyMemory(address, data.data(), data.size());
   }
   __except (EXCEPTION_EXECUTE_HANDLER)
   {
      throw BadPointerException(address, data.size());
   }
}

Address::Address
(void)
{
   this->memory = NULL;
   this->offset = 0;
}

Address::Address
(Memory *memory)
{
   if (memory == NULL)
      throw NullPointerException();

   this->memory = memory;
   this->offset = 0;

   this->memory->ref();
}

Address::Address
(Memory *memory, SIZE_T offset)
{
   if (memory == NULL)
      throw NullPointerException();

   if (!memory->inRange(memory->pointerAddress(offset)))
      throw OffsetOutOfBoundsException(memory, offset, 0);

   this->memory = memory;
   this->offset = offset;

   this->memory->ref();
}

Address::Address
(Address &address)
{
   if (this->memory != NULL)
      this->memory->deref();
   
   this->memory = address.memory;
   this->offset = address.offset;

   if (this->memory != NULL)
      this->memory->ref();
}

Address::~Address
(void)
{
   if (this->memory != NULL)
      this->memory->deref();

   this->memory = NULL;
   this->offset = 0;
}

LPVOID
Address::pointer
(void)
{
   return this->pointer(0);
}

LPVOID
Address::pointer
(SIZE_T offset)
{
   LPVOID addr;
   
   if (this->memory == NULL)
      throw NullPointerException();

   return this->memory->pointer(this->offset + offset);
}
   
Data
Address::read
(SIZE_T size)
{
   return this->read(0, size);
}

Data
Address::read
(SIZE_T offset, SIZE_T size)
{
   if (this->memory == NULL)
      throw NullPointerException();

   try
   {
      return this->memory->read(this->pointer(this->offset+offset), size);
   }
   catch (AddressOutOfBoundsException &exception)
   {
      throw OffsetOutOfBoundsException(exception.memory, this->offset+offset, size);
   }
}

void
Address::write
(Data data)
{
   this->write(0, data);
}

void
Address::write
(SIZE_T offset, Data data)
{
   if (this->memory == NULL)
      throw NullPointerException();

   try
   {
      this->memory->write(this->pointer(this->offset+offset), data);
   }
   catch (AddressOutOfBoundsException &exception)
   {
      throw OffsetOutOfBoundsException(exception.memory, this->offset+offset, data.size());
   }
}
