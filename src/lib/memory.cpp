#include <neurology/memory.hpp>

using namespace Neurology;

NullPointerException::NullPointerException
(void)
   : NeurologyException("buffer pointer is null")
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
   this->buffer = NULL;
   this->size = NULL;
   this->refCount = NULL;
}

Memory::Memory
(LPVOID buffer, SIZE_T size)
{
   this->setBuffer(buffer);
   this->setSize(size);
}

Memory::Memory
(Memory &memory)
{
   this->buffer = memory.buffer;
   this->size = memory.size;
   this->refCount = memory.refCount;

   ++*this->refCount;
}

Memory::~Memory
(void)
{
   this->free();
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
   return this->address(0);
}

Address
Memory::address
(SIZE_T offset)
{
   return Address(this, offset);
}

LPVOID
Memory::bufferAddress
(SIZE_T offset)
{
   if (this->buffer == NULL || *this->buffer == NULL)
      return NULL;

   return (LPVOID)(((LPBYTE)*this->buffer)+offset);
}

SIZE_T
Memory::bufferOffset
(LPVOID address)
{
   if (!this->inRange(address))
      throw AddressOutOfBoundsException(this, address, 0);

   return (SIZE_T)address - (SIZE_T)*this->buffer;
}
   
LPVOID
Memory::start
(void)
{
   return this->bufferAddress(0);
}

LPVOID
Memory::end
(void)
{
   if (this->size == NULL || *this->size == 0)
      return this->start();

   return this->bufferAddress(*this->size);
}

bool
Memory::inRange
(LPVOID address)
{
   return this->buffer != NULL && *this->buffer != NULL && address >= this->start() && address < this->end();
}

bool
Memory::inRange
(LPVOID address, SIZE_T size)
{
   LPBYTE calcAddress = (LPBYTE)address+size;
   
   return this->inRange(address) && calcAddress >= this->start() && calcAddress <= this->end();
}

void
Memory::setBuffer
(LPVOID base)
{
   if (this->buffer == NULL)
      this->buffer = new LPVOID;

   if (this->refCount == NULL)
   {
      this->refCount = new DWORD;
      *this->refCount = 1;
   }

   if (this->size == NULL)
   {
      this->size = new SIZE_T;
      *this->size = 0;
   }

   *this->buffer = base;
}

void
Memory::setSize
(SIZE_T size)
{
   if (this->buffer == NULL || *this->buffer == NULL)
      throw NullPointerException();

   if (this->size == NULL)
      this->size = new SIZE_T;

   *this->size = size;
}

void
Memory::free
(void)
{
   if (this->buffer == NULL || *this->buffer == NULL)
      throw NullPointerException();

   --*this->refCount;

   if (*this->refCount <= 0)
      this->kill();
}

Data
Memory::read
(void)
{
   if (this->size == NULL)
      throw NullPointerException();
   
   return this->read(*this->size);
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
      return this->read(this->bufferAddress(offset), size);
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
   
   if (this->buffer == NULL || *this->buffer == NULL)
      throw NullPointerException();

   if (!this->inRange(address, size))
      throw AddressOutOfBoundsException(this, address, size);

   if (size == 0)
      return result;

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
      this->write(this->bufferAddress(offset), data);
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
   if (this->buffer == NULL || *this->buffer == NULL)
      throw NullPointerException();

   if (!this->inRange(address, data.size()))
      throw AddressOutOfBoundsException(this, address, data.size());

   if (data.size() == 0)
      return;

   __try
   {
      CopyMemory(address, data.data(), data.size());
   }
   __except (EXCEPTION_EXECUTE_HANDLER)
   {
      throw BadPointerException(address, data.size());
   }
}

void
Memory::kill
(void)
{
   if (this->buffer != NULL)
   {
      *this->buffer = NULL;
      delete this->buffer;
   }

   if (this->size != NULL)
   {
      *this->size = 0;
      delete this->size;
   }

   if (this->refCount != NULL)
   {
      *this->refCount = 0;
      delete this->refCount;
   }

   this->buffer = NULL;
   this->size = NULL;
   this->refCount = NULL;
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
}

Address::Address
(Memory *memory, SIZE_T offset)
{
   if (memory == NULL)
      throw NullPointerException();

   if (!memory->inRange(memory->bufferAddress(offset)))
      throw OffsetOutOfBoundsException(memory, offset, 0);

   this->memory = memory;
   this->offset = offset;
}

Address::Address
(Address &address)
{
   this->memory = address.memory;
   this->offset = address.offset;
}

LPVOID
Address::address
(void)
{
   return this->address(0);
}

LPVOID
Address::address
(SIZE_T offset)
{
   LPVOID addr;
   
   if (this->memory == NULL)
      throw NullPointerException();

   addr = this->memory->bufferAddress(this->offset+offset);
   
   if (!this->memory->inRange(addr))
      throw OffsetOutOfBoundsException(this->memory, this->offset+offset, 0);
   
   return addr;
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
      return this->memory->read(this->address(offset), size);
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
      this->memory->write(this->address(offset), data);
   }
   catch (AddressOutOfBoundsException &exception)
   {
      throw OffsetOutOfBoundsException(exception.memory, this->offset+offset, data.size());
   }
}
