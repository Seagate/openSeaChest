# SPDX-License-Identifier: MPL-2.0
# Copyright (c) 2021-2026 Seagate Technology LLC and/or its Affiliates, All Rights Reserved
project('openSeaChest', 'c', version: 'v26.03.0', license: 'MPL-2.0', default_options: ['warning_level=2', 'b_pie=true'])

c = meson.get_compiler('c')

machine = target_machine.system()
architecture = target_machine.cpu()
if architecture == ''
    architecture = target_machine.cpu_family()
endif
# Add debug flag if debug option is enabled
if get_option('debug')
    add_project_arguments('-D_DEBUG', language: 'c')
endif
# Add libc musl flag if enabled
if get_option('libc_musl')
    add_project_arguments('-DUSING_MUSL_LIBC=1', language: 'c')
endif
# Add JSON format support flag if enabled
if get_option('json_outputformat')
  add_project_arguments('-DFEATURE_JSONOUTPUT_SUPPORT', language : 'c')
endif
warning_flags = []
linker_flags = []  #additional linker flags to add per-compiler for hardening.

if c.get_id().contains('gcc') or c.get_id().contains('clang')
    #TODO: Add -Wcast-align=strict and fix these issues to help ensure better portability
    #NOTE: -Wsign-conversion can be useful while debugging, but there are numerous places this shows up
    #      and it is not useful, so only add it while debugging.
    #NOTE: 4/4/2024 - adding flags from https://best.openssf.org/Compiler-Hardening-Guides/Compiler-Options-Hardening-Guide-for-C-and-C++
    warning_flags = [
        #	'-Wcast-align=strict',
        '-Wshadow=compatible-local',
        '-Wvla',
        '-Wfloat-equal',
        '-Wnull-dereference',
        '-Wunused-const-variable',
        '-Wunused-parameter',
        '-Wunused-value',
        '-Wduplicated-cond',
        '-Wjump-misses-init',
        '-Wstringop-overflow',
        '-Wlogical-op',
        '-Wshift-overflow',
        '-Wshift-overflow=1',
        '-Wshift-overflow=2',
        '-Wdouble-promotion',
        '-Werror=format-security',
        '-Wold-style-definition',
        '-Wstrict-prototypes',
        '-Wmissing-declarations',
        '-Wmissing-prototypes',
        '-Wchar-subscripts',
        '-Wundef',
        '-Wformat',
        '-Wformat=2',
        '-Wint-conversion',  #-Warith-conversion
        '-Wenum-conversion',
        '-Wfloat-conversion',
        '-Wint-to-pointer-cast',
        '-Wimplicit-fallthrough',
        '-D_GLIBCXX_ASSERTIONS',
        '-fstrict-flex-arrays=3',
        '-fno-delete-null-pointer-checks',
        '-fno-strict-overflow',
        '-fno-strict-aliasing',
        '-ftrivial-auto-var-init=zero',
        '-fzero-init-padding-bits=all',
        '-Wtrampolines',  #GCC only at this time
        '-Werror=implicit',
        '-Werror=incompatible-pointer-types',
        '-Wincompatible-pointer-types-discards-qualifiers',
        '-Werror=int-conversion',
        '-Werror=implicit-int',
        '-Woverlength-strings',
        '-Wnewline-eof',
        '-Wno-c23-extensions',  #We do not want this warning since we are already checking for when C23 extensions are available before we use them. If not, we use a compiler specific definition, or make it an empty definition.
        '-Wparentheses',
        '-Wextra-semi',
        '-Wcast-qual',
        '-Werror=sometimes-uninitialized',
        '-Wuninitialized',
        '-Wunevaluated-expression',
        '-Wunsequenced',
        '-Wvarargs',
        '-Wwrite-strings',
        '-Wrestrict',
        '-Wstringop-truncation',
        '-Werror=trigraphs',
        '-Wunreachable-code',
        '-Wcomment',
        '-Wsequence-point',
        '-Wreturn-type',
        '-Wpointer-bool-conversion',
        '-fvisibility=hidden',  #to work similarly to Window's DLL import/export
        '-Wmismatched-dealloc',
        '-Wfree-nonheap-object',
        '-Wnullability',  # Warnings for nullability annotations
        '-Wnullability-completeness',  # Warn if nullability annotations are incomplete
        '-Wnullability-completeness-on-arrays',  # Warn if nullability annotations are incomplete
        # '-Wnullable-to-nonnull-conversion', # Basically warns so you have to cast to the correct nullability type. Not needed today -TJE
        #'-Wswitch-enum',
        '-Walloc-zero',
        '-Walloc-size',
        '-Wcalloc-transposed-args',
        '-Werror=alloca',  # Do not allow use of the alloca function
        '-Wstring-compare',  # Helps detect when strcmp is used when the programmer likely meant to use strncmp
        '-Wfortify-source' # needed for diagnose_as_builtin attributes to work.
    ]

    if get_option('cc-suggest-attribute')
        warning_flags += [
            '-Wsuggest-attribute=pure',
            '-Wsuggest-attribute=const',
            '-Wsuggest-attribute=noreturn',
            '-Wsuggest-attribute=returns_nonnull',
            '-Wmissing-noreturn',
            '-Wsuggest-attribute=malloc',
            '-Wsuggest-attribute=format',
            '-Wmissing-format-attribute',
            '-Wsuggest-attribute=cold',
            '-fipa-pure-const',
        ]
    endif
    if c.get_id().contains('gcc') and c.version().version_compare('>=10.0')
        #only enable the sign conversion warning on versions 10 and up because it is way too noisy on earlier GCC versions than it is useful-TJE
        warning_flags += '-Wsign-conversion'
    endif

    if target_machine.system() != 'sunos'
        warning_flags += '-fstack-protector-strong'
    else
        #Illumos will support this, but solaris will not due to library differences in the systems
        if meson.version().version_compare('>=1.2.0')
            if target_machine.kernel() == 'illumos'
                warning_flags += '-fstack-protector-strong'
            endif
        else
            # TODO: Backup method to detect illumos vs solaris
        endif
    endif

    if c.get_id().contains('gcc') and target_machine.system() == 'windows'
        #According to the link below, this is not needed in Windows...it also causes a bug in some versions of GCC for Windows.
        #https://gcc.gnu.org/bugzilla/show_bug.cgi?id=90458
        #NOTE: While this appears to be fixed for versions 11 and 12, the current CI is failing on this with
        #      version 12.2. If we want to or need to enable this, it should be done based on which versions we
        #      know have been patched for this. -TJE
    else
        warning_flags += '-fstack-clash-protection'
    endif

    if target_machine.cpu_family() == 'ppc64'
        #power pc builds generate a lot of warnings/notes about ABI changes since GCC-5
        #this flag is disabling them because this is way too noisy.
        warning_flags += ['-Wno-psabi']
    elif target_machine.cpu_family() == 'x86_64'
        warning_flags += ['-fcf-protection=full']        #this may be linux only at this time.
    elif target_machine.cpu_family() == 'aarch64'
        warning_flags += ['-mbranch-protection=standard']
    endif

    linker_flags += ['-Wl,-z,nodlopen', '-Wl,-z,noexecstack', '-Wl,-z,relro', '-Wl,-z,now']
    fortifytest = ''' #include <stdio.h>
                      int main() {
                          return 0;
                      }
                  '''
    fortifyresult = c.compiles(
        fortifytest,
        name: '_FORTIFY_SOURCE override',
        args: ['-U_FORTIFY_SOURCE', '-D_FORTIFY_SOURCE=5', '-Werror'],
    )
    if fortifyresult == true
        warning_flags += ['-U_FORTIFY_SOURCE', '-D_FORTIFY_SOURCE=3']
    endif
