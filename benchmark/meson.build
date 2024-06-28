project('benchmark', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++20'])

subdir('proto')

inc = include_directories('include/', '../include/', '../../libprotobuf-mutator/')
src = files('src/vuln1.cc')
deps = []
ldflags = []

cc = meson.get_compiler('cpp')
if get_option('mode') == 'afl'
  if not cc.has_argument('-fsanitize=fuzzer')
    error('Compiler must support -fsanitize=fuzzer')
  endif
  deps += cc.find_library('afl/libAFLDriver')
  add_project_arguments('-fsanitize=fuzzer', language : ['cpp', 'c'])
  src += files('src/harness/afl.cc')
else
  if not cc.has_argument('-fsanitize=fuzzer-no-link')
    error('Compiler must support -fsanitize=fuzzer-no-link')
  endif
  deps += dependency('protobuf')
  add_project_arguments('-fsanitize=fuzzer-no-link', language : ['cpp', 'c'])
  ldflags += '-fsanitize=fuzzer'
  src += files(
    'src/harness/lpm.cc', 
    'proto/vuln.pb.cc',
    'convert/convert.cc',
    '../libprotobuf-mutator/src/binary_format.cc',
    '../libprotobuf-mutator/src/mutator.cc',
    '../libprotobuf-mutator/src/text_format.cc',
    '../libprotobuf-mutator/src/utf8_fix.cc',
    '../libprotobuf-mutator/src/libfuzzer/libfuzzer_macro.cc',
    '../libprotobuf-mutator/src/libfuzzer/libfuzzer_mutator.cc',
  )
endif

executable('fuzz_vuln1',
           src,
           include_directories: inc,
           dependencies: deps,
           link_args: ldflags)
