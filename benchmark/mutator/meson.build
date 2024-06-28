project('mutator', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++20'])

libprotobuf = dependency('protobuf')

inc = include_directories(
  '../', 
  '../../', 
  '../../include', 
  '../../include/libprotobuf-mutator'
)

libprotobuf_mutator_src = files(
  '../../libprotobuf-mutator/src/binary_format.cc',
  '../../libprotobuf-mutator/src/mutator.cc',
  '../../libprotobuf-mutator/src/text_format.cc',
  '../../libprotobuf-mutator/src/utf8_fix.cc',
)

lpmpp_src = files(
  '../../trimming.cc',
  '../../utils.cc',
  '../../statistics.cc',
  '../convert/convert.cc',
)

src = files(
  './vuln_mutator.cc',
  '../proto/vuln.pb.cc',
)

library('vuln_mutator',
        [src, lpmpp_src, libprotobuf_mutator_src],
        include_directories: inc,
        dependencies: [libprotobuf])
