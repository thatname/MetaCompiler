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
#include "MetaCompileAction.h"
#include "llvm/Support/CommandLine.h"
#include <algorithm>
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"
extern llvm::cl::opt<std::string> defaultGenerateDir;

char myTolower(char in) {
	if (in <= 'Z' && in >= 'A')
		return in - ('Z' - 'z');
	if (in == '\\')
		return '/';
	return in;
}
using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;
extern cl::OptionCategory ClangCheckCategory;
cl::list<std::string> sourceDirs(
	"source-dirs",
	cl::desc("specifie the sources"),
	cl::cat(ClangCheckCategory), cl::CommaSeparated);
bool MetaCompileASTConsumer::HandleTopLevelDecl(clang::DeclGroupRef DR)
{
	using namespace clang;
	for (DeclGroupRef::iterator b = DR.begin(), e = DR.end();b != e; ++b)
	{
		SourceRange range = (*b)->getSourceRange();
		StringRef name = _ci->getSourceManager().getFilename(range.getBegin());

		if (!defaultGenerateDir.empty() && name.substr(0, defaultGenerateDir.size()) == defaultGenerateDir)
			_v.defaultGenerate = true;
		else
			_v.defaultGenerate = false;
		for (auto &WorkingDir : sourceDirs)
		{
			//auto WorkingDir = _ci->getFileSystemOpts().WorkingDir;
			std::string str = name.substr(0, WorkingDir.size());

			std::transform(str.begin(), str.end(), str.begin(), myTolower);
			std::transform(WorkingDir.begin(), WorkingDir.end(), WorkingDir.begin(), myTolower);
			if (str == WorkingDir && name.find(".gen.") == StringRef::npos)
			{
				_v.TraverseDecl(*b);
				break;
			}
		}
	}
	return true;
}


