
meson setup --reconfigure --native-file native.ini build
meson setup --reconfigure -Db_coverage=true --native-file native.ini build

meson setup --cross-file cross-arm64.ini --buildtype release --optimization 3 --debug -Dunit-tests=disabled --wipe build-arm64

https://roy.marples.name/projects/parpd/
https://www.rfc-editor.org/rfc/rfc1027
https://www.gsp.com/cgi-bin/man.cgi?section=8&topic=parpd
