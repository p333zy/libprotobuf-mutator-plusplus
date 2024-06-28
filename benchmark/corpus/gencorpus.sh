#!/bin/bash

cd ../proto

protoc --encode=vuln.fuzz.RPCCall ./vuln.proto < ../corpus/vuln1/t1.pb.txt > ../corpus/vuln1/proto/t1.pb.bin
protoc --encode=vuln.fuzz.RPCCall ./vuln.proto < ../corpus/vuln1/t2.pb.txt > ../corpus/vuln1/proto/t2.pb.bin
protoc --encode=vuln.fuzz.RPCCall ./vuln.proto < ../corpus/vuln1/t3.pb.txt > ../corpus/vuln1/proto/t3.pb.bin
protoc --encode=vuln.fuzz.RPCCall ./vuln.proto < ../corpus/vuln1/t4.pb.txt > ../corpus/vuln1/proto/t4.pb.bin
