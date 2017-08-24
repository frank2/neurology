#include <neurology/memory.hpp>

using namespace Neurology;

BadReferenceStateException::BadReferenceStateException
(const Memory::Reference &reference)
   : ReferenceException(reference, EXCSTR(L"The memory reference is in a bad state."))
{
}

MemoryException::MemoryException
(Memory &memory, const LPWSTR message)
   : NeurologyException(message)
{
   this->memory = memory;
}

MemoryException::MemoryException
(MemoryException &exception)
   : NeurologyException(exception)
{
   this->memory = exception.memory;
}

MemoryException::~MemoryException
(void)
{
}

BadModeException::BadModeException
(Memory &memory, Memory::Mode mode)
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
(Memory &memory, LPVOID pointer, SIZE_T size, Memory::Mode op)
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
(Memory &memory, LPVOID pointer, SIZE_T size)
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
(Memory &memory, SIZE_T offset, SIZE_T size)
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
(Memory &memory)
   : MemoryException(memory, EXCSTR(L"Pointer created with parent present. Use Memory::setOffset instead."))
{
}

PointerWithParentException::PointerWithParentException
(PointerWithParentException &exception)
   : MemoryException(exception)
{
}

OffsetWithoutParentException::OffsetWithoutParentException
(Memory &memory)
   : MemoryException(memory, EXCSTR(L"Offset created without parent present. Use Memory::setPointer instead."))
{
}

OffsetWithoutParentException::OffsetWithoutParentException
(OffsetWithoutParentException &exception)
   : MemoryException(exception)
{
}

NegativeOffsetException::NegativeOffsetException
(Memory &memory, __int64 offset)
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
   this->setFlags(0);
}

Memory::Mode::Mode
(bool read, bool write, bool execute)
{
   this->setFlags(Memory::Mode::MakeFlags(read, write, execute));
}

Memory::Mode::Mode
(BYTE flags)
{
   this->setFlags(flags);
}

Memory::Mode::Mode
(Memory::Mode &mode)
{
   *this = mode;
}

Memory::Mode::Mode
(const Memory::Mode &mode)
{
   *this = mode;
}

Memory::Mode::operator
int(void) const
{
   return this->modeFlags;
}

void
Memory::Mode::operator=
(const Memory::Mode &mode)
{
   this->setFlags(mode.modeFlags);
}

void
Memory::Mode::operator=
(BYTE flags)
{
   this->setFlags(flags);
}

Memory::Mode
Memory::Mode::operator&
(BYTE mask) const
{
   return Memory::Mode(this->modeFlags & mask);
}

Memory::Mode
Memory::Mode::operator&
(const Mode mask) const
{
   return Memory::Mode(this->modeFlags & mask.flags);
}

Memory::Mode
Memory::Mode::operator|
(BYTE mask) const
{
   return Memory::Mode(mask | this->flags);
}

Memory::Mode
Memory::Mode::operator|
(const Mode mask) const
{
   return Memory::Mode(mask.flags | this->flags);
}

Memory::Mode
Memory::Mode::operator^
(BYTE mask) const
{
   return Memory::Mode(mask ^ this->flags);
}

Memory::Mode
Memory::Mode::operator^
(const Mode mask) const
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
(const Mode mask)
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
(const Mode mask)
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
(const Mode mask)
{
   this->flags ^= mask.flags;
}

BYTE
Memory::Mode::MakeFlags
(bool read, bool write, bool execute)
{
   return (BYTE)(((BYTE)read << 2) | ((BYTE)write << 1) | ((BYTE)execute));
}

BYTE
Memory::Mode::flags
(void) const
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

void
Memory::Reference::operator=
(Memory::Reference &reference)
{
   Neurology::Reference::operator=(reference);
      
   this->state = reference.state;
   this->pointerRef = reference.pointerRef;
   this->sizeRef = reference.sizeRef;
   this->modeRef = reference.modeRef;
}

void
Memory::Reference::operator=
(const Memory::Reference &reference)
{
   Neurology::Reference::operator=(reference);

   this->state = reference.state;

   if (!reference.isNull())
   {
      *this->pointerRef = *reference.pointerRef;
      *this->sizeRef = *reference.sizeRef;
      *this->modeRef = *reference.modeRef;
   }
   else
   {
      *this->pointerRef = NULL;
      *this->sizeRef = NULL;
      *this->modeRef = NULL;
   }
}

