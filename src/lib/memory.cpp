#include <neurology/memory.hpp>

using namespace Neurology;

BadReferenceStateException::BadReferenceStateException
(void)
   : ReferenceException(EXCSTR(L"The memory reference is in a bad state."))
{
}

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
   this->setFlags(flags);
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

BYTE
Memory::Mode::getFlags
(void)
{
   return this->flags;
}

void
Memory::Mode::setFlags
(BYTE flags)
{
   this->flags = flags;
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

Memory::Reference::Reference
(void)
   : Neurology::Reference()
{
   this->state = false;
   this->pointerRef = NULL;
   this->sizeRef = NULL;
   this->modeRef = NULL;
}

Memory::Reference::Reference
(LPVOID pointer, SIZE_T size, Memory::Mode mode)
{
   this->allocate();
   this->state = Memory::Reference::STATE_POINTER;
   this->setPointer(pointer);
   this->setSize(size);
   this->setMode(mode);
}

Memory::Reference::Reference
(SIZE_T offset, SIZE_T size, Memory::Mode mode)
{
   this->allocate();
   this->state = Memory::Reference::STATE_OFFSET;
   this->setOffset(offset);
   this->setSize(size);
   this->setMode(mode);
}

Memory::Reference::Reference
(Memory::Reference &reference)
   : Neurology::Reference(reference)
{
   this->state = reference.state;
   this->pointerRef = reference.pointerRef;
   this->sizeRef = reference.sizeRef;
   this->modeRef = reference.modeRef;
}

bool
Memory::Reference::isNull
(void)
{
   return Neurology::Reference::isNull() ||
      (this->sizeRef == NULL || *this->sizeRef == 0) ||
      (this->state == Memory::Reference::STATE_POINTER &&
       (this->pointerRef == NULL || *this->pointerRef == NULL)) ||
      (this->state == Memory::Reference::STATE_OFFSET && this->offsetRef == NULL);
}

bool
Memory::Reference::state
(void)
{
   return this->state;
}

void
Memory::Reference::setState
(bool state)
{
   this->state = state;
}

LPVOID
Memory::Reference::pointer
(void)
{
   if (this->pointerRef == NULL)
      throw NullReferenceException();

   if (this->state != Memory::Reference::STATE_POINTER)
      throw BadReferenceStateException();

   return *this->pointerRef;
}

void
Memory::Reference::setPointer
(LPVOID pointer)
{
   if (this->pointerRef == NULL)
      throw NullReferenceException();

   if (this->state != Memory::Reference::STATE_POINTER)
      throw BadReferenceStateException();

   *this->pointerRef = pointer;
}

SIZE_T
Memory::Reference::offset
(void)
{
   if (this->offsetRef == NULL)
      throw NullReferenceException();

   if (this->state != Memory::Reference::STATE_POINTER)
      throw BadReferenceStateException();

   return *this->offsetRef;
}

void
Memory::Reference::setOffset
(SIZE_T offset)
{
   if (this->offsetRef == NULL)
      throw NullReferenceException();

   if (this->state != Memory::Reference::STATE_OFFSET)
      throw BadReferenceStateException();

   *this->offsetRef = offset;
}

SIZE_T
Memory::Reference::size
(void)
{
   if (this->sizeRef == NULL)
      throw NullReferenceException();

   return *this->sizeRef;
}

void
Memory::Reference::setSize
(SIZE_T size)
{
   if (this->sizeRef == NULL)
      throw NullReferenceException();

   *this->sizeRef = size;
}

Memory::Mode
Memory::Reference::mode
(void)
{
   if (this->modeRef == NULL)
      throw NullReferenceException();

   return *this->modeRef;
}

void
Memory::Reference::setMode
(Memory::Mode mode)
{
   *this->modeRef = mode;
}

void
Memory::Reference::allocate
(void)
{
   Neurology::Reference::allocate();
   
   if (this->state == Memory::Reference::STATE_POINTER)
      this->pointerRef = new LPVOID;
   else
      this->offsetRef = new SIZE_T;

   *this->pointerRef = 0;

   this->sizeRef = new SIZE_T;
   *this->sizeRef = 0;

   this->modeRef = new Mode();
}

void
Memory::Reference::release
(void)
{
   Neurology::Reference::release();
   
   delete this->pointerRef;
   delete this->sizeRef;
   delete this->modeRef;
}
   
Memory::Memory
(void)
{
   this->parent = NULL;
}

Memory::Memory
(LPVOID pointer, SIZE_T size, Mode mode)
{
   this->parent = NULL;
   this->reference = Memory::Reference(pointer, size, mode);
}

Memory::Memory
(Memory *parent, SIZE_T offset, SIZE_T size, Mode mode)
{
   this->parent = parent;

   if (parent == NULL)
      throw NullPointerException();

   this->reference = Memory::Reference(offset, size, mode);
}

Memory::Memory
(Memory &memory)
{
   this->parent = memory.parent;
   this->reference = memory.reference;
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
   this->reference.ref();

   if (this->parent != NULL)
      this->parent->ref();
}

void
Memory::deref
(void)
{
   this->reference.deref();

   if (this->parent != NULL)
      this->parent->deref();
}

DWORD
Memory::refs
(void)
{
   return this->reference.refs();
}

bool
Memory::isNull
(void)
{
   return this->reference.isNull();
}

bool
Memory::isValid
(void)
{
   bool result = this->reference.isNull();

   if (this->parent != NULL)
      result &= this->parent->isValid() && this->parent->inRange(this->parent->offset() + this->offset());
   else if (result)
      result &= this->reference.pointer() != NULL;

   return result && this->size() != 0;
}

void
Memory::invalidate
(void)
{
   this->setPointer(NULL);
   this->setSize(0);
}

Memory::Mode
Memory::mode
(void)
{
   return this->reference.mode();
}

bool
Memory::readable
(void)
{
   return (bool)this->mode().read;
}

bool
Memory::writable
(void)
{
   return (bool)this->mode().write;
}

bool
Memory::executable
(void)
{
   return (bool)this->mode().execute;
}

void
Memory::markReadable
(void)
{
   this->setMode(this->mode() | Memory::Mode::READ);
}

void
Memory::markWritable
(void)
{
   this->setMode(this->mode() | Memory::Mode::WRITE);
}

void
Memory::markExecutable
(void)
{
   this->setMode(this->mode() | Memory::Mode::EXECUTE);
}

void
Memory::unmarkReadable
(void)
{
   this->setMode(this->mode() & ~Memory::Mode::READ);
}

void
Memory::unmarkWritable
(void)
{
   this->setMode(this->mode() & ~Memory::Mode::WRITE);
}

void
Memory::unmarkExecutable
(void)
{
   this->setMode(this->mode() & ~Memory::Mode::EXECUTE);
}

SIZE_T
Memory::size
(void)
{
   return this->reference.size();
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
      return this->parent->pointer(this->reference.offset());

   return this->pointer(0);
}

LPVOID
Memory::pointer
(SIZE_T offset)
{
   if (this->parent != NULL)
      return this->parent->pointer(offset + this->reference.offset());

   if (offset > this->reference.size())
      throw OffsetOutOfBoundsException(this, offset, 0);

   return (LPVOID)((LPBYTE)this->reference.pointer() + offset);
}

SIZE_T
Memory::offset
(void)
{
   if (this->parent == NULL)
      return 0;
   else
      return this->reference.offset();
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
   if (this->size() == 0)
      return this->start();

   return this->pointer(this->size());
}

bool
Memory::inRange
(SIZE_T offset, SIZE_T size)
{
   return this->inRange(offset) && offset >= this->offset() && offset <= (this->offset() + this->size());
}

bool
Memory::inRange
(SIZE_T offset)
{
   if (this->parent != NULL)
      return this->inRange((LPVOID)((LPBYTE)this->parent->pointer() + this->offset() + offset));

   return this->inRange((LPVOID)((LPBYTE)this->reference.pointer() + offset));
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
   return address >= this->start() && address <= this->end();
}

void
Memory::setPointer
(LPVOID base)
{
   this->reference.setPointer(base);
}

void
Memory::setOffset
(SIZE_T offset)
{
   this->reference.setOffset(offset);
}

void
Memory::setSize
(SIZE_T size)
{
   this->reference.setSize(size);
}

void
Memory::setMode
(Memory::Mode mode)
{
   this->reference.setMode(mode);
}

Data
Memory::read
(void)
{
   return this->read(this->size());
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

   if (!this->readable())
      throw BadModeException(this, Memory::Mode::READ);
   
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
   if (!this->writable())
      throw BadModeException(this, Memory::Mode::WRITE);
   
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

void
Address::operator+=
(__int64 offset)
{
   this->shift(offset);
}

void
Address::operator-=
(__int64 offset)
{
   this->shift(-offset);
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
      throw OffsetOutOfBoundsException(this->memory, this->offset+offset, exception.size);
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
      throw OffsetOutOfBoundsException(this->memory, this->offset+offset, exception.size);
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

   while (parent != NULL && (newOffset > parent->size() || !parent->inRange(parent->pointer(newOffset))))
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
