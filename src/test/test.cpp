#include "test.hpp"

using namespace NeurologyTest;

Test::Test
(void)
{
}

void
Test::run
(std::vector<TestFailure> *failures)
{
   /* null test, return no failures. */
   return;
}

void
Test::launch
(std::vector<TestFailure> *failures)
{
   std::uintptr_t failureCount = failures->size();
   
   this->assertMessage(L"[*] Running %S.", typeid(*this).name());

   this->run(failures);

   this->assertMessage(L"[*] %I64d new errors.", failures->size() - failureCount);
}

void
Test::assertMacro
(std::vector<TestFailure> *failures, const char *test, const char *expression, bool result, std::uintptr_t line, const char *fileName)

{
   this->assertMessage(L"[%s] [%S] assertion: %S... %s"
                       ,(result) ? L"+" : L"!"
                       ,test
                       ,expression
                       ,(result) ? L"success" : L"failure");

   if (!result)
      failures->push_back(TestFailure(test, expression, line, fileName));
}

TestFailure::TestFailure
(void)
   : test(NULL)
   , expression(NULL)
   , line(0)
   , fileName(NULL)
{
}

TestFailure::TestFailure
(const char *test, const char *expression, std::uintptr_t line, const char *fileName)
   : test(test)
   , expression(expression)
   , line(line)
   , fileName(fileName)
{
}

TestFailure::TestFailure
(const TestFailure &failure)
   : test(failure.test)
   , expression(failure.expression)
   , line(failure.line)
   , fileName(failure.fileName)
{
}