bool
Memory::Reference::isNull
(void) const
{
   return Neurology::Reference::isNull() || this->sizeRef == NULL || this->pointerRef == NULL || this->modeRef == NULL;
}

bool
Memory::Reference::isNullAddress
(void) const
{
   this->throwIfNull();
   
   return *this->sizeRef == 0 || (this->state == Memory::Reference::STATE_POINTER && *this->pointerRef == NULL);
}

bool
Memory::Reference::state
(void) const
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
(void) const
{
   this->throwIfNull();
   
   if (this->state != Memory::Reference::STATE_POINTER)
      throw BadReferenceStateException(*this);

   return *this->pointerRef;
}

void
Memory::Reference::setPointer
(LPVOID pointer)
{
   this->throwIfNull();
   this->throwIfConst();

   if (this->state != Memory::Reference::STATE_POINTER)
      throw BadReferenceStateException(*this);

   *this->pointerRef = pointer;
}

SIZE_T
Memory::Reference::offset
(void) const
{
   this->throwIfNull();

   if (this->state != Memory::Reference::STATE_POINTER)
      throw BadReferenceStateException(*this);

   return *this->offsetRef;
}

void
Memory::Reference::setOffset
(SIZE_T offset)
{
   this->throwIfNull();
   this->throwIfConst();
   
   if (this->state != Memory::Reference::STATE_OFFSET)
      throw BadReferenceStateException(*this);

   *this->offsetRef = offset;
}

SIZE_T
Memory::Reference::size
(void) const
{
   this->throwIfNull();
   
   return *this->sizeRef;
}

void
Memory::Reference::setSize
(SIZE_T size)
{
   this->throwIfNull();
   this->throwIfConst();
   
   *this->sizeRef = size;
}

Memory::Mode
Memory::Reference::mode
(void) const
{
   this->throwIfNull();

   return *this->modeRef;
}

void
Memory::Reference::setMode
(Memory::Mode mode)
{
   this->throwIfNull();
   this->throwIfConst();
   
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
   if (parent == NULL)
      throw NullPointerException();
   
   this->parent = parent;
   this->reference = Memory::Reference(offset, size, mode);

   if (parent != NULL)
      parent->ref();
}

Memory::Memory
(Memory &memory)
{
   *this = memory;
}

void
Memory::operator=
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

const Memory *
Memory::getParent
(void) const
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
(void) const
{
   return this->reference.refs();
}

bool
Memory::isNull
(void) const
{
   return this->reference.isNull();
}

bool
Memory::isValid
(void) const
{
   bool result = !this->reference.isNull();

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
(void) const
{
   return this->reference.mode();
}

bool
Memory::readable
(void) const
{
   return (bool)this->mode().read;
}

bool
Memory::writable
(void) const
{
   return (bool)this->mode().write;
}

bool
Memory::executable
(void) const
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
(void) const
{
   return this->reference.size();
}

Address
Memory::address
(void)
{
   return this->address(0);
}

const Address
Memory::address
(void) const
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
      addressParent = &addressParent->getParent();

      if (addressParent->isNull())
         addressParent = NULL;
   }

   if (addressParent == NULL)
      throw OffsetOutOfBoundsException(*this, offset, 0);
   
   return Address(addressParent, offset);
}

const Address
Memory::address
(SIZE_T offset) const
{
   Memory *addressParent = this;

   while (addressParent != NULL && !addressParent->inRange(addressParent->pointer(offset)))
   {
      offset += addressParent->offset();
      addressParent = &addressParent->getParent();

      if (addressParent->isNull())
         addressParent = NULL;
   }

   if (addressParent == NULL)
      throw OffsetOutOfBoundsException(*this, offset, 0);
   
   return Address(addressParent, offset);
}

LPVOID
Memory::pointer
(void)
{
   if (this->parent != NULL)
      return this->parent->pointer(this->offset());

   return this->pointer(0);
}

const LPVOID
Memory::pointer
(void) const
{
   if (this->parent != NULL)
      return this->parent->pointer(this->offset());

   return this->pointer(0);
}

