project('openSeaChest', 'c', license: 'MPL-2.0', version: '22.6.23', default_options : ['warning_level=2'])
#note: project version is year.month.day

c = meson.get_compiler('c')

#TODO: Add -Wcast-align=strict and fix these issues to help ensure better portability
#NOTE: -Wsign-conversion can be useful while debugging, but there are numerous places this shows up
#      and it is not useful, so only add it while debugging.
warning_flags = [
#	'-Wcast-align=strict',
#	'-Wsign-conversion',
	'-Wshadow=compatible-local',
	'-Wvla',
	'-Wfloat-equal',
	'-Wnull-dereference',
	'-Wunused-const-variable',
	'-Wduplicated-cond',
	'-Wjump-misses-init',
	'-Wstringop-overflow',
	'-Wlogical-op',
	'-Wshift-overflow=2',
	'-Wdouble-promotion',
	'-Wformat-security',
  '-Wold-style-definition',
  '-Wstrict-prototypes',
  '-Wmissing-declarations',
  '-Wmissing-prototypes'
]

add_project_arguments(c.get_supported_arguments(warning_flags), language : 'c')

if not (target_machine.system() == 'sunos') and (c.get_id() == 'gcc' or c.get_id() == 'clang')
  add_global_arguments(
      '-ffunction-sections',
      '-fdata-sections',
      language: 'c',
  )
  add_global_link_arguments(
      '-Wl,--gc-sections',
      language: 'c',
  )
  #if GCC less than 5, need to set -std=gnu99 at minimum. gnu11 became the default in 5, 17 default in 7 or 8.
  #TODO: May be able to move to c11/gnu11 instead, but will need to do a lot of testing first
  #skipping sunos since this was a compatibility issue that was reported earlier. May be able to find a better way to handle this in the future.
  if not (target_machine.system() == 'sunos') and c.get_id().contains('gcc')
    verStr = c.version().split('.')
    if verStr.get(0).to_int() < 5
        add_global_arguments('-std=gnu99', language: 'c',)
    endif
  endif
  
endif

if not get_option('tcg').enabled()
  add_project_arguments('-DDISABLE_TCG_SUPPORT', language : 'c')
endif

if get_option('debug')
  add_project_arguments('-D_DEBUG', language : 'c')
endif

if get_option('libc_musl')
  add_project_arguments('-DUSING_MUSL_LIBC=1', language : 'c')
endif

exec_prefix = 'openSeaChest_'
common_sources = ['src/EULA.c', 'src/openseachest_util_options.c']

os_deps = []

if target_machine.system() == 'windows'
  if not c.has_header('getopt.h')
    wingetopt = subproject('wingetopt', default_options: 'default_library=static')
    wingetopt_dep = wingetopt.get_variable('wingetopt_dep')
    os_deps += [wingetopt_dep]
  endif
  windows = import('windows')
  resources = windows.compile_resources('openSeaChest.rc')
  common_sources += [resources]
  add_project_arguments('-D_CRT_SECURE_NO_WARNINGS', language : 'c')
endif

opensea_common = subproject('opensea-common')
opensea_common_dep = opensea_common.get_variable('opensea_common_dep')

opensea_transport = subproject('opensea-transport')
opensea_transport_dep = opensea_transport.get_variable('opensea_transport_dep')

opensea_operations = subproject('opensea-operations')
opensea_operations_dep = opensea_operations.get_variable('opensea_operations_dep')

incdir = include_directories('include')

exe_src_map = {
  'FormatUnit': 'Format'
}

foreach p : get_option('tools')
  executable('openSeaChest_' + p, common_sources, 'utils/C/openSeaChest/openSeaChest_' + exe_src_map.get(p, p) + '.c', dependencies : [opensea_common_dep, opensea_transport_dep, opensea_operations_dep, os_deps], include_directories : incdir, install : true)
  if p not in ['NVMe', 'PassthroughTest', 'Security', 'ZBD'] # Blacklist tools without man pages
    install_man('docs/man/man8/openSeaChest_' + exe_src_map.get(p, p) + '.8')
  endif
endforeach
install_man('docs/man/man8/openSeaChest.8')
