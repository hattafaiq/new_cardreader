cmake -Bbuildx86 -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=cl -DCMAKE_CXX_COMPILER=cl ^
-DCMAKE_TOOLCHAIN_FILE=C:\MurniSN\Project_cpp\vcpkg-master\vcpkg-master\scripts\buildsystems\vcpkg.cmake ^
-DCMAKE_EXPORT_COMPILE_COMMANDS=1 ^
-DBUILD_DLL_ONLY=ON -Bbuilddll -DCMAKE_INSTALL_PREFIX=C:\MurniSN\Front_End\FE_MANDIRI_KD100\ServiceKD100\project_kisan\cdm_kd100\src\cpp\sockclient1 -G Ninja -S .