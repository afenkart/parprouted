/*
 * This application is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#pragma once

#include <cstdio>
#include <memory>

struct FileSystem {
  virtual FILE *fopen(const char *pathname, const char *mode) = 0;
  virtual int fclose(FILE *) = 0;
  virtual int feof(FILE *) = 0;
  virtual int ferror(FILE *) = 0;
  virtual char *fgets(char s[], int size, FILE *stream) = 0;
  virtual ~FileSystem() = default;
};

std::unique_ptr<FileSystem> makeFileSystem();
