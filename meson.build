
project('parprouted', 'cpp')
parprouted=executable('parprouted', 'src/parprouted.cpp', 'src/arp.cpp')

pkg = import('pkgconfig')

#catch2 = dependency('catch2')
catch2 = dependency('catch2-with-main')
trompeloeil = dependency('trompeloeil')

objs = parprouted.extract_objects(['src/arp.cpp', 'src/parprouted.cpp'])

e = executable('parprouted-test', ['src/parprouted-test.cpp'],
  objects : objs,
  dependencies : [
    catch2,
    trompeloeil,
  ]
)

test('name of test', e)

install_man('parprouted.8')

