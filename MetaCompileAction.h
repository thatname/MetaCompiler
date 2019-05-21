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
#pragma once
#include "clang/Frontend/FrontendActions.h"
#include "MetaCompileASTConsumer.h"
#include "clang/Frontend/CompilerInstance.h"
#include "llvm/Support/CommandLine.h"
#include "ropeStream.h"
#include <thread>
#include <chrono>

// For each source file provided to the tool, a new FrontendAction is created.
extern llvm::cl::opt<std::string> GeneratedFileName;
extern llvm::cl::opt<std::string> projectDir;
struct MetaCompileAction : public clang::ASTFrontendAction {
public:
	MetaCompileAction():filehead(genFileContent, 0) , header(genHeaderContent,0)
	{
		/*generatedFileName = GeneratedFileName;
		if (generatedFileName.empty())
			generatedFileName = "gen";*/
		header << "#pragma once\n";
		//header << "#ifndef pe3METACOMPILE\n";
		header << "#include <pe3Object.h>\n";
		//filehead << "#include <pe3Object_inl.h>\n";
		filehead << "#pragma warning(disable:4483)\n";
		filehead << "#include <pe3.gen.h>\n";		
		//filehead << "using namespace pe3;\n";
	}
	std::string workingDir;
	~MetaCompileAction()
	{
		generateHead();
		//ropeStream(genHeaderContent, genHeaderContent.size()) << "#endif";
		using namespace std::chrono_literals;
		{
			FILE* fp;
			printf("writing:%s\n", (projectDir + "/pe3.gen.h").c_str());
			while(!(fp = fopen((projectDir + "/pe3.gen.h").c_str(), "w")))
				std::this_thread::sleep_for(1s);
			for (auto I = genHeaderContent.begin(), E = genHeaderContent.end(); I != E; I.MoveToNextPiece())
				fwrite(I.piece().begin(), 1, I.piece().size(), fp);
			fclose(fp);
		}
		{			
			FILE* fp;
			printf("writing:%s\n", (projectDir + "/" + GeneratedFileName + ".gen.cxx").c_str());
			while (!(fp = fopen((projectDir + "/" + GeneratedFileName + ".gen.cxx").c_str(), "w")))
				std::this_thread::sleep_for(1s);
			for (auto I = genFileContent.begin(), E = genFileContent.end(); I != E; I.MoveToNextPiece())
				fwrite(I.piece().begin(), 1, I.piece().size(), fp);
			//llvm::errs() << I.piece();
			fclose(fp);
		}
	}
	clang::RewriteRope genHeaderContent;
	ropeStream header;
	clang::RewriteRope genFileContent;
	ropeStream filehead;
	std::set<std::string>forwardDecl;// , dontforwardDecl;
	std::set<std::string>defined;
	struct typesToStoreUnit
	{
		std::string shortName, msVTableName, vtableName;
		const char* isDynamic = "bool";
		bool dynamicNonAbstract;
		bool isAbstract;
		std::string hasStaticData;
		const char* hasEnumerator = "false";
		std::set<std::string> derives;
		std::set<std::string> bases;
		std::string hasMetaData = "nullptr";
	};
	struct EnumToStoreUnit
	{
		struct enumValue
		{
			std::string name;
			int value;
			int offset;
		};
		std::vector<enumValue>values;
	};
	std::map<std::string, EnumToStoreUnit>enumToStore;
	std::map<std::string, typesToStoreUnit>typesToStore;
	int tempVarCount = 0;
	void generateHead()
	{
		header << "namespace pe3 {\n""extern void init_reflection_"/* << GeneratedFileName << */"();";
		filehead << "namespace pe3 {\n";
		for (auto& d : enumToStore)
		{
			header << "template<> struct EnumDescTmpl<" << d.first << ">{static EnumDescType desc;};\n";
			header << "template<typename S> struct memberEnumerateRecursive<S, " << d.first << ">{static void op(S& s, " << d.first << "& t){t = (" << d.first << ")s.processEnum((int)t, EnumDescTmpl<" << d.first << ">::desc);}};\n";
			filehead << " EnumDescType EnumDescTmpl<" << d.first << ">::desc={\"" << d.first << "\",\"";
			int offset = 0;
			for (auto & e : d.second.values)
			{
				filehead << e.name << "\\0";
				e.offset = offset;
				offset += e.name.size() + 1;
			}
			filehead << "\\0\"};\n";
		}
		for (auto& d : typesToStore)
		{/*
			char derivesCountBuffer[128];
			itoa(d.second.derives.size(), derivesCountBuffer, 10);
			char derivesCountBuffer_1[128];
			itoa(d.second.derives.size()+1, derivesCountBuffer_1, 10);
			*/
			char basesCountBuffer[128];
			itoa(d.second.bases.size(), basesCountBuffer, 10);
			/*char basesCountBuffer_1[128];
			itoa(d.second.bases.size(), basesCountBuffer_1, 10);*/
			
			header << "template<> struct ClassDescTmpl<" << d.first << ">{static ClassDescType desc; typedef " << d.second.isDynamic << " isDynamic; ";
			if (!d.second.bases.empty())
				header << "static BaseLink bases[" << basesCountBuffer << "];";
			header << "};\n";
			/*filehead << " ClassDescType* ClassDescTmpl<" << d.first << ">::derives[" << derivesCountBuffer_1 << "] = {";
			for (auto& t : d.second.derives)
			{
				filehead << "&ClassDescTmpl<" << t << ">::desc,\n";
			}
			filehead << "nullptr};";*/



			filehead << " ClassDescType ClassDescTmpl<" << d.first << ">::desc = {sizeof(" << d.first << "), ";

			filehead << basesCountBuffer << "," << d.second.hasEnumerator << ", \"" << d.first << "\",\"" << d.second.shortName << "\", nullptr, pe3_MEMBER_ENUMERATORS(__MEMBER_ENUMERATOR_OP_DEFINE," << d.first << ", void)";
			if(d.second.bases.empty())
				filehead << "nullptr,";
			else
				filehead << "ClassDescTmpl<" << d.first << ">::bases,";

			if (d.second.isAbstract)
				filehead << "nullptr, nullptr";
			else
			{
				std::string trueName;
				auto r = d.first.find_last_of(':');
				if (r != std::string::npos)
					trueName = d.first.substr(r + 1);
				else
					trueName = d.first;

				filehead << "[](void* ptr){new (ptr)" << d.first << ";}, [](void* ptr){((" << d.first << "*)ptr)->~" << trueName << "();}";
			}
			if (!d.second.hasStaticData.empty())
				filehead << "," << /*d.first*/d.second.hasStaticData << "::getStaticData(),";
			else
				filehead << ",nullptr,";
			//now the meta data.			
			filehead << d.second.hasMetaData <<"};\n";
		}
		for (auto& d : typesToStore)
		{
			char basesCountBuffer[128];
			itoa(d.second.bases.size(), basesCountBuffer, 10);
			if (!d.second.bases.empty())
			{
				filehead << " BaseLink ClassDescTmpl<" << d.first << ">::bases[" << basesCountBuffer << "] = {";
				int n = d.second.bases.size();
				for (auto& t : d.second.bases)
				{
					filehead << "{&ClassDescTmpl<" << t << ">::desc, &ClassDescTmpl<" << d.first << ">::desc}";
					if (--n)
						filehead << ",";
				}
				filehead << "};\n";
			}
		}

		filehead << "void init_reflection_"/* << GeneratedFileName <<*/ "(){\n";

		for (auto& d : enumToStore)
		{
			for (auto& v : d.second.values)
			{
				char basesCountBuffer[128];
				itoa(v.value, basesCountBuffer, 10);
				char offsetBuffer[128];
				itoa(v.offset, offsetBuffer, 10);
				filehead << "EnumDescTmpl<" << d.first << ">::desc.ValueToName[" << basesCountBuffer << "]=EnumDescTmpl<" << d.first << ">::desc.enumNameList + "<< offsetBuffer << ";\n";
				filehead << "EnumDescTmpl<" << d.first << ">::desc.NameToValue[EnumDescTmpl<" << d.first << ">::desc.enumNameList + "<< offsetBuffer << "]=" << basesCountBuffer << ";\n";
			}
		}
		/*
		filehead << R"~(	
			static struct __type_map_initializer__t_{__type_map_initializer__t_(){)~";*/
		for (auto& d : typesToStore)
		{
			filehead << "classDescByName[\"" << d.first << "\"]=&ClassDescTmpl<" << d.first << ">::desc;\n";
			if (d.second.dynamicNonAbstract)
			{
				filehead << "{\n#ifdef WIN32\nvoid *dummy = (void*)__identifier(\"" << d.second.msVTableName << "\");\n#else\nextern\"C\"void*" << d.second.vtableName << ";void*dummy= " << d.second.vtableName << ";\n#endif\n"
						<< "\n ClassDescTmpl<" << d.first << ">::desc.vptr = dummy;\n";
				filehead << "classDescByVptr[dummy]=&ClassDescTmpl<" << d.first << ">::desc;}\n";
			}
		}
		filehead << "}\n";
		/*filehead << "}} __type_map_initializer__;\n";*/
		for(auto& d : forwardDecl)
			//if (dontforwardDecl.find(d) == dontforwardDecl.end())
			{
				header << R"~(pe3_MEMBER_ENUMERATORS(pe3_MEMBER_ENUMERATION_DECLARE,)~" << d << R"~(, void)
)~";
			}
		header << "}\n";
		ropeStream gen(genFileContent, genFileContent.size());
		gen << "}";
	}
	 void EndSourceFileAction() override {
	  using namespace clang;
    SourceManager &SM = TheRewriter.getSourceMgr();
    llvm::errs() << "** EndSourceFileAction for: "
                 << SM.getFileEntryForID(SM.getMainFileID())->getName() << "\n";
	return;
    TheRewriter.getEditBuffer(SM.getMainFileID()).write(llvm::errs());
  }
		std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &CI,
		llvm::StringRef file) override {
			workingDir = CI.getFileSystemOpts().WorkingDir;
			using namespace clang;
			//auto last = file.substr(file.find_last_of('/')+1);
			header << "#include <" << file.substr(std::min(file.find_last_of('/'), file.find_last_of('\\')) + 1)/*file.substr(CI.getFileSystemOpts().WorkingDir.size()) */<< ">\n";

			TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
			return llvm::make_unique<MetaCompileASTConsumer>(&TheRewriter, &CI, /*file,*/ *this);
			//return new MetaCompileASTConsumer(&TheRewriter, &CI, file);
	}

	clang::Rewriter TheRewriter;
};