elif c.get_id().contains('msvc')
    #See here for enabling/disabling msvc warnings:
    #https://learn.microsoft.com/en-us/cpp/build/reference/compiler-option-warning-level?view=msvc-170
    #warnings off by default: https://learn.microsoft.com/en-us/cpp/preprocessor/compiler-warnings-that-are-off-by-default?view=msvc-170
    warning_flags = [
        #Turn off the following warnings. If using /wall in Windows, many of these show all over the Windows API
        #This is likely not an issue with meson, but matching VS project files for now
        '/wd4214',  # nonstandard extension used : bit field types other than int
        '/wd4201',  # nonstandard extension used : nameless struct/union
        '/wd4668',  # 'symbol' is not defined as a preprocessor macro, replacing with '0' for 'directives'. While like -Wundef, this creates too many warnings in system headers to use
        '/wd4820',  # 'bytes' bytes padding added after construct 'member_name'
        '/wd4710',  # 'function' : function not inlined
        '/wd5045',  # Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
        '/wd4711',  # function 'function' selected for inline expansion
        '/wd4324',  # 'struct_name' : structure was padded due to __declspec(align())
        '/wd4221',  # nonstandard extension used : 'identifier' : cannot be initialized using address of automatic variable
        '/wd4204',  # nonstandard extension used : non-constant aggregate initializer
        '/wd5105',  # macro expansion producing 'defined' has undefined behavior
        '/wd4746',  # volatile access of '<expression>' is subject to /volatile:[iso|ms] setting; consider using __iso_volatile_load/store intrinsic functions.
        #Turn on the following warnings to make the output more useful or like GCC/clang
        '/w14255',  # 'function' : no function prototype given: converting '()' to '(void)'
        '/w14062',  # enumerator 'identifier' in switch of enum 'enumeration' is not handled
        '/w14101',  # 'identifier' : unreferenced local variable
        '/w14189',  # 'identifier' : local variable is initialized but not referenced
        '/w15031',  # #pragma warning(pop): likely mismatch, popping warning state pushed in different file
        '/w15032',  # detected #pragma warning(push) with no corresponding #pragma warning(pop)
        '/w15262',  # implicit fall-through occurs here; are you missing a break statement? Use [[fallthrough]] when a break statement is intentionally omitted between cases
        '/w14255',  # 'function' : no function prototype given: converting '()' to '(void)' #NOTE: Only needed for /Wall, otherwise enabling can be good-TJE
        '/w14242',  # identifier conversion from type 1 to type 2, possible loss of data (matches -wconversion above)
        '/w14254',  # operator conversion from type 1 to type 2, possible loss of data (matches -wconversion above)
        '/w14287',  # operator: unsigned/negative constant mismatch (matches -wconversion above)
        '/w14296',  # operator: expression is always false
        '/w14365',  # action: conversion from type 1 to type 2, signed/unsigned mismatch (matches -wconversion above)
        '/w14388',  # implicit conversion warning during a comparison (matches -wconversion above)
        '/w14545',  # expression before comma evaluates to a function which is missing an argument list
        '/w14546',  # function call before comma missing argument list
        '/w14547',  # 'operator' : operator before comma has no effect; expected operator with side-effect
        '/w14548',  # expression before comma has no effect; expected expression with side-effect
        '/w14549',  # 'operator1': operator before comma has no effect; did you intend 'operator2'?
        '/w14574',  # 'identifier' is defined to be '0': did you mean to use '#if identifier'?
        '/w14605',  # 	'/Dmacro' specified on current command line, but was not specified when precompiled header was built
        '/w14555',  # expression has no effect; expected expression with side-effect
        '/w14774',  # 'string' : format string expected in argument number is not a string literal
        '/w14777',  # 'function' : format string 'string' requires an argument of type 'type1', but variadic argument number has type 'type2'
        '/w14826',  # Conversion from 'type1' to 'type2' is sign-extended. This may cause unexpected runtime behavior (more -wconversion)
        '/w15219',  # implicit conversion from 'type-1' to 'type-2', possible loss of data (-wconversion)
        '/w15240',  # 'attribute-name': attribute is ignored in this syntactic position
        '/w15245',  # 'function': unreferenced function with internal linkage has been removed
        '/w14555',  # expression has no effect; expected expression with side-effect
        '/w15264',  # 'variable-name': 'const' variable is not used
        '/w24302',  # 'conversion': truncation from 'type1' to 'type2'
        '/w14311',  # 'variable': pointer truncation from 'type' to 'type'
        '/w14312',  # 'operation': conversion from 'type1' to 'type2' of greater size
        '/w14319',  # 'operator': zero extending 'type1' to 'type2' of greater size
        #Treat the following as errors
        '/we4431',  # missing type specifier - int assumed. Note: C no longer supports default-int
        '/we4905',  # wide string literal cast to 'LPSTR'
        '/we4906',  # string literal cast to 'LPWSTR'
        '/we4837',  # trigraph detected: '??character' replaced by 'character'
        '/we4628',  # digraphs not supported with -Ze. Character sequence 'digraph' not interpreted as alternate token for 'char'
        '/we4289',  # nonstandard extension used : 'var' : loop control variable declared in the for-loop is used outside the for-loop scope
        '/we4464',  # relative include path contains '..'
        '/GS',  #security cookie for stack protection
        '/sdl',  #adds recommended security development lifecycle checks
        '/Qspectre',
        '/guard:cf',  #control flow guard
        '/d2guard4',  #control flow guard
    ]

    if target_machine.cpu_family() == 'x86_64' or target_machine.cpu_family() == 'x86'
        # https://learn.microsoft.com/en-us/cpp/build/reference/qintel-jcc-erratum?view=msvc-170
        warning_flags += '/QIntel-jcc-erratum'
    endif

    if c.has_argument('/std:c17')
        c_std = 'c17'
    endif

    linker_flags += [
        '/guard:cf',  #control flow guard
        '/SafeSEH',  #on by default in x64 so it is unrecognized otherwise.
        '/NXCOMPAT',  #data execution prevention
        '/dynamicbase',  #address space randomization
    ]