LPVOID
Memory::pointer
(SIZE_T offset)
{
   if (this->parent != NULL)
      return this->parent->pointer(offset + this->offset());

   if (offset > this->size())
      throw OffsetOutOfBoundsException(*this, offset, 0);

   return (LPVOID)((LPBYTE)this->reference.pointer() + offset);
}

const LPVOID
Memory::pointer
(SIZE_T offset) const
{
   if (this->parent != NULL)
      return this->parent->pointer(offset + this->offset());

   if (offset > this->size())
      throw OffsetOutOfBoundsException(*this, offset, 0);

   return (LPVOID)((LPBYTE)this->reference.pointer() + offset);
}

SIZE_T
Memory::offset
(void) const
{
   if (this->parent == NULL)
      return 0;
   else
      return this->reference.offset();
}
   
SIZE_T
Memory::offset
(LPVOID address) const
{
   if (!this->inRange(address))
      throw PointerOutOfBoundsException(*this, address, 0);

   return (SIZE_T)address - (SIZE_T)this->pointer();
}
   
LPVOID
Memory::start
(void)
{
   return this->pointer();
}

const LPVOID
Memory::start
(void) const
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

const LPVOID
Memory::end
(void) const
{
   if (this->size() == 0)
      return this->start();

   return this->pointer(this->size());
}

bool
Memory::inRange
(SIZE_T offset, SIZE_T size) const
{
   return this->inRange(offset) && offset >= this->offset() && offset <= (this->offset() + this->size());
}

bool
Memory::inRange
(SIZE_T offset) const
{
   try
   {
      if (!this->parent.isNull())
         return this->inRange((LPVOID)((LPBYTE)this->parent.pointer() + this->offset() + offset));
   }
   catch (PointerOutOfBoundsException &exception)
   {
      return false;
   }
   catch (OffsetOutOfBoundsException &exception)
   {
      return false;
   }

   return this->inRange((LPVOID)((LPBYTE)this->pointer() + offset));
}
   

bool
Memory::inRange
(LPVOID address, SIZE_T size) const
{
   try
   {
      LPBYTE calcAddress = (LPBYTE)address+size;
      return this->inRange(address) && calcAddress >= this->start() && calcAddress <= this->end();
   }
   catch (PointerOutOfBoundsException &exception)
   {
      return false;
   }
   catch (OffsetOutOfBoundsException &exception)
   {
      return false;
   }
}

