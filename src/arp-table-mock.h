#include <trompeloeil.hpp>

#include "arp-table.h"

struct ArpTableMock : trompeloeil::mock_interface<ArpTable> {
  IMPLEMENT_CONST_MOCK1(findentry);
};