elif c.get_id().contains('xlc')
    #This section is for IBM's xlc compiler and warning options it may need.
    #NOTE: xlcclang should be handled above
    #See following links:
    #https://www.ibm.com/docs/en/xl-c-and-cpp-aix/16.1?topic=reference-individual-xl-compiler-option-descriptions
    #https://www.ibm.com/docs/en/xl-c-and-cpp-aix/16.1?topic=reference-supported-xl-compiler-options-by-different-invocations
    #https://www.ibm.com/docs/en/xl-c-and-cpp-aix/16.1?topic=end-mapping-legacy-xl-compiler-options-gcc-options
    #https://www.ibm.com/docs/en/xl-c-and-cpp-aix/16.1?topic=reference-individual-xl-compiler-option-descriptions
    #https://www.ibm.com/docs/en/xl-c-and-cpp-aix/16.1?topic=descriptions-qlanglvl-std
    warning_flags = []
endif

# =============================================================================
# REPRODUCIBLE BUILD SUPPORT
# =============================================================================

source_epoch = ''

posixshell = find_program('sh', required: false)
bashshell = find_program('bash', required: false)
# 1. Check for SOURCE_DATE_EPOCH environment variable using shell
if posixshell.found()
    env_cmd = run_command(posixshell, '-c', 'echo $SOURCE_DATE_EPOCH', check: false)
    if env_cmd.returncode() == 0 and env_cmd.stdout().strip() != ''
        source_epoch = env_cmd.stdout().strip()
    endif
