  conan install . --output-folder=build --build=missing

https://docs.conan.io/2/examples/tools/meson/mesontoolchain/build_simple_meson_project.html#examples-tools-meson-toolchain-build-simple-meson-project

   cd build
   meson setup --native-file conan_meson_native.ini .. meson-build
   meson compile -C meson-build
   ./meson-build/compressor

