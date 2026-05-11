#include <catch2/reporters/catch_reporter_event_listener.hpp>
#include <catch2/reporters/catch_reporter_registrars.hpp>
#include <iostream>

class test_name_printer : public Catch::EventListenerBase
{
public:
  using Catch::EventListenerBase::EventListenerBase;

  void testCaseStarting(Catch::TestInfo const& test_info) override
  {
    std::cerr << "\n=== [TEST] " << test_info.name << " ===\n";
  }
};

CATCH_REGISTER_LISTENER(test_name_printer)
