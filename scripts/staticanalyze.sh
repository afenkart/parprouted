#!/bin/bash
set -e

cd ${MESON_SOURCE_ROOT}
rm -rf scantmp
mkdir scantmp && cd scantmp

scan-build meson setup ..
scan-build ninja

cd .. && rm -rf scantmp
