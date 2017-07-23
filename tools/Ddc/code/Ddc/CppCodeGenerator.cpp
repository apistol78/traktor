/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cctype>
#include <Core/Io/FileSystem.h>
#include <Core/Io/FileOutputStream.h>
#include <Core/Io/AnsiEncoding.h>
#include <Core/Log/Log.h>
#include <Core/Misc/String.h>
#include <Core/Guid.h>
#include "Ddc/CppCodeGenerator.h"
#include "Ddc/DfnAlias.h"
#include "Ddc/DfnClass.h"
#include "Ddc/DfnImport.h"
#include "Ddc/DfnMember.h"
#include "Ddc/DfnNamespace.h"
#include "Ddc/DfnType.h"

using namespace traktor;

namespace ddc
{
	namespace
	{

/*! \brief Generate map with qualified class names. */
void generateQualifiedClassNames(const DfnNode* node, const std::wstring& qualifiedNamespace, std::map< const DfnClass*, std::wstring >& outQualifiedClassNames)
{
	if (const DfnClass* classNode = dynamic_type_cast< const DfnClass* >(node))
	{
		std::wstring className =
			qualifiedNamespace.empty() ?
			classNode->getName() :
			qualifiedNamespace + L"." + classNode->getName();

		outQualifiedClassNames[classNode] = className;
	}
	else if (const DfnImport* importNode = dynamic_type_cast< const DfnImport* >(node))
	{
		if (importNode->getDefinition())
			generateQualifiedClassNames(importNode->getDefinition(), qualifiedNamespace, outQualifiedClassNames);
	}
	else if (const DfnNamespace* namespaceNode = dynamic_type_cast< const DfnNamespace* >(node))
	{
		if (namespaceNode->getStatements())
		{
			std::wstring ns = 
				qualifiedNamespace.empty() ? 
				namespaceNode->getName() :
				qualifiedNamespace + L"." + namespaceNode->getName();
			generateQualifiedClassNames(namespaceNode->getStatements(), ns, outQualifiedClassNames);
		}
	}

	if (node->getNext())
		generateQualifiedClassNames(node->getNext(), qualifiedNamespace, outQualifiedClassNames);
}

/*! Find matching class from concrete type. */
const DfnClass* findClass(const DfnNode* node, const std::wstring& qualifiedNamespace, const DfnType* type)
{
	const DfnClass* foundClassNode = 0;

	if (const DfnClass* classNode = dynamic_type_cast< const DfnClass* >(node))
	{
		std::wstring className =
			qualifiedNamespace.empty() ?
			classNode->getName() :
			qualifiedNamespace + L"." + classNode->getName();

		if (className == type->getName())
			foundClassNode = classNode;
	}
	else if (const DfnImport* importNode = dynamic_type_cast< const DfnImport* >(node))
	{
		if (importNode->getDefinition())
			foundClassNode = findClass(importNode->getDefinition(), qualifiedNamespace, type);
	}
	else if (const DfnNamespace* namespaceNode = dynamic_type_cast< const DfnNamespace* >(node))
	{
		if (namespaceNode->getStatements())
		{
			std::wstring ns = 
				qualifiedNamespace.empty() ? 
				namespaceNode->getName() :
			qualifiedNamespace + L"." + namespaceNode->getName();
			foundClassNode = findClass(namespaceNode->getStatements(), ns, type);
		}
	}

	if (!foundClassNode && node->getNext())
		foundClassNode = findClass(node->getNext(), qualifiedNamespace, type);

	return foundClassNode;
}

/*! Find matching alias from concrete type. */
const DfnAlias* findAlias(const DfnNode* node, const std::wstring& qualifiedNamespace, const DfnType* type)
{
	const DfnAlias* foundAliasNode = 0;

	if (const DfnAlias* aliasNode = dynamic_type_cast< const DfnAlias* >(node))
	{
		if (compareIgnoreCase< std::wstring >(aliasNode->getLanguage(), L"cpp") == 0)
		{
			const DfnType* aliasType = checked_type_cast< const DfnType*, false >(aliasNode->getType());

			std::wstring aliasTypeName =
				qualifiedNamespace.empty() ?
				aliasType->getName() :
				qualifiedNamespace + L"." + aliasType->getName();

			if (aliasTypeName == type->getName() && aliasType->isArray() == type->isArray())
			{
				const DfnType* aliasTypeSubst = checked_type_cast< const DfnType* >(aliasType->getSubst());
				const DfnType* typeSubst = checked_type_cast< const DfnType* >(type->getSubst());

				while (aliasTypeSubst && typeSubst)
				{
					aliasTypeSubst = checked_type_cast< const DfnType* >(aliasTypeSubst->getNext());
					typeSubst = checked_type_cast< const DfnType* >(typeSubst->getNext());
				}

				if ((aliasTypeSubst != 0) == (typeSubst != 0))
					foundAliasNode = aliasNode;
			}
		}
	}
	else if (const DfnImport* importNode = dynamic_type_cast< const DfnImport* >(node))
	{
		if (importNode->getDefinition())
			foundAliasNode = findAlias(importNode->getDefinition(), qualifiedNamespace, type);
	}
	else if (const DfnNamespace* namespaceNode = dynamic_type_cast< const DfnNamespace* >(node))
	{
		if (namespaceNode->getStatements())
		{
			std::wstring ns = 
				qualifiedNamespace.empty() ? 
				namespaceNode->getName() :
				qualifiedNamespace + L"." + namespaceNode->getName();
			foundAliasNode = findAlias(namespaceNode->getStatements(), ns, type);
		}
	}

	if (!foundAliasNode && node->getNext())
		foundAliasNode = findAlias(node->getNext(), qualifiedNamespace, type);

	return foundAliasNode;
}

/*! Resolve language specific type from data member. */
bool resolveLanguageType(
	const DfnNode* program,
	const DfnMember* member,
	const DfnType* type,
	const std::wstring& (DfnAlias::*aliasGet)() const,
	std::wstring& out
)
{
	const DfnClass* classNode = findClass(program, L"", type);
	const DfnAlias* aliasNode = findAlias(program, L"", type);

	if (classNode)
	{
		out = classNode->getName();
		return true;
	}
	else if (aliasNode)
	{
		const DfnType* aliasType = checked_type_cast< const DfnType*, false >(aliasNode->getType());
		StringOutputStream ss;

		// Get list of alias substitute variables.
		std::vector< std::pair< std::wstring, int32_t > > aliasSubstIndices;
		int32_t aliasSubstIndex = 0;
		for (const DfnNode* aliasSubstNode = aliasType->getSubst(); aliasSubstNode; aliasSubstNode = aliasSubstNode->getNext())
		{
			const DfnType* aliasSubstType = checked_type_cast< const DfnType*, false >(aliasSubstNode);
			aliasSubstIndices.push_back(std::make_pair(
				aliasSubstType->getName(),
				aliasSubstIndex++
			));
		}
		aliasSubstIndices.push_back(std::make_pair(
			L"name",
			-1
		));

		// Sort variables; longest first.
		struct SortAliasSubstPredicate
		{
			bool operator () (const std::pair< std::wstring, int32_t >& a, const std::pair< std::wstring, int32_t >& b) const
			{
				return a.first.length() > b.first.length();
			}
		};
		std::sort(aliasSubstIndices.begin(), aliasSubstIndices.end(), SortAliasSubstPredicate());

		// Resolve substitute from type implementation.
		std::wstring id = (aliasNode->*aliasGet)();
		for (size_t i = 0; i < id.length(); )
		{
			if (id[i] == L'%')
			{
				size_t s = ++i;
				size_t e = s;

				// Find index of substitute; able to exit early as list is sorted on longest.
				int32_t aliasSubstIndex;
				for (size_t j = 0; j < aliasSubstIndices.size(); ++j)
				{
					if (startsWith(id.substr(s), aliasSubstIndices[j].first))
					{
						aliasSubstIndex = aliasSubstIndices[j].second;
						e = s + aliasSubstIndices[j].first.length();
						break;
					}
				}
				if (e <= s)
					return false;

				if (aliasSubstIndex == -1)
					ss << member->getName();
				else
				{
					// Get type implementation from index.
					const DfnNode* typeSubstNode = type->getSubst();
					while (typeSubstNode)
					{
						if (aliasSubstIndex-- <= 0)
							break;
						typeSubstNode = typeSubstNode->getNext();
					}
					if (!typeSubstNode)
						return false;

					// Parse accessor.
					aliasGet = &DfnAlias::getLanguageType;
					if (id[e] == L'.')
					{
						if (startsWith< std::wstring >(id.substr(e), L".type"))
						{
							aliasGet = &DfnAlias::getLanguageType;
							e += 5;
						}
						else if (startsWith< std::wstring >(id.substr(e), L".argument"))
						{
							aliasGet = &DfnAlias::getLanguageArgType;
							e += 9;
						}
						else if (startsWith< std::wstring >(id.substr(e), L".member"))
						{
							aliasGet = &DfnAlias::getLanguageMember;
							e += 7;
						}
						else
							return false;
					}

					// Recursively resolve type name.
					std::wstring resolvedId;
					if (!resolveLanguageType(
						program,
						member,
						checked_type_cast< const DfnType*, false >(typeSubstNode),
						aliasGet,
						resolvedId
					))
						return false;

					ss << resolvedId;
				}

				i = e;
			}
			else
				ss << id[i++];
		}

		out = ss.str();
	}
	else
	{
		traktor::log::error << L"Unable to find matching class nor language alias from type \"" << type->getName() << L"\"" << Endl;
		return false;
	}

	return true;
}

bool resolveCppTypeName(const DfnNode* program, const DfnMember* member, std::wstring& outTypeName)
{
	const DfnType* type = checked_type_cast< const DfnType*, false >(member->getMemberType());
	return resolveLanguageType(program, member, type, &DfnAlias::getLanguageType, outTypeName);
}

bool resolveCppArgumentTypeName(const DfnNode* program, const DfnMember* member, std::wstring& outTypeName)
{
	const DfnType* type = checked_type_cast< const DfnType*, false >(member->getMemberType());
	return resolveLanguageType(program, member, type, &DfnAlias::getLanguageArgType, outTypeName);
}

bool resolveCppSerializationMember(const DfnNode* program, const DfnMember* member, std::wstring& outMember)
{
	const DfnType* type = checked_type_cast< const DfnType*, false >(member->getMemberType());
	return resolveLanguageType(program, member, type, &DfnAlias::getLanguageMember, outMember);
}

std::wstring getCppMemberName(const std::wstring& name)
{
	return L"m_" + name;
}

std::wstring getCppMethodName(const std::wstring& name)
{
	std::wstring tmp = name; tmp[0] = towupper(tmp[0]);
	return tmp;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"ddc.CppCodeGenerator", CppCodeGenerator, ICodeGenerator)

bool CppCodeGenerator::generate(const Path& sourceFilePath, const DfnNode* node) const
{
	log::info << L"Generating C++ header..." << Endl;
	if (!generateHeader(sourceFilePath, node))
		return false;

	log::info << L"Generating C++ source..." << Endl;
	if (!generateSource(sourceFilePath, node))
		return false;

	return true;
}

bool CppCodeGenerator::generateHeader(const Path& sourceFilePath, const DfnNode* node) const
{
	Path headerPath = sourceFilePath.getPathNameNoExtension() + L".h";

	std::wstring includeGuard = Guid::create().format();
	includeGuard = replaceAll(includeGuard, '{', '_');
	includeGuard = replaceAll(includeGuard, '}', '_');
	includeGuard = replaceAll(includeGuard, '-', '_');
	includeGuard = L"INCLUDE" + includeGuard + L"H";

	Ref< IStream > file = FileSystem::getInstance().open(headerPath, File::FmWrite);
	FileOutputStream os(file, new AnsiEncoding());

	os << L"// THIS FILE IS AUTOMATICALLY GENERATED, DO NOT EDIT!" << Endl;
	os << Endl;
	os << L"#ifndef " << includeGuard << Endl;
	os << L"#define " << includeGuard << Endl;
	os << Endl;

	os << L"#include \"Core/Serialization/ISerializable.h\"" << Endl;
	os << Endl;
	
	generateHeader(os, node, node);

	os << Endl;
	os << L"#endif	// " << includeGuard << Endl;

	os.close();

	return true;
}

bool CppCodeGenerator::generateHeader(OutputStream& os, const DfnNode* programNode, const DfnNode* node) const
{
	if (!node)
		return true;

	if (const DfnNamespace* namespaceNode = dynamic_type_cast< const DfnNamespace* >(node))
	{
		os << L"namespace " << namespaceNode->getName() << Endl;
		os << L"{" << Endl;
		os << Endl;

		generateHeader(os, programNode, namespaceNode->getStatements());

		os << L"}" << Endl;
	}
	else if (const DfnClass* classNode = dynamic_type_cast< const DfnClass* >(node))
	{
		std::wstring superClassName = classNode->getSuper();
		if (!superClassName.empty())
		{
			// \fixme Translate into C++
		}
		else
			superClassName = L"traktor::ISerializable";

		os << L"class " << classNode->getName() << L" : public " << superClassName << Endl;
		os << L"{" << Endl;
		os << L"\tT_RTTI_CLASS;" << Endl;
		os << Endl;
		os << L"public:" << Endl;

		for (const DfnNode* memberIter = classNode->getMembers(); memberIter; memberIter = memberIter->getNext())
		{
			const DfnMember* memberNode = dynamic_type_cast< const DfnMember* >(memberIter);
			if (memberNode)
			{
				std::wstring typeName;
				if (resolveCppArgumentTypeName(programNode, memberNode, typeName))
				{
					std::wstring memberName = memberNode->getName();

					os << L"\tvoid set" << getCppMethodName(memberName) << L"(" << typeName << L" " << memberName << L") {" << Endl;
					os << L"\t\t" << getCppMemberName(memberName) << L" = " << memberName << L";" << Endl;
					os << L"\t}" << Endl;
					os << Endl;

					os << L"\t" << typeName << L" get" << getCppMethodName(memberName) << L"() const {" << Endl;
					os << L"\t\treturn " << getCppMemberName(memberName) << L";" << Endl;
					os << L"\t}" << Endl;
					os << Endl;
				}
			}
		}

		os << L"\tvirtual bool serialize(traktor::ISerializer& s);" << Endl;
		os << Endl;

		os << L"private:" << Endl;

		for (const DfnNode* memberIter = classNode->getMembers(); memberIter; memberIter = memberIter->getNext())
		{
			const DfnMember* memberNode = dynamic_type_cast< const DfnMember* >(memberIter);
			if (memberNode)
			{
				std::wstring typeName;	
				if (resolveCppTypeName(programNode, memberNode, typeName))
				{
					std::wstring memberName = memberNode->getName();
					os << L"\t" << typeName << L" " << getCppMemberName(memberName) << L";" << Endl;
				}
				else
				{
					traktor::log::error << L"Unable to find C++ type matching data type of member \"" << memberNode->getName() << L"\"" << Endl;
					return false;
				}
			}
		}

		os << L"};" << Endl;
		os << Endl;
	}

	generateHeader(os, programNode, node->getNext());
	return true;
}

bool CppCodeGenerator::generateSource(const Path& sourceFilePath, const DfnNode* node) const
{
	Path sourcePath = sourceFilePath.getPathNameNoExtension() + L".cpp";
	Path headerPath = sourceFilePath.getPathNameNoExtension() + L".h";

	Ref< IStream > file = FileSystem::getInstance().open(sourcePath, File::FmWrite);
	FileOutputStream os(file, new AnsiEncoding());

	os << L"// THIS FILE IS AUTOMATICALLY GENERATED, DO NOT EDIT!" << Endl;
	os << Endl;
	os << L"#include \"Core/Serialization/ISerializer.h\"" << Endl;
	os << L"#include \"Core/Serialization/Member.h\"" << Endl;
	os << L"#include \"" << headerPath.getPathName() << L"\"" << Endl;
	os << Endl;

	generateSource(os, node, node);

	os << Endl;
	os.close();

	return true;
}

bool CppCodeGenerator::generateSource(OutputStream& os, const DfnNode* programNode, const DfnNode* node) const
{
	if (!node)
		return true;

	if (const DfnNamespace* namespaceNode = dynamic_type_cast< const DfnNamespace* >(node))
	{
		os << L"namespace " << namespaceNode->getName() << Endl;
		os << L"{" << Endl;
		os << Endl;

		generateSource(os, programNode, namespaceNode->getStatements());

		os << L"}" << Endl;
	}
	else if (const DfnClass* classNode = dynamic_type_cast< const DfnClass* >(node))
	{
		std::wstring superClassName = classNode->getSuper();
		if (!superClassName.empty())
		{
			// \fixme Translate into C++
		}
		else
			superClassName = L"traktor::ISerializable";

		os << L"T_IMPLEMENT_RTTI_FACTORY_CLASS(\"" << classNode->getName() << L"\", 0, " << classNode->getName() << L", " << superClassName << L")" << Endl;
		os << Endl;

		os << L"bool " << classNode->getName() << L"::serialize(traktor::ISerializer& s)" << Endl;
		os << L"{" << Endl;

		for (const DfnNode* memberIter = classNode->getMembers(); memberIter; memberIter = memberIter->getNext())
		{
			const DfnMember* memberNode = dynamic_type_cast< const DfnMember* >(memberIter);
			if (memberNode)
			{
				std::wstring serializationMember;
				if (resolveCppSerializationMember(programNode, memberNode, serializationMember))
					os << L"\ts >> " << serializationMember << L"(\"" << memberNode->getName() << L"\", " << getCppMemberName(memberNode->getName()) << L");" << Endl;
			}
		}

		os << L"\treturn true;" << Endl;
		os << L"}" << Endl;
		os << Endl;
	}

	generateSource(os, programNode, node->getNext());
	return true;
}

}
