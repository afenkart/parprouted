  conan install . --output-folder=build --build=missing

https://docs.conan.io/2/examples/tools/meson/mesontoolchain/build_simple_meson_project.html#examples-tools-meson-toolchain-build-simple-meson-project

   cd build
   meson setup --native-file conan_meson_native.ini .. meson-build
   meson compile -C meson-build
   ./meson-build/compressor

meson setup --reconfigure --native-file native.ini build
meson setup --reconfigure -Db_coverage=true --native-file native.ini build

meson setup -Dunit-tests=disabled --cross-file cross-arm64.ini build-arm64


https://roy.marples.name/projects/parpd/
https://www.rfc-editor.org/rfc/rfc1027
https://www.gsp.com/cgi-bin/man.cgi?section=8&topic=parpd

ndppd
https://serverfault.com/questions/1106884/bridged-wireless-with-parprouted-is-there-a-way-to-do-the-same-with-ipv6
