#include <Core/Io/FileSystem.h>
#include <Core/Misc/CommandLine.h>
#include <Core/Log/Log.h>
#include "Ddc/Parser.h"
#include "Ddc/Token.h"
#include "Ddc/CppCodeGenerator.h"

using namespace traktor;
using namespace ddc;

Ref< DfnNode > g_resultNode;

void *ParseAlloc(void *(*mallocProc)(size_t));
void ParseFree(void *p, void (*freeProc)(void*));
void Parse(void *yyp, int yymajor, Token* yyminor);

int main(int argc, const char* argv[])
{
	CommandLine cmdLine(argc, argv);

	void* parser = ParseAlloc(malloc);
	if (!parser)
	{
		log::error << L"Unable to allocate parser object" << Endl;
		return 0;
	}

	Parse(parser, TOKEN_NAMESPACE, 0);
	Token t0(L"traktor");
	Parse(parser, TOKEN_LITERAL, &t0);
	Parse(parser, TOKEN_OPEN_BRACE, 0);

	Parse(parser, TOKEN_CLASS, 0);
	Token t1(L"MyDataClass");
	Parse(parser, TOKEN_LITERAL, &t1);
	Parse(parser, TOKEN_OPEN_BRACE, 0);

	Token t2(L"int");
	Parse(parser, TOKEN_LITERAL, &t2);
	Token t3(L"dataMember");
	Parse(parser, TOKEN_LITERAL, &t3);
	Parse(parser, TOKEN_SEMI_COLON, 0);

	Parse(parser, TOKEN_CLOSE_BRACE, 0);

	Parse(parser, TOKEN_CLOSE_BRACE, 0);
	Parse(parser, 0, 0);

	ParseFree(parser, free);

	if (g_resultNode)
	{
		CppCodeGenerator codeGenerator;
		codeGenerator.generate(L"MyDataClass.dd", g_resultNode);
	}
}
