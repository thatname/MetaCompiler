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
#include "MetaCompileASTVisitor.h"
#include "MetaCompileAction.h"
#include "clang/AST/Type.h"

#include <iosfwd>
//namespace pe3 
//{
bool MetaCompileASTVisitor::needGenerateSerializer(clang::TagDecl * R)
{
	using namespace clang;
	if (isa<RecordDecl>(R))
	{
		if (cast<RecordDecl>(R)->isAnonymousStructOrUnion())
			return false;
	}
	if (defaultGenerate)
		for (const auto *I : R->specific_attrs<AnnotateAttr>())
		{
			if (I->getAnnotation() == "pe3_meta_off" /*|| I->getAnnotation() == "default_enumeratee"*/)
				return false;
		}
	else
		for (const auto *I : R->specific_attrs<AnnotateAttr>())
		{
			if (I->getAnnotation() == "pe3_meta_on")
				return true;
		}
	return defaultGenerate;
}
bool MetaCompileASTVisitor::needRegisterType(clang::TagDecl * R)
{
	using namespace clang;
	if (isa<RecordDecl>(R))
	{
		if (cast<RecordDecl>(R)->isAnonymousStructOrUnion())
			return false;
	}
	if (defaultGenerate)
		for (const auto *I : R->specific_attrs<AnnotateAttr>())
		{
			if (I->getAnnotation() == "pe3_meta_off"/* || I->getAnnotation() == "default_enumeratee"*/)
				return false;
		}
	else
		for (const auto *I : R->specific_attrs<AnnotateAttr>())
		{
			if (I->getAnnotation() == "pe3_meta_no_serializer")
				return true;
		}
	return defaultGenerate;
}
	int MetaCompileASTVisitor::countMember(clang::RecordDecl * R, ropeStream& gen)
	{
	using namespace clang;
		int memberCount = 0;

		
		for (DeclContext::decl_iterator C0 = R->decls_begin(); C0 != R->decls_end(); ++C0)
		{
			if(!isa<DeclaratorDecl>(*C0))
				continue;
			DeclaratorDecl* C = cast<DeclaratorDecl>(*C0);
			FieldDecl *fdecl = nullptr;
			MSPropertyDecl* mdecl = nullptr;
			if (isa<FieldDecl>(C))
			{
				fdecl = cast<FieldDecl>(C);
			}
			if (isa<MSPropertyDecl>(C))
			{
				mdecl = cast<MSPropertyDecl>(C);
			}
			if (!fdecl && (!mdecl || !mdecl->hasGetter()))
			{
				continue;
			}
			bool emitAssign = false;
			bool generateEnumeratee = true;

			
			auto typePtr = C->getType().getTypePtrOrNull();
			while (const ElaboratedType *ET = dyn_cast<ElaboratedType>(typePtr)) {
				typePtr = ET->desugar().getTypePtrOrNull();
			}
			if (!typePtr)
				continue;
			if (isa<TemplateSpecializationType>(typePtr))
			{
				const TemplateSpecializationType* t = cast<TemplateSpecializationType>(typePtr);
				auto TD = t->getTemplateName().getAsTemplateDecl()->getTemplatedDecl();
				for (const auto *I : TD->specific_attrs<AnnotateAttr>())
				{
					if (I->getAnnotation() == "pe3_meta_off")
						goto cont;
					//if (I->getAnnotation() == "pe3_meta_on")
					//	generateEnumeratee = true;
				}
			}
			if(fdecl && fdecl->isBitField()|| mdecl && mdecl->hasSetter())
			{
				emitAssign = true;
			}
			if (isa<TagType>(typePtr))
			{
				const TagType* t = cast<TagType>(typePtr);
				auto TD = t->getDecl();
				for (const auto *I : TD->specific_attrs<AnnotateAttr>())
				{
					if (I->getAnnotation() == "pe3_meta_off")
						goto cont;
					//if (I->getAnnotation() == "pe3_meta_on")
					//	generateEnumeratee = true;
				}
			}

			if (C->getAccess() == AS_protected ||
				C->getAccess() == AS_private ||
				fdecl && fdecl->isMutable()
				)
			{
				generateEnumeratee = false;
			}

			for (const auto *I : C->specific_attrs<AnnotateAttr>())
			{
				if (I->getAnnotation() == "pe3_meta_off")
					goto cont;
				if (I->getAnnotation() == "pe3_meta_on")
					generateEnumeratee = true;
			}
			if (!generateEnumeratee)
				goto cont;
			if (isa<RecordType>(typePtr))
			{
				auto decl = cast<RecordType>(typePtr)->getDecl();
				if (decl && decl->isAnonymousStructOrUnion())
				{
					memberCount += countMember(decl,gen);
					continue;
				}
			}
			goto normal;
		cont:continue;
		normal:
			memberCount++;
			std::string name = C->getName();
			//const auto& oldpolicy = R->getParentASTContext().getPrintingPolicy();
			//clang::PrintingPolicy policy = oldpolicy.LangOpts;
			std::string type;// = C->getType()./*.getTypePtrOrNull()->getCanonicalTypeInternal().*/getAsString(R->getParentASTContext().getPrintingPolicy());
			//bool realtype = ptr->isRealType();
			//if (realtype)
			type = typePtr->getCanonicalTypeInternal().getAsString(policy);
			//std::string fullname;
			//C->getNameForDiagnostic(os, R->getParentASTContext().getPrintingPolicy(), true);
			//fullname = C->->getCanonicalTypeInternal().getAsString(R->getParentASTContext().getPrintingPolicy());
			/*const Type * t = C->getType()//.getTypePtrOrNull();
			auto r = typeid(t).name();*/
			/*auto temp = C->getType();
			while (temp.getTypePtrOrNull() && temp.getTypePtrOrNull()->isPointerType())
				temp = ((PointerType*)temp.getTypePtrOrNull())->getPointeeType();
			*/
			//if (isa<TagType>(temp))
			//	_action.forwardDecl.insert(temp.getAsString(R->getParentASTContext().getPrintingPolicy()));

			gen << R"~(	if(__MEMBER_ENUMERATOR__.structMemberBegin(")~" << type << "\", \"" << name /*<< "\", " << (realtype ? "true" : "false")*/ << R"~(" ))
	{
		)~";
		if (emitAssign)
		{
			gen << "auto __data =  (&__MEMBER_ENUMERATEE_)->" << name << R"~(;
		)~";
			gen << "((&__MEMBER_ENUMERATEE_)->" << name << " = memberEnumerateRecursiveAssign";
			gen << "<__member_enumerator_type__, " << type << R"~(>::op(__MEMBER_ENUMERATOR__, __data)~";
		}
		else
		{
			gen << "(memberEnumerateRecursive";
			gen << "<__member_enumerator_type__, " << type << R"~(>::op(__MEMBER_ENUMERATOR__, (&__MEMBER_ENUMERATEE_)->)~" << name;
		}		
		gen << R"~());
		__MEMBER_ENUMERATOR__.structMemberEnd();
	}
)~";
		}
		return memberCount;
	}
	void MetaCompileASTVisitor::generateSerializer(clang::CXXRecordDecl * R)
	{
		using namespace clang;
		if (!R->hasDefinition())
			return;
		std::string name = R->getTypeForDecl()->getCanonicalTypeInternal().getAsString(policy);
		if (_action.defined.find(name) != _action.defined.end())
			return;
		_action.defined.insert(name);
		_action.forwardDecl.insert(name);
		ropeStream gen(_action.genFileContent, _action.genFileContent.size());
		/*std::string fullname;
		//llvm::raw_string_ostream os(fullname);
		//R->getNameForDiagnostic(os, R->getParentASTContext().getPrintingPolicy(), false);
		fullname = ;*/

		//int memberCount = countMember(R);
			

		//if (R->getTypeForDecl()->isUnionType() && memberCount)
		//	memberCount = 1;
		
		
		gen << R"~(pe3_MEMBER_ENUMERATORS(pe3_MEMBER_ENUMERATION_DEFINE, )~"
			<< name
			<< R"~(,
if(__MEMBER_ENUMERATOR__.structBegin(")~" << name << "\", ";
		int numberOffset = gen.offset;//insert numbers here
		gen << R"~()){
)~";// only the function body is in the macro call		
		int baseNum = 0;
		for (CXXRecordDecl::base_class_const_iterator C = R->bases_begin(); C != R->bases_end(); ++C)
		{
			auto typePtr = C->getType().getTypePtrOrNull();
			while (const ElaboratedType *ET = dyn_cast<ElaboratedType>(typePtr)) {
				typePtr = ET->desugar().getTypePtrOrNull();
			}
			if (isa<TemplateSpecializationType>(typePtr))
			{
				const TemplateSpecializationType* t = cast<TemplateSpecializationType>(typePtr);
				auto TD = t->getTemplateName().getAsTemplateDecl()->getTemplatedDecl();
				for (const auto *I : TD->specific_attrs<AnnotateAttr>())
				{
					if (I->getAnnotation() == "pe3_meta_off" /*|| I->getAnnotation() == "default_enumeratee"*/)
						goto cont1;
				}
			}
			if (isa<TagType>(typePtr))
			{
				const TagType* t = cast<TagType>(typePtr);
				
				auto TD = t->getDecl();
				for (const auto *I : TD->specific_attrs<AnnotateAttr>())
				{
					if (I->getAnnotation() == "pe3_meta_off" /*|| I->getAnnotation() == "default_enumeratee"*/)
						goto cont1;
				}
			}
		goto normal1;
		cont1:continue;
	normal1:
		++baseNum;
			std::string name = "";
			std::string type;// = C->getType()./*getTypePtrOrNull()->getCanonicalTypeInternal().*/getAsString(R->getParentASTContext().getPrintingPolicy());
			//bool realtype = C->getType().getTypePtrOrNull()->isRealType();
			//if (realtype)
			
				type = C->getType().getTypePtrOrNull()->getCanonicalTypeInternal().getAsString(policy);
			//std::string fullname;
			//C->getNameForDiagnostic(os, R->getParentASTContext().getPrintingPolicy(), true);
			//fullname = C->->getCanonicalTypeInternal().getAsString(R->getParentASTContext().getPrintingPolicy());
			/*const Type * t = C->getType()//.getTypePtrOrNull();
			auto r = typeid(t).name();*/
			/*auto temp = C->getType();
			while (temp.getTypePtrOrNull()->isPointerType())
				temp = ((PointerType*)temp.getTypePtrOrNull())->getPointeeType();
				*/
			//if (isa<TagType>(temp))
			//	_action.forwardDecl.insert(temp.getAsString(R->getParentASTContext().getPrintingPolicy()));

			gen << R"~(	if(__MEMBER_ENUMERATOR__.structMemberBegin(")~" << type << "\", \"" << name/* << "\", "<< (realtype ?"true":"false")*/<<R"~(" ))
	{
		(memberEnumerateRecursive<__member_enumerator_type__, )~"
		<< type << R"~(>::op(__MEMBER_ENUMERATOR__, ()~" << type << R"~(&)__MEMBER_ENUMERATEE_));
		__MEMBER_ENUMERATOR__.structMemberEnd();
	}
)~";
		}


		char buffer[100];
		itoa(/*R->getNumBases()*/baseNum + countMember(R,gen), buffer, 10);
		auto l = strlen(buffer);
		gen.rope.insert(numberOffset, buffer, buffer + l);
		gen.offset += l;

		gen << std::string(R"~(	__MEMBER_ENUMERATOR__.structEnd();}
//return 0;
)
)~");
	}
	static std::string hasStaticData(const clang::CXXRecordDecl *R, std::string name = "")
	{
		using namespace clang;
		for (clang::DeclContext::decl_iterator it = R->decls_begin(); it != R->decls_end(); ++it)
		{
			if (isa<NamedDecl>(*it))
			{
				DeclarationName nd = cast<NamedDecl>(*it)->getDeclName();
				if (nd.isIdentifier() && nd.getAsIdentifierInfo() && nd.getAsIdentifierInfo()->getName() == "getStaticData")
				{
					if (name.empty())
						return R->getTypeForDecl()->getCanonicalTypeInternal().getAsString(R->getParentASTContext().getPrintingPolicy());
					else
						return name;
				}
			}
		}
		for (CXXRecordDecl::base_class_const_iterator C = R->bases_begin(); C != R->bases_end(); ++C)
		{
			if (isa<TemplateSpecializationType>(C->getType().getTypePtrOrNull()))
			{
				const TemplateSpecializationType* t = cast<TemplateSpecializationType>(C->getType().getTypePtrOrNull());
				auto TD = t->getTemplateName().getAsTemplateDecl()->getTemplatedDecl();
				
				if (isa<CXXRecordDecl>(TD))
				{
					auto name = C->getType()->getCanonicalTypeInternal().getAsString(R->getParentASTContext().getPrintingPolicy());
					auto r = hasStaticData(cast<CXXRecordDecl>(TD), name);
					if(!r.empty())
						return r;
				}
			}
			if (isa<TagType>(C->getType().getTypePtrOrNull()))
			{
				const TagType* t = cast<TagType>(C->getType().getTypePtrOrNull());
				const TagDecl* TD = t->getDecl();
				if (isa<CXXRecordDecl>(TD))
				{
					auto r = hasStaticData(cast<CXXRecordDecl>(TD));
					if (!r.empty())
						return r;
				}
			}
		}
		return "";
	}
	static void hasMetaData(const clang::CXXRecordDecl *R, MetaCompileAction::typesToStoreUnit& s, MetaCompileAction& _action)
	{
		char buffer[20];
		using namespace clang;
		int start = _action.tempVarCount;
		for (const auto *I : R->specific_attrs<AnnotateAttr>())
		{
			auto b = I->getAnnotation();
			if (b.substr(0, 8) == "__META__")
			{
				std::string str = b.substr(8);

				int t = 0;
				for (; t < str.size(); t++)
				{
					if (str[t] == '(')
					{
						str[t] = '{';
						break;
					}
				}
				auto back = str.find_last_of(')');
				if (back != std::string::npos)
				{
					str[back] = '}';
				}
				
				_action.filehead << "pe3::ConstructMeta<pe3::" << str.substr(0, t) << "> __metav__" << itoa(_action.tempVarCount++, buffer, 20) << str.substr(t) << ";\n";
			}			
		}
		if (start != _action.tempVarCount)
		{
			s.hasMetaData = std::string("__metav__") + itoa(_action.tempVarCount, buffer, 20);			
			_action.filehead << "pe3::MetaBase* " << s.hasMetaData << "[] = {";
			for (int x = start; x < _action.tempVarCount; ++x)
			{
				_action.filehead << "&__metav__" << itoa(x, buffer, 20) << ",";
			}
			_action.filehead << "};\n";			
			_action.tempVarCount++;
		}
	}
	void MetaCompileASTVisitor::registerTypes(clang::CXXRecordDecl *R)
	{
		using namespace clang;
		/*for (const auto *I : R->specific_attrs<AnnotateAttr>())
		{
			if (I->getAnnotation() == "pe3_meta_off")
				return;
		}*/
		bool needSerial = needGenerateSerializer(R);
		bool needReg = needRegisterType(R);
		
		if (!needSerial && !needReg)
			return;

		//auto& ctx = R->getParentASTContext();

		std::string r = R->getTypeForDecl()->getCanonicalTypeInternal().getAsString(policy);
		auto it = _action.typesToStore.find(r);
		if (it != _action.typesToStore.end())
			return;
		auto & s = _action.typesToStore[r];
		s.shortName = R->getName();
		
		if (needSerial)
			s.hasEnumerator = "true";
		else
			s.hasEnumerator = "false";		

		if (R->isDynamicClass())
			s.isDynamic = "int";
		else
			s.isDynamic = "bool";
		s.hasStaticData = hasStaticData(R);
		hasMetaData(R, s, _action);
		if (s.dynamicNonAbstract = R->isDynamicClass() && !(s.isAbstract = R->isAbstract()))
		{
			{
				const VPtrInfoVector&  r = msContext.getVFPtrOffsets(R);
				llvm::raw_string_ostream str(s.msVTableName);
				//r[0]->NonVirtualOffset
				msMangle->mangleCXXVFTable(R, r[0]->MangledPath, str);
			}
			{
				llvm::raw_string_ostream str(s.vtableName);
				mangle->mangleCXXVTable(R, str);
			}
		}
		for (CXXRecordDecl::base_class_const_iterator C = R->bases_begin(); C != R->bases_end(); ++C)
		{
			if (isa<TemplateSpecializationType>(C->getType().getTypePtrOrNull()))
			{
				const TemplateSpecializationType* t = cast<TemplateSpecializationType>(C->getType().getTypePtrOrNull());
				auto TD = t->getTemplateName().getAsTemplateDecl()->getTemplatedDecl();
				for (const auto *I : TD->specific_attrs<AnnotateAttr>())
				{
					if (I->getAnnotation() == "pe3_meta_on" || I->getAnnotation() == "pe3_meta_no_serializer")
						goto normal;
				}
			}
			if (isa<TagType>(C->getType().getTypePtrOrNull()))
			{
				const TagType* t = cast<TagType>(C->getType().getTypePtrOrNull());
				auto TD = t->getDecl();
				for (const auto *I : TD->specific_attrs<AnnotateAttr>())
				{
					if (I->getAnnotation() == "pe3_meta_on" || I->getAnnotation() == "pe3_meta_no_serializer")
						goto normal;
				}
			}
			//goto normal;
			/*cont:*/continue;
			normal:
			std::string c = C->getType()./*getTypePtrOrNull()->getCanonicalTypeInternal().*/getAsString(R->getParentASTContext().getPrintingPolicy());
			//_action.typesToStore[c].derives.insert(r);
			_action.typesToStore[r].bases.insert(c);
		}
		/*
		typesToStore.insert(C->getType()temp.getTypePtrOrNull());*/

	}
	bool  MetaCompileASTVisitor::TraverseCXXRecordDecl(clang::CXXRecordDecl * D)//this is class or struct
	{
		using namespace clang;
		char buffer[65536];
		//if(shouldEmitSerializer)
		if (D->hasDefinition() && !D->getDescribedClassTemplate())
		{
			if (needGenerateSerializer(D))
			{
				generateSerializer(D);				
				registerTypes(D);
			}			
			else if (needRegisterType(D))
			{
				registerTypes(D);
			}
			return clang::RecursiveASTVisitor<MetaCompileASTVisitor>::TraverseCXXRecordDecl(D);
			//stringstream innerClass;

			//1st step: add base class specifier...
			SourceLocation baseLoc;
			if (D->getNumBases())
			{
				baseLoc = (D->bases_end() - 1)->getEndLoc();
				_r->InsertTextAfterToken(baseLoc, ", public TObjectContainer<");
			}
			else
			{
				baseLoc = D->getLocation();
				_r->InsertTextAfterToken(baseLoc, " : public TObjectContainer<");
			}
			_r->InsertTextAfterToken(baseLoc, D->getName());
			_r->InsertTextAfterToken(baseLoc, ">");
			baseLoc = clang::Lexer::findLocationAfterToken(baseLoc, tok::TokenKind::l_brace, _ci->getSourceManager(), _ci->getLangOpts(), false);
			//2nd step: move all member specifier into inner class			
			_r->InsertText(baseLoc, "struct _INTERNAL_DATA_{", true, true);
			for (auto t = D->field_begin(); t != D->field_end(); ++t)			//now iterate all fields..
			{
				auto endOfLine = clang::Lexer::findLocationAfterToken(t->getSourceRange().getEnd(), tok::TokenKind::semi, _ci->getSourceManager(), _ci->getLangOpts(), false);
				if (endOfLine.isValid())
				{
					SourceRange r(t->getSourceRange().getBegin(), endOfLine);
					auto s = _r->getRewrittenText(r);
					_r->InsertText(baseLoc, s, true, true);
					_r->RemoveText(r);
				}
			}
			auto tName = D->getTypeForDecl()->getCanonicalTypeInternal().getAsString(D->getParentASTContext().getPrintingPolicy());
			sprintf(buffer, "}*_internal_data_ = new _INTERNAL_DATA_;\n~%s(){delete _internal_data_;}\nstatic const _std::string& getTypeName(){static _std::string name = \"%s\";return name;}", D->getName().str().c_str(), tName.c_str());
			_r->InsertText(baseLoc, buffer, true, true);
			//3rd step: generate msproperties for members
			for (auto t = D->field_begin(); t != D->field_end(); ++t)			//now iterate all fields..
			{
				const Type * type = t->getType().getTypePtrOrNull();
				std::string vName = t->getName();
				auto tName = t->getType().getAsString(D->getParentASTContext().getPrintingPolicy());
				if (!type->isReferenceType() && !type->isRValueReferenceType())
					tName += "&";
				sprintf(buffer, "__declspec(property(get=_gEt_%s))%s %s;\n%s _gEt_%s(){return _internal_data_->%s;};\n",
					vName.c_str(), tName.c_str(), vName.c_str(),
					tName.c_str(), vName.c_str(), vName.c_str()
					);
				_r->InsertText(D->getEndLoc(), buffer, true, true);
				/*const Type * type = t->getType().getTypePtrOrNull();
				_r->InsertText(D->getLocEnd(), "__declspec(property(get=_gEt_", true, true);
				_r->InsertText(D->getLocEnd(), t->getName(), true, true);
				_r->InsertText(D->getLocEnd(), "))", true, true);
				_r->InsertText(D->getLocEnd(), t->getType().getAsString(D->getParentASTContext().getPrintingPolicy()), true, true);
				if(!type->isReferenceType() && !type->isRValueReferenceType())
				_r->InsertText(D->getLocEnd(), "& ", true, true);
				_r->InsertText(D->getLocEnd(), t->getName(), true, true);
				_r->InsertText(D->getLocEnd(), ";\n", true, true);

				_r->InsertText(D->getLocEnd(), t->getType().getAsString(D->getParentASTContext().getPrintingPolicy()), true, true);
				if(!type->isReferenceType() && !type->isRValueReferenceType())
				_r->InsertText(D->getLocEnd(), "&", true, true);
				_r->InsertText(D->getLocEnd(), "_gEt_", true, true);
				_r->InsertText(D->getLocEnd(), t->getName(), true, true);
				_r->InsertText(D->getLocEnd(), "_gEt_", true, true);
				*/
				/*auto endOfLine = clang::Lexer::findLocationAfterToken(t->getSourceRange().getEnd(), tok::TokenKind::semi, _ci->getSourceManager(), _ci->getLangOpts(), false);
				if(endOfLine.isValid())
				{
				SourceRange r (t->getSourceRange().getBegin(), endOfLine);
				auto s = _r->getRewrittenText(r);
				_r->InsertText(D->getLocEnd(), s, true, true);
				_r->RemoveText(r);
				}*/
			}
			//4th step: generate reloaders

			/*SourceLocation nextToken = Lexer::getLocForEndOfToken(D->getInnerLocStart(), -1,_ci->getSourceManager(), _ci->getLangOpts());
			nextToken = Lexer::getLocForEndOfToken(nextToken, -1,_ci->getSourceManager(), _ci->getLangOpts());*/
			//nextToken = Lexer::getLocForEndOfToken(nextToken, -1,_ci->getSourceManager(), _ci->getLangOpts());
			/*SourceLocation loc = clang::Lexer::findLocationAfterToken(D->getLocStart(), tok::TokenKind::identifier, _ci->getSourceManager(), _ci->getLangOpts(), false);
			SourceLocation loc1 = loc.getLocWithOffset(-1);

			_r->InsertText(loc1, "(inner start)", true, true);*/

			/*_r->InsertText(D->getOuterLocStart(), "(outer start)", true, true);
			_r->InsertText(D->getSourceRange().getBegin(), "rfasd1", true, true);
			_r->InsertText(D->getSourceRange().getEnd(), "rfasd2", true, true);*/
		}
		return clang::RecursiveASTVisitor<MetaCompileASTVisitor>::TraverseCXXRecordDecl(D);
	}

	bool MetaCompileASTVisitor::TraverseEnumDecl(clang::EnumDecl * R)
	{
		using namespace clang;
		if (needGenerateSerializer(R))
		{
			std::string r = R->getTypeForDecl()->getCanonicalTypeInternal().getAsString(R->getParentASTContext().getPrintingPolicy());
			auto & e = _action.enumToStore[r];
			for (auto it = R->enumerator_begin(); it != R->enumerator_end(); ++it)
			{
				for (const auto *I : it->specific_attrs<AnnotateAttr>())
				{
					if (I->getAnnotation() == "pe3_meta_off")
						goto end;
				}
				{
					MetaCompileAction::EnumToStoreUnit::enumValue v;
					v.value = (int)(*(it->getInitVal().getRawData()));
					v.name = it->getName();
					e.values.push_back(v);
				}
			end:;
			}
		}
		return  clang::RecursiveASTVisitor<MetaCompileASTVisitor>::TraverseEnumDecl(R);
	}

	//}


