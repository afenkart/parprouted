project('parprouted', 'cpp')
parprouted=executable('parprouted', 'src/parprouted.cpp', 'src/arp.cpp', 'src/main.cpp', cpp_pch : 'pch/parprouted_pch.h')

#pkg = import('pkgconfig')

#catch2 = dependency('catch2')
zlib = dependency('zlib')

objs = parprouted.extract_objects(['src/arp.cpp', 'src/parprouted.cpp'])

if get_option('unit-tests').enabled()

  catch2 = dependency('catch2-with-main')
  trompeloeil = dependency('trompeloeil')

  e = executable('parprouted-test', ['src/parprouted-test.cpp'],
    objects : objs,
    dependencies : [
      catch2,
      trompeloeil,
      zlib,
    ]
  )

  test('name of test', e)
endif


install_man('parprouted.8')

