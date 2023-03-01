project('parprouted', 'cpp')
parprouted=executable('parprouted', 'src/parprouted.cpp', 'src/arp.cpp', 'src/main.cpp', cpp_pch : 'pch/parprouted_pch.h', install: true)

#pkg = import('pkgconfig')

if get_option('unit-tests').enabled()
  catch2 = dependency('catch2-with-main')
  trompeloeil = dependency('trompeloeil')

  # catch2.install() = false

  objs = parprouted.extract_objects(['src/arp.cpp', 'src/parprouted.cpp'])

  e = executable('parprouted-test', ['src/parprouted-test.cpp'],
    objects : objs,
    dependencies : [
      catch2,
      trompeloeil,
    ]
  )

  test('name of test', e)
endif

pod2man = find_program('pod2man')

pod2man_opts = [
	'--section', '8',
  '--center', 'Proxy ARP Bridging Daemon',
  '--release', 'parprouted',
  '--date', '`date \'+%B %Y\'`',
]
parprouted_man = custom_target(
  'parprouted.8',
  input: 'parprouted.pod',
  output: 'parprouted.8',
  command: [
    pod2man, pod2man_opts, '@INPUT@', '@OUTPUT@',
  ],
  install: true,
  install_dir: 'man'
)

#install_man('parprouted.8')
