#pragma once

#include <trompeloeil.hpp>

#include "fs.h"

struct FileSystemMock : trompeloeil::mock_interface<FileSystem> {
  IMPLEMENT_MOCK2(fopen);
  IMPLEMENT_MOCK1(fclose);
  IMPLEMENT_MOCK3(fgets);
  IMPLEMENT_MOCK1(feof);
  IMPLEMENT_MOCK1(ferror);
};
