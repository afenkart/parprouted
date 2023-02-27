project('parprouted', 'cpp')
executable('parprouted', 'src/parprouted.cpp', 'src/arp.cpp', 'src/main.cpp', install: true)

pod2man = find_program('pod2man')
if pod2man.found()
  # strip locale -> Maerz
  date_cmd = run_command('date', '+%B %Y', env : {'LC_TIME' : 'C' }, check: true)

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
    install_dir: 'share/man/man8'
    # install_man('parprouted.8') -> install only, does not run podman
  )
endif

catch2 = dependency('catch2-with-main')
trompeloeil = dependency('trompeloeil')

e = executable('parprouted-test', 'src/parprouted-test.cpp',
  dependencies : [
    catch2,
    trompeloeil,
  ]
)

test('basic tests', e)
