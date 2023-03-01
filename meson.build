project('parprouted', 'cpp')
executable('parprouted', 'src/parprouted.cpp', 'src/arp.cpp')

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
  install_dir: 'share/man/man8'
)

# does not run podman
#install_man('parprouted.8')