bool
Memory::inRange
(LPVOID address) const
{
   try
   {
      return address >= this->start() && address <= this->end();
   }
   catch (PointerOutOfBoundsException &exception)
   {
      return false;
   }
   catch (OffsetOutOfBoundsException &exception)
   {
      return false;
   }
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
(void) const
{
   return this->read(this->size());
}

Data
Memory::read
(SIZE_T size) const
{
   return this->read((SIZE_T)0, size);
}

Data
Memory::read
(SIZE_T offset, SIZE_T size) const
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
(LPVOID address, SIZE_T size) const
{
   Data result;

   if (!this->readable())
      throw BadModeException(this, Memory::Mode::READ);
   
   if (!this->inRange(address, size))
      throw PointerOutOfBoundsException(this, address, size);

   if (size == 0)
      return result;

   if (!this->parent.isNull())
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
(Memory region)
{
   this->write(region.read());
}

void
Memory::write
(Data data)
{
   this->write((SIZE_T)0, data);
}

void
Memory::write
(SIZE_T offset, Memory region)
{
   this->write(offset, region.read());
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
(LPVOID address, Memory region)
{
   this->write(address, region.read());
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

   if (this->parent.isNull())
      return this->parent.write(address, data);
   
   __try
   {
      CopyMemory(address, data.data(), data.size());
   }
   __except (EXCEPTION_EXECUTE_HANDLER)
   {
      throw BadPointerException(this, address, data.size(), Memory::Mode::WRITE);
   }
}

Memory
Memory::slice
(SIZE_T start, SIZE_T end)
{
   LPVOID startPtr, endPtr;

   startPtr = this->pointer(start);
   endPtr = this->pointer(end);
   
   try
   {
      return this->slice(startPtr, endPtr);
   }
   catch (AddressOutOfBoundsException &exception)
   {
      if (exception.pointer == startPtr)
         throw OffsetOutOfBoundsException(exception.memory, start, exception.size);
      else if (exception.pointer == endPtr)
         throw OffsetOutOfBoundsException(exception.memory, end, exception.size);
      else
         throw exception;
   }
}

const Memory
Memory::slice
(SIZE_T start, SIZE_T end) const
{
   const LPVOID startPtr, endPtr;

   startPtr = this->pointer(start);
   endPtr = this->pointer(end);
   
   try
   {
      return this->slice(startPtr, endPtr);
   }
   catch (AddressOutOfBoundsException &exception)
   {
      if (exception.pointer == startPtr)
         throw OffsetOutOfBoundsException(exception.memory, start, exception.size);
      else if (exception.pointer == endPtr)
         throw OffsetOutOfBoundsException(exception.memory, end, exception.size);
      else
         throw exception;
   }
}

Memory
Memory::slice
(LPVOID start, LPVOID end)
{
   SIZE_T size = (SIZE_T)end-(SIZE_T)start;
   
   if (!this->inRange(start))
      throw AddressOutOfBoundsException(*this, start, size);

   if (!this->inRange(end))
      throw AddressOutOfBoundsException(*this, end, size);

   return Memory(this, this->offset(start), size, this->mode());
}

const Memory
Memory::slice
(LPVOID start, LPVOID end) const
{
   SIZE_T size = (SIZE_T)end-(SIZE_T)start;
   
   if (!this->inRange(start))
      throw AddressOutOfBoundsException(*this, start, size);

   if (!this->inRange(end))
      throw AddressOutOfBoundsException(*this, end, size);

   return const Memory(this, this->offset(start), size, this->mode());
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
   *this = address;
}

Address::~Address
(void)
{
   if (this->memory != NULL)
      this->memory->deref();

   this->memory = NULL;
   this->offset = 0;
}

void
Address::operator=
(Address &address)
{
   if (this->memory != NULL)
      this->memory->deref();
   
   this->memory = address.memory;
   this->offset = address.offset;

   if (this->memory != NULL)
      this->memory->ref();
}

bool
Address::operator==
(Address &address) const
{
   if (this->isNull() || address.isNull())
      return this->isNull() && address.isNull();
   
   return address.pointer() == this->pointer();
}

bool
Address::operator==
(LPVOID pointer) const
{
   if (this->isNull())
      throw NullPointerException();
   
   return pointer == this->pointer();
}

Address
Address::operator+
(__int64 offset) const
{
   Address newAddress = *this;

   newAddress.shift(offset);
   
   return newAddress;
}

Address
Address::operator-
(__int64 offset) const
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

bool
Address::isNull
(void) const
{
   return this->memory == NULL || this->memory->isNull();
}

LPVOID
Address::pointer
(void)
{
   return this->pointer(0);
}

const LPVOID
Address::pointer
(void) const
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

const LPVOID
Address::pointer
(SIZE_T offset) const
{
   if (this->memory == NULL)
      throw NullPointerException();

   return this->memory->pointer(this->offset + offset);
}

SIZE_T
Address::getOffset
(void) const
{
   return this->offset;
}

Data
Address::read
(SIZE_T size) const
{
   return this->read(0, size);
}

Data
Address::read
(SIZE_T offset, SIZE_T size) const
{
   if (this->memory == NULL)
      throw NullPointerException();

   try
   {
      return this->memory->read(this->pointer(this->offset+offset), size);
   }
   catch (PointerOutOfBoundsException &exception)
   {
      throw OffsetOutOfBoundsException(*this->memory, this->offset+offset, exception.size);
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
      throw OffsetOutOfBoundsException(*this->memory, this->offset+offset, exception.size);
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
      throw NegativeOffsetException(*this->memory, offset);

   while (parent != NULL && !parent->inRange(newOffset))
   {
      newOffset += parent->offset();
      parent = parent->getParent();
   }

   if (parent == NULL)
      throw OffsetOutOfBoundsException(*this->memory, (SIZE_T)offset, 0);

   this->memory->deref();
   this->memory = parent;
   this->memory->ref();
   this->offset = (SIZE_T)newOffset;
}
