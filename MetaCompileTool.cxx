/* BSD 2-Clause License
** 
** Copyright (c) 2019, ZHOU He
** All rights reserved.
** 
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
** 
** 1. Redistributions of source code must retain the above copyright notice, this
**    list of conditions and the following disclaimer.
** 
** 2. Redistributions in binary form must reproduce the above copyright notice,
**    this list of conditions and the following disclaimer in the documentation
**    and/or other materials provided with the distribution.
** 
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
** DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
** SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
** CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
** OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS
#include <cstdio>
#include <string>
#include <sstream>
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"
#include "MetaCompileAction.h"

using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;
cl::OptionCategory MetaCompilerCategory("metaCompiler");

cl::opt<std::string> GeneratedFileName(
	"generated-file-name",
	cl::desc("specifie the generated file name without extension"),
	cl::cat(MetaCompilerCategory));

cl::opt<std::string> defaultGenerateDir(
	"default-generate-dir",
	cl::desc("specifie the dir contains all file defaulted to generate serializer"),
	cl::cat(MetaCompilerCategory));

cl::opt<std::string> projectDir(
	"project-dir",
	cl::desc("specifie the project dir"),
	cl::cat(MetaCompilerCategory));



//#ifdef _WIN32
//#include <windows.h>
//cl::opt<bool> popupModale(
//	"popup-modale",
//	cl::desc("show popup modale"),
//	cl::cat(MetaCompilerCategory));
//#endif
//test option like this :"d:/test.h -- -x c++"
class MyOoptionParser : public CommonOptionsParser
{

};
int main(int argc, const char **argv) {
	auto optionParser = CommonOptionsParser::create(
		argc, argv, MetaCompilerCategory, llvm::cl::OneOrMore,
		"Generating reflection metadata.");
	if (optionParser)
	{
		ClangTool Tool(optionParser->getCompilations(), optionParser->getSourcePathList());
		int result = Tool.run(newFrontendActionFactory<MetaCompileAction>().get());
	}
	return 0;
	//return result;
}