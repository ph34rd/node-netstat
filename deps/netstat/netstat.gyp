{
	'targets': [{
		'target_name': 'libnetstat',
		'type': 'static_library',
		'direct_dependent_settings': {
			'include_dirs': [ 'include' ],
		},
		'include_dirs': [
			'include',
			'src/'
		],
		'sources': [ 'include/netstat.h' ],
		'defines': [
			'HAVE_CONFIG_H'
		],
		'conditions': [
			['OS=="mac"', {
				'defines': [
					'NODE_DARWIN'
				],
				'sources': [ 'src/darwin/ia.c' ],
				'cflags': [
					'-g',
					'--std=gnu89',
					'-pedantic',
					'-Wall',
					'-Wextra',
					'-Wno-unused-parameter',
				],
				'xcode_settings': {
					'GCC_C_LANGUAGE_STANDARD': 'c89'
				}
			}],
			['OS=="linux"', {
				'defines': [
					'NODE_LINUX'
				],
				'sources': [ 'src/linux/ia.c' ],
				'cflags': [
					'-g',
					'--std=gnu89',
					'-pedantic',
					'-Wall',
					'-Wextra',
					'-Wno-unused-parameter',
				]
			}]
		]
	}]
}
