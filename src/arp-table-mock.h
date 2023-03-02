#include <trompeloeil.hpp>

#include "arp-table.h"

struct ArpTableMock : trompeloeil::mock_interface<ArpTable> {
  IMPLEMENT_CONST_MOCK1(findentry);
  IMPLEMENT_MOCK2(replace_entry);
  IMPLEMENT_MOCK2(remove_other_routes);
};
