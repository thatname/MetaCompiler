# MetaCompiler
This is the meta compiler for Peace Engine 3.
It generates serialization and reflection code for C++ code automatically.
## Build
### Windows
The following steps demonstrate how to use command line cmake and MSBuild to build metaCompiler.exe
Note that you can also use cmake-gui.exe instead of command line cmake to generate the VS solutions,
In addition you can open the generated .sln with Visual studio to build and debug them.

1. Clone and build LLVM.

   git clone https://github.com/llvm/llvm-project.git
   mkdir llvm-build
   cd llvm-build
   cmake ../llvm-project/llvm -DLLVM_EXTERNAL_CLANG_SOURCE_DIR=../llvm-project/clang -DCMAKE_BUILD_TYPE="RelWithDebInfo"
   cmake --build . --config "RelWithDebInfo"
   cd ..

2. Clone and build MetaCompiler

   git clone https://github.com/thatname/MetaCompiler.git
   mkdir MetaCompiler-build
   cd MetaCompiler-build
   cmake ../MetaCompiler -DLLVM_GIT=../llvm-project -DLLVM_BUILD=../llvm-build -DCMAKE_BUILD_TYPE="RelWithDebInfo"
   cmake --build . --config "RelWithDebInfo"
   


### Other OS
   TBD

   The output metaCompiler executable is placed in MetaCompiler-build/meta-exe/<OS-Name>. 
   E.g. if you are using Windows, it's MetaCompiler-build/meta-exe/Windows/metaCompiler.exe

## LICENSE
MetaCompiler is BSD Licensed, You can use it for commercial purposes. See LICENSE for details.
