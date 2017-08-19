#pragma once

#include <windows.h>

#include <neurology/exception.hpp>
#include <neurology/memory.hpp>

namespace Neurology
{
   class InvalidDOSImageException : public NeurologyException
   {
   public:
      Memory *offensiveMemory;

      InvalidDOSImageException(Memory *offensiveMemory);
      InvalidDOSImageException(InvalidDOSImageException &exception);
   };

   class NullHeaderException : public NeurologyException
   {
   public:
      NullHeaderException();
      NullHeaderException(NullHeaderException &exception);
   };
   
   class DOSHeader
   {
   protected:
      Memory *memory;

   public:
      DOSHeader(Memory *memory);
      DOSHeader(DOSHeader &header);

      WORD &e_magic(void);
      WORD &e_cblp(void);
      WORD &e_cp(void);
      WORD &e_crlc(void);
      WORD &e_cparhdr(void);
      WORD &e_minalloc(void);
      WORD &e_maxalloc(void);
      WORD &e_ss(void);
      WORD &e_csum(void);
      WORD &e_ip(void);
      WORD &e_cs(void);
      WORD &e_lfarlc(void);
      WORD &e_ovno(void);
      WORD (&e_res(void))[4];
      WORD &e_oemid(void);
      WORD &e_oeminfo(void);
      WORD (&e_res2(void))[10];
      LONG &e_lfanew(void);

      PIMAGE_DOS_HEADER getHeader(void);
      bool isDOSImage(void) const;
   };
}
