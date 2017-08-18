#include <neurology/configuration.hpp>

Configuration::Configuration
(void)
{
}

Configuration::Configuration
(Configuration &config)
{
}

void
Configuration::Standard
(void)
{
   Heap::ProcessHeap = Heap(GetProcessHeap());
   Process::CurrentProcess = Process(GetCurrentProcess());
}
