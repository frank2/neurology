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
#define NEXCEPT(expression, expected) try                \
   {\
      expression;\
      this->exceptionMacro(failures, typeid(*this).name(), NSTR(expression), expected, false, __LINE__, __FILE__); \
   }\
   catch (Neurology::Exception &e)\
   {\
      this->assertMessage(L"Exception raised: %s", e.explanation); \
      this->exceptionMacro(failures, typeid(*this).name(), NSTR(expression), expected, true, __LINE__, __FILE__); \
   }

namespace NeurologyTest
{
   class TestFailure;
   
   typedef std::vector<TestFailure> FailVector;
   
   class Test
   {
   public:
      Test(void);

      virtual void run(FailVector *failures);

      void launch(FailVector *failures);
      
      template <class ... Args> void assertMessage(LPWSTR format, Args ... args)
      {
         wprintf(format, args...);
         wprintf(L"\r\n");
      }

      void assertMacro(FailVector *failures, const char *test, const char *expression
                       ,bool result, std::uintptr_t line, const char *fileName);
      void exceptionMacro(FailVector *failures, const char *test, const char *expression
                          ,bool expected, bool result, std::uintptr_t line, const char *fileName);
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

   class TestRunner
   {
   public:
      static TestRunner Instance;
      
   protected:
      std::vector<Test *> tests;
      FailVector failures;
      
      TestRunner(void);

   public:
      int run(void);
      void pushTest(Test *test);
   };
}
   
