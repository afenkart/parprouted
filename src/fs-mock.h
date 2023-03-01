#include <trompeloeil.hpp>

#include "fs.h"

struct FileSystemMock : trompeloeil::mock_interface<FileSystem> {
  IMPLEMENT_MOCK3(fgets);
};
