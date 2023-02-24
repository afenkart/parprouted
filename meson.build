
project('parprouted', 'cpp')
executable('parprouted', 'src/parprouted.cpp', 'src/arp.cpp')

pkg = import('pkgconfig')

catch2 = dependency('catch2-with-main')
trompeloeil = dependency('trompeloeil')

e = executable('parprouted-test', 'src/parprouted-test.cpp',
  dependencies : [
    catch2,
    trompeloeil,
  ]
)

test('name of test', e)

