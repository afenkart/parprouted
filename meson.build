project('parprouted', 'cpp')
parprouted=executable('parprouted', 'src/parprouted.cpp', 'src/arp.cpp', 'src/main.cpp', cpp_pch : 'pch/parprouted_pch.h')

#pkg = import('pkgconfig')

#catch2 = dependency('catch2')
catch2 = dependency('catch2-with-main')
trompeloeil = dependency('trompeloeil')
zlib = dependency('zlib')

objs = parprouted.extract_objects(['src/arp.cpp', 'src/parprouted.cpp'])

e = executable('parprouted-test', ['src/parprouted-test.cpp'],
  objects : objs,
  dependencies : [
    catch2,
    trompeloeil,
    zlib,
  ]
)

if get_option('unit-tests')
  test('name of test', e)
endif


install_man('parprouted.8')

