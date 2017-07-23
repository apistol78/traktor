/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Core/Io/FileSystem.h>
#include <Core/Io/IStream.h>
#include <Core/Misc/CommandLine.h>
#include <Core/Log/Log.h>
#include <Xml/XmlSerializer.h>
#include "Ddc/DfnImport.h"
#include "Ddc/Lexer.h"
#include "Ddc/Parser.h"
#include "Ddc/ParseState.h"
#include "Ddc/Token.h"
#include "Ddc/CppCodeGenerator.h"

using namespace traktor;
using namespace ddc;

void *ParseAlloc(void *(*mallocProc)(size_t));
void ParseFree(void *p, void (*freeProc)(void*));
void Parse(void *yyp, int yymajor, Token* yyminor, ddc::ParseState* state);

Ref< DfnNode > parseDefinition(const Path& path)
{
	Ref< IStream > stream = FileSystem::getInstance().open(path, File::FmRead);
	if (!stream)
	{
		log::error << L"Unable to open file \"" << path.getPathName() << L"\"" << Endl;
		return 0;
	}

	void* parser = ParseAlloc(malloc);
	if (!parser)
	{
		log::error << L"Unable to allocate parser object" << Endl;
		return 0;
	}

	ParseState state;

	Ref< Lexer > lexer = new Lexer(stream);

	int32_t type;
	while ((type = lexer->next()) != Lexer::LtEndOfFile)
	{
		if (type == Lexer::LtWord)
		{
			std::wstring word = lexer->getWord();
			if (word == L"alias")
				Parse(parser, TOKEN_ALIAS, 0, &state);
			else if (word == L"import")
				Parse(parser, TOKEN_IMPORT, 0, &state);
			else if (word == L"namespace")
				Parse(parser, TOKEN_NAMESPACE, 0, &state);
			else if (word == L"class")
				Parse(parser, TOKEN_CLASS, 0, &state);
			else if (word == L"public")
				Parse(parser, TOKEN_PUBLIC, 0, &state);
			else if (word == L"private")
				Parse(parser, TOKEN_PRIVATE, 0, &state);
			else
				Parse(parser, TOKEN_LITERAL, new Token(word), &state);
		}
		else if (type == Lexer::LtString)
		{
			std::wstring str = lexer->getString();
			Parse(parser, TOKEN_STRING, new Token(str), &state);
		}
		else if (type == Lexer::LtNumber)
		{
			double number = lexer->getNumber();
			Parse(parser, TOKEN_NUMBER, new Token(number), &state);
		}
		else if (type == L'{')
			Parse(parser, TOKEN_OPEN_BRACE, 0, &state);
		else if (type == L'}')
			Parse(parser, TOKEN_CLOSE_BRACE, 0, &state);
		else if (type == L',')
			Parse(parser, TOKEN_COMMA, 0, &state);
		else if (type == L'.')
			Parse(parser, TOKEN_DOT, 0, &state);
		else if (type == L'<')
			Parse(parser, TOKEN_LESS, 0, &state);
		else if (type == L'>')
			Parse(parser, TOKEN_GREATER, 0, &state);
		else if (type == L':')
			Parse(parser, TOKEN_COLON, 0, &state);
		else if (type == L';')
			Parse(parser, TOKEN_SEMI_COLON, 0, &state);
		else if (type == L'[')
			Parse(parser, TOKEN_OPEN_BRACKET, 0, &state);
		else if (type == L']')
			Parse(parser, TOKEN_CLOSE_BRACKET, 0, &state);
	}

	Parse(parser, 0, 0, &state);

	ParseFree(parser, free);

	return state.root;
}


void resolveImports(DfnNode* node)
{
	if (DfnImport* import = dynamic_type_cast< DfnImport* >(node))
	{
		std::wstring module = import->getModule();
		Path modulePath = module + L".td";

		Ref< DfnNode > moduleNode = parseDefinition(modulePath);
		if (moduleNode)
		{
			import->setPath(modulePath);
			import->setDefinition(moduleNode);
		}
		else
			log::error << L"Unable to import \"" << module << L"\"" << Endl;
	}
	if (node->getNext())
		resolveImports(node->getNext());
}

int main(int argc, const char* argv[])
{
	CommandLine cmdLine(argc, argv);

	Path filePath = cmdLine.getString(0);
	Ref< DfnNode > node = parseDefinition(filePath);
	if (node)
	{
		Ref< IStream > file = FileSystem::getInstance().open(filePath.getPathNameNoExtension() + L".bnf", File::FmWrite);
		if (file)
		{
			xml::XmlSerializer(file).writeObject(node);
			file->close();
		}

		resolveImports(node);

		traktor::log::info << L"Compiled successfully" << Endl;

		CppCodeGenerator codeGenerator;
		codeGenerator.generate(filePath, node);

		traktor::log::info << L"Code generated successfully" << Endl;
	}
}
