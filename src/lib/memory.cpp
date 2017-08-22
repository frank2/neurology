#include <neurology/memory.hpp>

using namespace Neurology;

MemoryException::MemoryException
(Memory *memory, const LPWSTR message)
   : NeurologyException(message)
{
   this->memory = memory;

   if (memory != NULL)
      memory->ref();
}

MemoryException::MemoryException
(MemoryException &exception)
   : NeurologyException(exception)
{
   if (this->memory != NULL)
      this->memory->deref();

   this->memory = exception.memory;

   if (this->memory != NULL)
      this->memory->ref();
}

MemoryException::~MemoryException
(void)
{
   if (this->memory != NULL)
      this->memory->deref();
}

BadModeException::BadModeException
(Memory *memory, Memory::Mode mode)
   : MemoryException(memory, EXCSTR(L"Invalid mode operation on memory."))
{
   this->mode = mode;
}

BadModeException::BadModeException
(BadModeException &exception)
   : MemoryException(exception)
{
   this->mode = exception.mode;
}

BadPointerException::BadPointerException
(Memory *memory, LPVOID pointer, SIZE_T size, Memory::Mode op)
   : MemoryException(memory, EXCSTR(L"Pointer is bad."))
{
   this->pointer = pointer;
   this->size = size;
   this->op = op;
}

BadPointerException::BadPointerException
(BadPointerException &exception)
   : MemoryException(exception)
{
   this->pointer = exception.pointer;
   this->size = exception.size;
   this->op = exception.op;
}

PointerOutOfBoundsException::PointerOutOfBoundsException
(Memory *memory, LPVOID pointer, SIZE_T size)
   : MemoryException(memory, EXCSTR(L"Pointer and size are out of bounds in region."))
{
   this->pointer = pointer;
   this->size = size;
}

PointerOutOfBoundsException::PointerOutOfBoundsException
(PointerOutOfBoundsException &exception)
   : MemoryException(exception)
{
   this->pointer = exception.pointer;
   this->size = exception.size;
}

OffsetOutOfBoundsException::OffsetOutOfBoundsException
(Memory *memory, SIZE_T offset, SIZE_T size)
   : MemoryException(memory, EXCSTR(L"Offset and size are out of bounds in region"))
{
   this->offset = offset;
   this->size = size;
}

OffsetOutOfBoundsException::OffsetOutOfBoundsException
(OffsetOutOfBoundsException &exception)
   : MemoryException(exception)
{
   this->offset = exception.offset;
   this->size = exception.size;
}

PointerWithParentException::PointerWithParentException
(Memory *memory)
   : MemoryException(memory, EXCSTR(L"Pointer created with parent present. Use Memory::setOffset instead."))
{
}

PointerWithParentException::PointerWithParentException
(PointerWithParentException &exception)
   : MemoryException(exception)
{
}

OffsetWithoutParentException::OffsetWithoutParentException
(Memory *memory)
   : MemoryException(memory, EXCSTR(L"Offset created without parent present. Use Memory::setPointer instead."))
{
}

OffsetWithoutParentException::OffsetWithoutParentException
(OffsetWithoutParentException &exception)
   : MemoryException(exception)
{
}

NegativeOffsetException::NegativeOffsetException
(Memory *memory, __int64 offset)
   : MemoryException(memory, EXCSTR(L"Negative offset exceeds all parent region boundaries."))
{
   this->offset = offset;
}

NegativeOffsetException::NegativeOffsetException
(NegativeOffsetException &exception)
   : MemoryException(exception)
{
   this->offset = exception.offset;
}

Memory::Mode::Mode
(void)
{
   this->flags = 0;
}

Memory::Mode::Mode
(bool read, bool write, bool execute)
{
   this->flags = Memory::Mode::Flags(read, write, execute);
}

Memory::Mode::Mode
(BYTE flags)
{
   this->flags = flags;
}

Memory::Mode::Mode
(Memory::Mode &mode)
{
   this->flags = mode.flags;
}

void
Memory::Mode::operator=
(BYTE flags)
{
   this->flags = flags;
}

Memory::Mode
Memory::Mode::operator&
(BYTE mask)
{
   return Memory::Mode(this->flags & mask);
}

Memory::Mode
Memory::Mode::operator&
(Mode mask)
{
   return Memory::Mode(this->flags & mask.flags);
}

Memory::Mode
Memory::Mode::operator|
(BYTE mask)
{
   return Memory::Mode(mask | this->flags);
}

Memory::Mode
Memory::Mode::operator|
(Mode mask)
{
   return Memory::Mode(mask.flags | this->flags);
}

Memory::Mode
Memory::Mode::operator^
(BYTE mask)
{
   return Memory::Mode(mask ^ this->flags);
}

Memory::Mode
Memory::Mode::operator^
(Mode mask)
{
   return Memory::Mode(mask.flags ^ this->flags);
}