elif bashshell.found()
    env_cmd = run_command(bashshell, '-c', 'echo $SOURCE_DATE_EPOCH', check: false)
    if env_cmd.returncode() == 0 and env_cmd.stdout().strip() != ''
        source_epoch = env_cmd.stdout().strip()
    endif
else
    # Probably in Windows, so check for powershell and try to get the environment variable from there
    powershell = find_program('pwsh', 'powershell', required: false)
    if powershell.found()
        env_cmd = run_command(powershell, '-NoProfile', '-NonInteractive', '-Command', '[Environment]::GetEnvironmentVariable("SOURCE_DATE_EPOCH")', check: false)
        if env_cmd.returncode() == 0 and env_cmd.stdout().strip() != ''
            source_epoch = env_cmd.stdout().strip()
        endif
    else
        cmdexe = find_program('cmd.exe', required: false)
        if cmdexe.found()
            # cmd.exe is the last resort
            env_cmd = run_command(cmdexe, '/c', 'echo %SOURCE_DATE_EPOCH%', check: false)
            if env_cmd.returncode() == 0 and env_cmd.stdout().strip() != '' and env_cmd.stdout().strip() != '%SOURCE_DATE_EPOCH%'
                source_epoch = env_cmd.stdout().strip()
            endif
        endif
    endif
endif
# 2. If Env Var is empty, try to fetch from Git
if source_epoch == ''
    git = find_program('git', required: false)
    if git.found()
        # Get the last commit timestamp (Unix Epoch format)
        git_cmd = run_command(git, 'log', '-1', '--pretty=%ct', check: false)
        if git_cmd.returncode() == 0
            source_epoch = git_cmd.stdout().strip()
        endif
    endif
