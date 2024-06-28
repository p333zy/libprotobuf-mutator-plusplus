run_command(
  'protoc', 
  '-I=./', 
  '--cpp_out=./', 
  './test.proto', 
  check: true)

proto_src = files(
  './test.pb.cc',
)