#include "main.hpp"

using namespace NeurologyTest;

int
main
(int argc, char *argv[])
{
   std::vector<TestFailure> failures;
   std::vector<Test *> tests = {new AddressTest};
   int result;

   for (std::vector<Test *>::iterator iter=tests.begin();
        iter!=tests.end();
        ++iter)
      (*iter)->launch(&failures);

   if (failures.size() == 0)
   {
      wprintf(L"\r\n[$] All tests passed!\r\n\r\n");
      result = 0;
   }
   else
   {
      std::map<const char *, std::vector<TestFailure> > organizedFails;
      wprintf(L"[#] Errors occurred.\r\n");
      wprintf(L"[*] Organizing failures...");

      for (std::vector<TestFailure>::iterator iter=failures.begin();
           iter!=failures.end();
           ++iter)
      {
         organizedFails[iter->test].push_back(*iter);
      }

      wprintf(L"done.\r\n\r\n==========\r\n\r\n");
      wprintf(L"[!] %I64d total failures.\r\n\r\n", failures.size());

      for (std::map<const char *, std::vector<TestFailure> >::iterator iter=organizedFails.begin();
           iter!=organizedFails.end();
           ++iter)
      {
         wprintf(L"[!] %I64d failures in %S...\r\n", iter->second.size(), iter->first);

         for (std::vector<TestFailure>::iterator failIter=iter->second.begin();
              failIter!=iter->second.end();
              ++failIter)
         {
            wprintf(L"... %S (line %I64d, file %S)\r\n", failIter->expression, failIter->line, failIter->fileName);
         }

         wprintf(L"\r\n");
      }
      
      result = 1;
   }

   for (std::vector<Test *>::iterator iter=tests.begin();
        iter!=tests.end();
        ++iter)
   {
      delete *iter;
   }

   system("pause");
   return result;
}
