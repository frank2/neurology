#include "test.hpp"

using namespace NeurologyTest;

TestRunner TestRunner::Instance;

Test::Test
(void)
{
   TestRunner::Instance.pushTest(this);
}

void
Test::run
(FailVector *failures)
{
   /* null test, return no failures. */
   return;
}

void
Test::launch
(FailVector *failures)
{
   std::uintptr_t failureCount = failures->size();
   
   this->assertMessage(L"[*] Running %S.", typeid(*this).name());

   this->run(failures);

   this->assertMessage(L"[*] %I64d new errors.", failures->size() - failureCount);
}

void
Test::assertMacro
(FailVector *failures, const char *test, const char *expression, bool result, std::uintptr_t line, const char *fileName)

{
   this->assertMessage(L"[%s] [%S] assertion: %S... %s"
                       ,(result) ? L"+" : L"!"
                       ,test
                       ,expression
                       ,(result) ? L"success" : L"failure");

   if (!result)
      failures->push_back(TestFailure(test, expression, line, fileName));
}

void
Test::exceptionMacro
(FailVector *failures, const char *test, const char *expression, bool expected, bool result, std::uintptr_t line, const char *fileName)

{
   bool conclusion = (expected == result);
   this->assertMessage(L"[%s] [%S] exception: %S... %s"
                       ,(conclusion) ? L"+" : L"!"
                       ,test
                       ,expression
                       ,(conclusion) ? L"success" : L"failure");

   if (!conclusion)
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

TestRunner::TestRunner
(void)
{
}

int
TestRunner::run
(void)
{
   int result;

   for (std::vector<Test *>::iterator iter=this->tests.begin();
        iter!=this->tests.end();
        ++iter)
      (*iter)->launch(&this->failures);

   if (this->failures.size() == 0)
   {
      wprintf(L"\r\n[$] All tests passed!\r\n\r\n");
      result = 0;
   }
   else
   {
      std::map<const char *, FailVector > organizedFails;
      wprintf(L"[#] Errors occurred.\r\n");
      wprintf(L"[*] Organizing failures...");

      for (FailVector::iterator iter=this->failures.begin();
           iter!=this->failures.end();
           ++iter)
      {
         organizedFails[iter->test].push_back(*iter);
      }

      wprintf(L"done.\r\n\r\n==========\r\n\r\n");
      wprintf(L"[!] %I64d total failures.\r\n\r\n", failures.size());

      for (std::map<const char *, FailVector>::iterator iter=organizedFails.begin();
           iter!=organizedFails.end();
           ++iter)
      {
         wprintf(L"[!] %I64d failures in %S...\r\n", iter->second.size(), iter->first);

         for (FailVector::iterator failIter=iter->second.begin();
              failIter!=iter->second.end();
              ++failIter)
         {
            wprintf(L"... %S (line %I64d, file %S)\r\n", failIter->expression, failIter->line, failIter->fileName);
         }

         wprintf(L"\r\n");
      }
      
      result = 1;
   }

   system("pause");
   return result;
}

void
TestRunner::pushTest
(Test *test)
{
   this->tests.push_back(test);
}