void
Memory::Mode::operator&=
(BYTE mask)
{
   this->flags &= mask;
}

void
Memory::Mode::operator&=
(Mode mask)
{
   this->flags &= mask.flags;
}

void
Memory::Mode::operator|=
(BYTE mask)
{
   this->flags |= mask;
}

void
Memory::Mode::operator|=
(Mode mask)
{
   this->flags |= mask.flags;
}

void
Memory::Mode::operator^=
(BYTE mask)
{
   this->flags ^= mask;
}

void
Memory::Mode::operator^=
(Mode mask)
{
   this->flags ^= mask.flags;
}

BYTE
Memory::Mode::Flags
(bool read, bool write, bool execute)
{
   return (BYTE)(((BYTE)read << 2) | ((BYTE)write << 1) | ((BYTE)execute));
}

void
Memory::Mode::markReadable
(void)
{
   this->read = 1;
}

void
Memory::Mode::markWritable
(void)
{
   this->write = 1;
}

void
Memory::Mode::markExecutable
(void)
{
   this->execute = 1;
}

void
Memory::Mode::unmarkReadable
(void)
{
   this->read = 0;
}

void
Memory::Mode::unmarkWritable
(void)
{
   this->write = 0;
}

void
Memory::Mode::unmarkExecutable
(void)
{
   this->execute = 0;
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
(LPVOID pointer, SIZE_T size, Mode mode)
{
   this->parent = NULL;
   this->setPointer(pointer);
   this->setSize(size);
   this->setMode(mode);
   this->ref();
}

Memory::Memory
(Memory *parent, SIZE_T offset, SIZE_T size, Mode mode)
{
   this->parent = parent;

   if (parent == NULL)
      throw NullPointerException();

   this->parent->ref();
   this->setOffset(offset);
   this->setSize(size);
   this->setMode(mode);
   this->ref();
}

Memory::Memory
(Memory &memory)
{
   this->deref();

   this->parent = memory.parent;
   this->pointerRef = memory.pointerRef;
   this->sizeRef = memory.sizeRef;
   this->modeRef = memory.modeRef;
   this->refCount = memory.refCount;

   this->ref();
}

Memory::~Memory
(void)
{
   this->deref();
}

Memory *
Memory::getParent
(void)
{
   return this->parent;
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

      if (this->modeRef != NULL)
      {
         delete this->modeRef;
         this->modeRef = NULL;
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
Memory::isNull
(void)
{
   return (this->parent != NULL && (this->offsetRef == NULL || this->parent->isNull()) || (this->pointerRef == NULL || *this->pointerRef == NULL);
}

bool
Memory::isValid
(void)
{
   bool result = this->pointerRef != NULL;

   if (this->parent != NULL)
      result &= this->parent->isValid() && this->parent->inRange(*this->offsetRef);
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

Memory::Mode
Memory::mode
(void)
{
   if (this->modeRef == NULL)
      throw NullPointerException();
   
   return *this->modeRef;
}

bool
Memory::readable
(void)
{
   if (this->modeRef == NULL)
      throw NullPointerException();

   return (bool)this->modeRef->read;
}

bool
Memory::writable
(void)
{
   if (this->modeRef == NULL)
      throw NullPointerException();

   return (bool)this->modeRef->write;
}

bool
Memory::executable
(void)
{
   if (this->modeRef == NULL)
      throw NullPointerException();

   return (bool)this->modeRef->execute;
}

void
Memory::markReadable
(void)
{
   if (this->modeRef == NULL)
      throw NullPointerException();

   this->modeRef->markReadable();
}

void
Memory::markWritable
(void)
{
   if (this->modeRef == NULL)
      throw NullPointerException();

   this->modeRef->markWritable();
}

void
Memory::markExecutable
(void)
{
   if (this->modeRef == NULL)
      throw NullPointerException();

   this->modeRef->markExecutable();
}

void
Memory::unmarkReadable
(void)
{
   if (this->modeRef == NULL)
      throw NullPointerException();

   this->modeRef->unmarkReadable();
}

void
Memory::unmarkWritable
(void)
{
   if (this->modeRef == NULL)
      throw NullPointerException();

   this->modeRef->unmarkWritable();
}

void
Memory::unmarkExecutable
(void)
{
   if (this->modeRef == NULL)
      throw NullPointerException();

   this->modeRef->unmarkExecutable();
}

void
Memory::setModeFlags
(BYTE flags)
{
   if (this->modeRef == NULL)
      throw NullPointerException();

   *this->modeRef = flags;
}

SIZE_T
Memory::size
(void)
{
   if (this->size == NULL)
      return 0;
   
   return *this->size;
}

Memory::Mode
Memory::mode
(void)
{
   if (this->modeRef == NULL)
      throw NullPointerException();

   return *this->modeRef;
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
   Memory *addressParent = this;

   while (addressParent != NULL && !addressParent->inRange(addressParent->pointer(offset)))
   {
      offset += addressParent->offset();
      addressParent = addressParent->getParent();
   }

   if (addressParent == NULL)
      throw OffsetOutOfBoundsException(this, offset, 0);
   
   return Address(addressParent, offset);
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
(void)
{
   if (this->parent == NULL)
      return 0;
   else if (this->offsetRef == NULL)
      throw NullPointerException();
   else
      return *this->offsetRef;
}
   
SIZE_T
Memory::offset
(LPVOID address)
{
   if (!this->inRange(address))
      throw PointerOutOfBoundsException(this, address, 0);

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
(SIZE_T offset, SIZE_T size)
{
   return this->inRange(offset) && this->sizeRef != NULL && offset >= *this->offsetRef && offset <= (*this->offsetRef + *this->sizeRef);
}

bool
Memory::inRange
(SIZE_T offset)
{
   if (this->pointerRef == NULL || *this->pointerRef == NULL)
      throw NullPointerException();

   if (this->parent != NULL)
      return this->inRange((LPVOID)((LPBYTE)this->parent->pointer() + *this->offsetRef + offset));

   return this->inRange((LPVOID)((LPBYTE)*this->pointerRef + offset));
}
   

bool
Memory::inRange
(LPVOID address, SIZE_T size)
{
   LPBYTE calcAddress = (LPBYTE)address+size;

   return this->inRange(address) && calcAddress >= this->start() && calcAddress <= this->end();
}

bool
Memory::inRange
(LPVOID address)
{
   return this->pointerRef != NULL && address >= this->start() && address <= this->end();
}

void
Memory::setPointer
(LPVOID base)
{
   if (this->parent != NULL)
   {
      if (this->offsetRef == NULL)
         this->offsetRef = new SIZE_T;

      *this->offsetRef = this->parent->offset(base);
   }
   else
   {
      if (this->pointerRef == NULL)
         this->pointerRef = new LPVOID;

      *this->pointerRef = base;
   }
}

void
Memory::setOffset
(SIZE_T offset)
{
   if (this->parent == NULL)
      throw OffsetWithoutParentException(this);
   else
   {
      if (this->offsetRef == NULL)
         this->offsetRef = new SIZE_T;

      *this->offsetRef = offset;
   }
}

void
Memory::setSize
(SIZE_T size)
{
   if (this->sizeRef == NULL)
      this->sizeRef = new SIZE_T;

   *this->sizeRef = size;
}

void
Memory::setMode
(Mode mode)
{
   if (this->modeRef == NULL)
      this->modeRef = new Mode;

   *this->modeRef = mode;
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
   catch (PointerOutOfBoundsException &exception)
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

   if (!this->readable())

   if (!this->inRange(address, size))
      throw PointerOutOfBoundsException(this, address, size);

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
      throw BadPointerException(this, address, size, Memory::Mode::READ);
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
      this->write(this->pointer(offset), data);
   }
   catch (PointerOutOfBoundsException &exception)
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
      throw PointerOutOfBoundsException(this, address, data.size());

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
      throw BadPointerException(this, address, data.size(), Memory::Mode::WRITE);
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

   if (!memory->inRange(memory->pointer(offset)))
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

Address
Address::operator+
(__int64 offset)
{
   Address newAddress = *this;

   newAddress.shift(offset);
   
   return newAddress;
}

Address
Address::operator-
(__int64 offset)
{
   Address newAddress = *this;

   newAddress.shift(-offset);
   
   return newAddress;
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

SIZE_T
Address::getOffset
(void)
{
   return this->offset;
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
   catch (PointerOutOfBoundsException &exception)
   {
      throw OffsetOutOfBoundsException(exception.memory, this->offset+offset, exception.size);
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
   catch (PointerOutOfBoundsException &exception)
   {
      throw OffsetOutOfBoundsException(exception.memory, this->offset+offset, exception.size);
   }
}

void
Address::shift
(__int64 offset)
{
   __int64 newOffset = (__int64)this->offset + offset;
   Memory *parent = this->memory;

   if (parent == NULL)
      throw NullPointerException();

   while (parent != NULL && newOffset < 0)
   {
      newOffset += parent->offset();
      parent = parent->getParent();
   }

   if (parent == NULL)
      throw NegativeOffsetException(this->memory, offset);

   while (parent != NULL && (newOffset > parent->size() || !parent->inRange(parent->pointer(newOffset))
   {
      newOffset += parent->offset();
      parent = parent->getParent();
   }

   if (parent == NULL)
      throw OffsetOutOfBoundsException(this->memory, (SIZE_T)offset, 0);

   this->memory->deref();
   this->memory = parent;
   this->memory->ref();
   this->offset = (SIZE_T)newOffset;
}
