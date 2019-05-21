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
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/Lexer.h"
#include "clang/AST/DeclBase.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Mangle.h"
#include "clang/AST/VTableBuilder.h"
#include "clang/Rewrite/Core/RewriteRope.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "ropeStream.h"

//#include <iosfwd>

struct MetaCompileAction;
// For each source file provided to the tool, a new FrontendAction is created.
class MetaCompileASTVisitor : public clang::RecursiveASTVisitor<MetaCompileASTVisitor>
{
public:
	//typedef RecursiveASTVisitor<MyASTVisitor> base;
	MetaCompileASTVisitor(clang::Rewriter* r, clang::CompilerInstance * ci, MetaCompileAction& action)
		:_r(r), _ci(ci), _action(action), policy(_ci->getLangOpts())
		, msMangle(clang::MicrosoftMangleContext::create(_ci->getASTContext(), _ci->getDiagnostics()))
		, mangle(clang::ItaniumMangleContext::create(_ci->getASTContext(), _ci->getDiagnostics()))
		, msContext(_ci->getASTContext())
	{
	}
	~MetaCompileASTVisitor()
	{
		delete msMangle;
		delete mangle;
	}
	MetaCompileAction& _action;
	bool defaultGenerate = false;
	bool needGenerateSerializer(clang::TagDecl * D);
	bool needRegisterType(clang::TagDecl * D);
	bool  TraverseCXXRecordDecl(clang::CXXRecordDecl * D);
	bool  TraverseEnumDecl(clang::EnumDecl * D);
	void registerTypes(clang::CXXRecordDecl * D);
	int countMember(clang::RecordDecl * R,ropeStream & gen);
	void generateSerializer(clang::CXXRecordDecl * R);
	clang::Rewriter* _r;
	clang::CompilerInstance * _ci;
	clang::PrintingPolicy policy;
	clang::MicrosoftVTableContext msContext;
	clang::MicrosoftMangleContext *msMangle;
	clang::ItaniumMangleContext* mangle;
};