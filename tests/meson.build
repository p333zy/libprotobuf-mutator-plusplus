subdir('proto')

gtest = dependency('gtest')

trimtest = executable(
  'test_trimming', 
  [src, proto_src, files('test_trimming.cc')], 
  dependencies: [libprotobuf, gtest],
  include_directories: inc,
)

test('trimming tests', trimtest)
