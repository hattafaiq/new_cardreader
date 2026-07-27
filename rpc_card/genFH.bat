set GRPC_TOOL_DIR=C:\MurniSN\Project_cpp\vcpkg-master\vcpkg-master\installed\x64-windows\tools\protobuf
SET GRPC_GEN_DIR=C:\MurniSN\Project_cpp\vcpkg-master\vcpkg-master\installed\x64-windows\tools\grpc
::versi id
set PROTO_LIVEFILE=card.proto
%GRPC_TOOL_DIR%\protoc.exe --grpc_out=. --cpp_out=. --plugin=protoc-gen-grpc=%GRPC_GEN_DIR%\grpc_cpp_plugin.exe -I . %PROTO_LIVEFILE%