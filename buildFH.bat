set VCPKGROOTDIR=C:\MurniSN\Project_cpp\vcpkg-master\vcpkg-master

cmake -Bbuildx86 -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=cl -DCMAKE_CXX_COMPILER=cl ^
-Dspdlog_DIR=%VCPKGROOTDIR%\installed\x86-windows\share\spdlog ^
-Dfmt_DIR=%VCPKGROOTDIR%\installed\x86-windows\share\fmt ^
-DgRPC_DIR=%VCPKGROOTDIR%\installed\x86-windows\share\grpc ^
-DZLIB_LIBRARY=%VCPKGROOTDIR%\installed\x86-windows\lib\zlib.lib ^
-DZLIB_INCLUDE_DIR=%VCPKGROOTDIR%\installed\x86-windows\include ^
-DProtobuf_PROTOC_EXECUTABLE=%VCPKGROOTDIR%\installed\x86-windows\tools\protobuf\protoc.exe ^
-DProtobuf_DIR=%VCPKGROOTDIR%\installed\x86-windows\share\protobuf ^
-DOPENSSL_ROOT_DIR=%VCPKGROOTDIR%\installed\x86-windows ^
-Dc-ares_DIR=%VCPKGROOTDIR%\installed\x86-windows\share\c-ares ^
-Dabsl_DIR=%VCPKGROOTDIR%\installed\x86-windows\share\absl ^
-Dupb_DIR=%VCPKGROOTDIR%\installed\x86-windows\share\upb ^
-Dre2_DIR=%VCPKGROOTDIR%\installed\x86-windows\share\re2 ^
-DPKG_CONFIG_EXECUTABLE=%VCPKGROOTDIR%\installed\x86-windows\tools\pkgconf\pkgconf.exe ^
-Dsqlpp11_DIR=%VCPKGROOTDIR%\installed\x86-windows\share\sqlpp11 ^
-Dunofficial-sqlite3_DIR=%VCPKGROOTDIR%\installed\x86-windows\share\unofficial-sqlite3 ^
-Ddate_DIR=%VCPKGROOTDIR%\installed\x86-windows\share\date ^
-Ddlfcn-win32_DIR=%VCPKGROOTDIR%\installed\x86-windows\share\dlfcn-win32 ^
-Dutf8_range_DIR=%VCPKGROOTDIR%\installed\x86-windows\share\utf8_range ^
-DCMAKE_EXPORT_COMPILE_COMMANDS=1 ^
-Dquirc_DIR=%VCPKGROOTDIR%\installed\x86-windows\share\quirc ^
-DOpenCV_DIR=%VCPKGROOTDIR%\installed\x86-windows\share\opencv ^
-DTIFF_INCLUDE_DIR=%VCPKGROOTDIR%\installed\x86-windows\include ^
-DTIFF_LIBRARY=%VCPKGROOTDIR%\installed\x86-windows\lib\tiff.lib ^
-DCURL_DIR=%VCPKGROOTDIR%\installed\x86-windows\share\curl ^
-DBUILD_DLL_ONLY=OFF ^
-Bbuild -G Ninja .