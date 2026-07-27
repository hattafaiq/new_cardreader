set GRPC_TOOL_DIR=D:\kantor\cplus\macam\vcpkg-2024.12.16\installed\x86-windows\tools\protobuf
SET GRPC_GEN_DIR=D:\kantor\cplus\macam\vcpkg-2024.12.16\installed\x64-windows\tools\grpc
::versi id
set PROTO_LIVEFILE=card.proto
%GRPC_TOOL_DIR%\protoc.exe --grpc_out=. --cpp_out=. --plugin=protoc-gen-grpc=%GRPC_GEN_DIR%\grpc_cpp_plugin.exe -I . %PROTO_LIVEFILE%