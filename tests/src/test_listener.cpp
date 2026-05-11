#include <catch2/reporters/catch_reporter_event_listener.hpp>
#include <catch2/reporters/catch_reporter_registrars.hpp>
#include <catch2/catch_test_case_info.hpp>
#include <iostream>

#include <Windows.h>

class test_name_printer : public Catch::EventListenerBase
{
public:
  using Catch::EventListenerBase::EventListenerBase;

  void testCaseStarting(Catch::TestCaseInfo const& test_info) override
  {
    SetConsoleOutputCP(CP_UTF8);

    std::cout << "\n=== [TEST] " << test_info.name << " ===\n";
  }
};

CATCH_REGISTER_LISTENER(test_name_printer)