endif

# 3. If we found a timestamp, format it and pass to C compiler
if source_epoch != ''
    # Format: "%b %d %Y" -> "Nov 17 2025"
    # Note: __DATE__ uses space padding for days < 10 (e.g. "Nov  7 2025").

    # Try Python first (most reliable, handles formatting consistently)
    python = find_program('python3', 'python', required: false)
    if python.found()
        py_script = 'import datetime; ' + \
                    'ts = int(' + source_epoch + '); ' + \
                    'dt = datetime.datetime.fromtimestamp(ts, datetime.timezone.utc);' + \
                    'print(dt.strftime("%b %d %Y").replace(" 0", "  "));'

        date_cmd = run_command(python, '-c', py_script, check: false)
        if date_cmd.returncode() == 0
            formatted_date = date_cmd.stdout().strip()
            message('Reproducible Build: Setting BUILD_TIMESTAMP to "' + formatted_date + '" (via Python)')
            add_project_arguments('-DBUILD_TIMESTAMP="' + formatted_date + '"', language: 'c')
        endif
    endif

    # Fallback to date command if Python failed or not found
    if not python.found() or date_cmd.returncode() != 0
        message('Reproducible Build: Python not available, trying date command')

        # Try GNU date first (Linux, MUSL)
        date_cmd = run_command('sh', '-c', 'date -u -d @' + source_epoch + ' "+%b %_d %Y" 2>/dev/null', check: false)

        if date_cmd.returncode() != 0
            # Try BSD date (FreeBSD, macOS, OpenBSD, NetBSD, DragonFlyBSD)
            date_cmd = run_command('sh', '-c', 'date -u -r ' + source_epoch + ' "+%b %_d %Y" 2>/dev/null', check: false)
        endif

        if date_cmd.returncode() == 0
            formatted_date = date_cmd.stdout().strip()
            message('Reproducible Build: Setting BUILD_TIMESTAMP to "' + formatted_date + '" (via date command)')
            add_project_arguments('-DBUILD_TIMESTAMP="' + formatted_date + '"', language: 'c')
        else
            message('Reproducible Build: Failed to format timestamp. Using compiler default (__DATE__).')
        endif
    endif
else
    message('Reproducible Build: No SOURCE_DATE_EPOCH or Git found. Using compiler default (__DATE__).')
endif
# =============================================================================

add_project_arguments(c.get_supported_arguments(warning_flags), language: 'c')
add_project_link_arguments(c.get_supported_link_arguments(linker_flags), language: 'c')

small_code_cc_flags = []
small_code_link_flags = []

if (c.get_id() == 'gcc' or c.get_id() == 'clang') and target_machine.system() != 'windows'

    small_code_cc_flags = ['-ffunction-sections', '-fdata-sections']
    small_code_link_flags = ['-Wl,--gc-sections']

    #if GCC less than 5, need to set -std=gnu99 at minimum. gnu11 became the default in 5, 17 default in 7 or 8.
    #TODO: May be able to move to c11/gnu11 instead, but will need to do a lot of testing first
    #skipping sunos since this was a compatibility issue that was reported earlier. May be able to find a better way to handle this in the future.
    if not (target_machine.system() == 'sunos') and c.get_id().contains('gcc')
        if c.version().version_compare('<5.0')
            #4.7.4+ has C11 support, but c89 is the default standard so we need to change it.
            if c.has_argument('-std=gnu11')
                c_std = 'gnu11'
                if meson.version().version_compare('<1.0.0')
                    add_project_arguments('-std=gnu11', language: 'c')
                endif
            elif c.has_argument('-std=gnu99')
                #Add this argument to the list since C99 is a minimum required C compiler standard
                c_std = 'gnu99'
                if meson.version().version_compare('<1.0.0')
                    add_project_arguments('-std=gnu99', language: 'c')
                endif
            else
                error('C99/GNU99 standard is required but was not able to be set!')
            endif
        endif
    endif

endif

add_global_arguments(c.get_supported_arguments(small_code_cc_flags), language: 'c')
add_global_link_arguments(c.get_supported_link_arguments(small_code_link_flags), language: 'c')

