project('parprouted', 'cpp', default_options : ['cpp_std=gnu++20'])

#pkg = import('pkgconfig')

parprouted = executable(
  'parprouted',
   [ 'src/parprouted.cpp', 'src/arp.cpp', 'src/main.cpp', 'src/fs.cpp', 'src/arp-table.cpp' ],
  install: true,
  install_dir: 'sbin',
)

pod2man = find_program('pod2man')
if pod2man.found()
  # strip locale -> Maerz
  date_cmd = run_command('date', '+%B %Y', check: true)

  pod2man_opts = [
    '--section', '8',
    '--center', 'Proxy ARP Bridging Daemon',
    '--release', 'parprouted',
    '--date', date_cmd.stdout().strip(),
  ]
  parprouted_man = custom_target(
    'parprouted.8',
    input: 'parprouted.pod',
    output: 'parprouted.8',
    command: [
      pod2man, pod2man_opts, '@INPUT@', '@OUTPUT@',
    ],
    install: true,
    install_dir: 'man/man8'
  )
endif

if get_option('unit-tests').enabled()
  catch2 = dependency('catch2-with-main')
  trompeloeil = dependency('trompeloeil')

  objs = parprouted.extract_objects(['src/arp.cpp', 'src/parprouted.cpp', 'src/arp-table.cpp'])

  e = executable('parprouted-test', ['src/parprouted-test.cpp', 'src/arp-table-test.cpp'],
    objects : objs,
    dependencies : [
      catch2,
      trompeloeil,
    ]
  )

  test('name of test', e)
endif
