project('parprouted', 'cpp', default_options: [ 'warning_level=3', 'cpp_std=c++20'], version: '0.8.0', license: 'gpl-2.0+')

add_global_arguments(['-Wuseless-cast', '-Wconversion', '-Wstrict-aliasing'], language: 'cpp')

cpp_files = files('src/parprouted.cpp', 'src/arp.cpp', 'src/main.cpp', 'src/fs.cpp', 'src/context.cpp')

parprouted = executable(
  'parprouted',
  cpp_files,
  install: true,
  install_dir: 'sbin',
)

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

if get_option('unit-tests').enabled()
  catch2 = dependency('catch2')
  trompeloeil = dependency('trompeloeil')

  objs = parprouted.extract_objects(['src/arp.cpp', 'src/parprouted.cpp'])
  e = executable('parprouted-test', ['src/parprouted-test.cpp', 'src/test-main.cpp'],
    objects : objs,
    dependencies : [
      catch2,
      trompeloeil,
    ]
  )
  test('basic tests', e)
endif

run_target('staticanalyze', command: 'scripts/staticanalyze.sh')

clangtidy = find_program('clang-tidy', required: false)
if clangtidy.found()
    run_target(
        'clang-tidy',
        command: [
            'scripts/clang-tidy.sh',
            clangtidy.path(),
            meson.source_root(),
            meson.build_root(),
            cpp_files,
        ],
    depends: parprouted)
endif
