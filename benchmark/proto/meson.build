run_command(
  'protoc', 
  '-I=./', 
  '--cpp_out=./', 
  './vuln.proto', 
  check: true)

proto_src = files(
  './vuln.pb.cc',
)
