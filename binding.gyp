{
	'targets': [{
		"target_name": "netstat",
		"sources": [
			"src/binding.cc",
			"src/get.cc"
		],
		"dependencies": [
			"deps/netstat/netstat.gyp:libnetstat"
		],
		"defines": [
			"_GNU_SOURCE"
		],
		"include_dirs": [
			"<!(node -e \"require('nan')\")"
		]
	}]
}