if not get_option('tcg').enabled()
    add_project_arguments('-DDISABLE_TCG_SUPPORT', language: 'c')
endif

if get_option('atasecsetpass').enabled()
    add_project_arguments('-DENABLE_ATA_SET_PASSWORD', language: 'c')
endif

exec_prefix = 'openSeaChest_'
common_sources = ['src/EULA.c', 'src/openseachest_util_options.c']

os_deps = []

#The wingetopt has been amended to build on any system openSeaChest supports.
#This was done as AIX did not have getopt_long so this was done.
#Additionally, it made sense to keep the same implementation between OSs so it is now used for getopt_long support.
wingetopt = subproject('wingetopt', default_options: 'default_library=static')
wingetopt_dep = wingetopt.get_variable('wingetopt_dep')
os_deps += [wingetopt_dep]

if target_machine.system() == 'windows'
    windows = import('windows')
    resources = windows.compile_resources('openSeaChest.rc')
    common_sources += [resources]
    add_project_arguments('-D_CRT_SECURE_NO_WARNINGS', language: 'c')
endif

usehelp2man = false
if target_machine.system() != 'windows'
    if meson.can_run_host_binaries()
        help2man = find_program('help2man', required: false)
        if help2man.found()
            usehelp2man = true
        endif
    endif
endif

opensea_common = subproject('opensea-common')
opensea_common_dep = opensea_common.get_variable('opensea_common_dep')

opensea_transport = subproject('opensea-transport')
opensea_transport_dep = opensea_transport.get_variable('opensea_transport_dep')

csmisupport = opensea_transport.get_variable('csmisupport')
if csmisupport.enabled()
    add_project_arguments('-DENABLE_CSMI', language: 'c')
endif

opensea_operations = subproject('opensea-operations')
opensea_operations_dep = opensea_operations.get_variable('opensea_operations_dep')

jsonc_dep = disabler()
opensea_jsonformat_dep = disabler()

if get_option('json_outputformat')

    add_project_arguments('-DFEATURE_JSONOUTPUT_SUPPORT', language : 'c')

    # Pass BUILD_TIMESTAMP to opensea-jsonformat for reproducible builds
    jsonformat_options = []
    if source_epoch != ''
        jsonformat_options += 'build_timestamp=' + formatted_date
    endif

    opensea_jsonformat = subproject('opensea-jsonformat',
        default_options: jsonformat_options
    )
    opensea_jsonformat_dep = opensea_jsonformat.get_variable('opensea_jsonformat_dep')

    jsonc = subproject(
        'json-c',
        default_options: [
            'default_library=static',
            'build_apps=false',
            'disable_extra_libs=true',
            'disable_json_pointer=false',
            'disable_json_patch=false',
        ],
    )
    jsonc_dep = jsonc.get_variable('jsonc_dep')
endif

incdir = include_directories('include')

exe_src_map = {
    #This exists to allow mapping an exe from a specific filename to a different output name.
    #the openSeaChest_Format tool was called openSeaChest_FormatUnit before NVMe was added and needed this map for a while.
    #This is no longer needed at this time, but may end up used in the future as more utilities are migrated to openSeaChest
}

foreach p : get_option('tools')
    exe_deps = [
        opensea_common_dep,
        opensea_transport_dep,
        opensea_operations_dep,
        os_deps,
    ]

    # Only add JSON dependencies if the feature is enabled
    if get_option('json_outputformat')
        exe_deps += [jsonc_dep, opensea_jsonformat_dep]
    endif

    executable(
        'openSeaChest_' + p,
        common_sources,
        'utils/C/openSeaChest/openSeaChest_' + exe_src_map.get(p, p) + '.c',
        dependencies: exe_deps,
        include_directories: incdir,
        install: true,
    )
    if usehelp2man
        #use help2man to generate manpages before installing them
        run_command(
            help2man,
            '-o',
            meson.project_source_root() + '/docs/man/man8/openSeaChest_' + exe_src_map.get(p, p) + '.8',
            meson.project_build_root() + '/openSeaChest_' + exe_src_map.get(p, p),
            check: false,
        )
    endif
    install_man('docs/man/man8/openSeaChest_' + exe_src_map.get(p, p) + '.8')
endforeach
install_man('docs/man/man8/openSeaChest.8')
