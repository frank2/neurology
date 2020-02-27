#include "object.hpp"

using namespace Neurology;
using namespace NeurologyTest;

ObjectTest ObjectTest::Instance;

ObjectTest::ObjectTest
(void)
   : Test()
{
}

void
ObjectTest::run
(FailVector *failures)
{
   this->objectTest(failures);
   this->pointerTest(failures);
}

void
ObjectTest::objectTest
(FailVector *failures)
{
   /* test void objects */
   int externalInt;
   Object<void> voidObject;
   
   NASSERT((std::is_same<void *, Object<void>::BaseType>::value));
   NASSERT(std::is_void<Object<void>::UnpointedType>::value);
   NASSERT(!voidObject.isBuilt());
   NASSERT(!voidObject.isCached());
   NASSERT(!voidObject.willAutoflush());

   voidObject = &externalInt;

   NASSERT(voidObject.isBuilt());
   NASSERT(*voidObject == &externalInt);

   externalInt = 0xDEADBEEF;

   NASSERT(*reinterpret_cast<int *>(*voidObject) == externalInt);

   /* test int objects */
   Object<int> intObject;

   intObject = 0xDEADBEEF;

   NASSERT(*intObject == 0xDEADBEEF);

   *intObject += 5;

   NASSERT(*intObject == 0xDEADBEEF+5);

   *intObject -= 5;

   NASSERT(*intObject == 0xDEADBEEF);

   /* test struct objects */
   typedef struct
   {
      unsigned char c;
      unsigned short s;
      unsigned int i;
      std::uintptr_t uintptr;
   } StructTest;
   
   Object<StructTest> structObject = Object<StructTest>::New();
   
   structObject->c = 0x74;
   structObject->s = 0xBEEF;
   structObject->i = 0xDEFACED1;
   structObject->uintptr = 0xABAD1DEAB00BFACE;

   NASSERT((*structObject).c == 0x74);
   NASSERT((*structObject).s == 0xBEEF);
   NASSERT((*structObject).i == 0xDEFACED1);
   NASSERT((*structObject).uintptr == 0xABAD1DEAB00BFACE);
}

void
ObjectTest::pointerTest
(FailVector *failures)
{
   unsigned int stackInt;
   Pointer<unsigned int> intPtr;
   VirtualAllocator allocator;

   allocator.enumerate();
   intPtr.setAllocator(&allocator);
   
   intPtr = &stackInt;
   *intPtr = 0xDEADBEEF;

   NASSERT(**intPtr == 0xDEADBEEF);
   NASSERT(stackInt == 0xDEADBEEF);
}
