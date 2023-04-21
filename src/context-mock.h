#pragma once

#include <trompeloeil.hpp>

#include "context.h"

struct ContextMock : trompeloeil::mock_interface<Context> {
  IMPLEMENT_MOCK1(system);
  IMPLEMENT_MOCK3(bind);
  IMPLEMENT_MOCK3(socket);
  IMPLEMENT_MOCK6(sendto);
};
