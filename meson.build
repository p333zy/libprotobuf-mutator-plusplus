project('libprotobuf-mutator++', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++20'])

src = files(
  './statistics.cc',
  './trimming.cc',
  './utils.cc',
  'libprotobuf-mutator/src/binary_format.cc',
  'libprotobuf-mutator/src/mutator.cc',
  'libprotobuf-mutator/src/text_format.cc',
  'libprotobuf-mutator/src/utf8_fix.cc',
)

inc = include_directories('.', './libprotobuf-mutator')

libprotobuf = dependency('protobuf')

subdir('tests')
