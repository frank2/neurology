#pragma once

#include <cstdint>
#include <stdio.h>
#include <typeinfo>
#include <vector>

#include <neurology.hpp>
#pragma comment(lib, "neurology")
   
#define NEXPAND(exp) #exp
#define NSTR(exp) NEXPAND(exp)
#define NASSERT(expression) this->assertMacro(failures, typeid(*this).name(), NSTR(expression), (expression), __LINE__, __FILE__)

namespace NeurologyTest
{
   class TestFailure;
   
   class Test
   {
   public:
      Test(void);

      virtual void run(std::vector<TestFailure> *failures);

      void launch(std::vector<TestFailure> *failures);
      
      template <class ... Args> void assertMessage(LPWSTR format, Args ... args)
      {
         wprintf(format, args...);
         wprintf(L"\r\n");
      }

      void assertMacro(std::vector<TestFailure> *failures, const char *test, const char *expression
                       ,bool result, std::uintptr_t line, const char *fileName);
   };

   class TestFailure
   {
   public:
      const char *test;
      const char *expression;
      std::uintptr_t line;
      const char *fileName;

      TestFailure(void);
      TestFailure(const char *test, const char *expression, std::uintptr_t line, const char *fileName);
      TestFailure(const TestFailure &failure);
   };
}
   
