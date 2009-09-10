
%token_prefix TOKEN_
%token_type { Token* }
%default_type { Token* }
%token_destructor { delete $$; }
 
%include
{

#include <cassert>
#include <Core/Heap/GcNew.h>
#include <Core/Log/Log.h>
#include "Ddc/DfnBranch.h"
#include "Ddc/DfnClass.h"
#include "Ddc/DfnMember.h"
#include "Ddc/DfnNamespace.h"
#include "Ddc/DfnType.h"
#include "Ddc/Token.h"

using namespace traktor;
using namespace ddc;

extern Ref< DfnNode > g_resultNode;

}

%syntax_error
{

log::error << L"Syntax error; unable to generate data definition class" << Endl;

}

program ::= statements(A).
{
	g_resultNode = A->node;
}

statements(A) ::= statement(B) statements(C).
{
	A = new Token(
		gc_new< DfnBranch >(
			B->node,
			C->node
		)
	);
}
	
statements(A) ::= statement(B).
{
	A = new Token(B->node);
}

statement(A) ::= NAMESPACE LITERAL(B) OPEN_BRACE statements(C) CLOSE_BRACE.
{
	A = new Token(
		gc_new< DfnNamespace >(
			B->literal,
			C->node
		)
	);
}

statement(A) ::= CLASS LITERAL(B) OPEN_BRACE members(C) CLOSE_BRACE.
{
	A = new Token(
		gc_new< DfnClass >(
			DfnClass::AccPrivate,
			B->literal,
			C->node
		)
	);
}

statement(A) ::= PUBLIC CLASS LITERAL(B) OPEN_BRACE members(C) CLOSE_BRACE.
{
	A = new Token(
		gc_new< DfnClass >(
			DfnClass::AccPublic,
			B->literal,
			C->node
		)
	);
}

statement(A) ::= PRIVATE CLASS LITERAL(B) OPEN_BRACE members(C) CLOSE_BRACE.
{
	A = new Token(
		gc_new< DfnClass >(
			DfnClass::AccPrivate,
			B->literal,
			C->node
		)
	);
}

statement(A) ::= CLASS LITERAL(B) COLON LITERAL(C) OPEN_BRACE members(D) CLOSE_BRACE.
{
	A = new Token(
		gc_new< DfnClass >(
			DfnClass::AccPrivate,
			B->literal,
			C->literal,
			D->node
		)
	);
}

statement(A) ::= PUBLIC CLASS LITERAL(B) COLON LITERAL(C) OPEN_BRACE members(D) CLOSE_BRACE.
{
	A = new Token(
		gc_new< DfnClass >(
			DfnClass::AccPublic,
			B->literal,
			C->literal,
			D->node
		)
	);
}

statement(A) ::= PRIVATE CLASS LITERAL(B) COLON LITERAL(C) OPEN_BRACE members(D) CLOSE_BRACE.
{
	A = new Token(
		gc_new< DfnClass >(
			DfnClass::AccPrivate,
			B->literal,
			C->literal,
			D->node
		)
	);
}

members(A) ::= member(B) members(C).
{
	A = new Token(
		gc_new< DfnBranch >(B->node, C->node)
	);
}

members(A) ::= member(B).
{
	A = new Token(
		B->node
	);
}

member(A) ::= type(B) LITERAL(C) SEMI_COLON.
{
	A = new Token(
		gc_new< DfnMember >(B->node, C->literal)
	);
}

type(A) ::= LITERAL(B).
{
	A = new Token(
		gc_new< DfnType >(B->literal, false)
	);
}

type(A) ::= LITERAL(B) OPEN_BRACKET CLOSE_BRACKET.
{
	A = new Token(
		gc_new< DfnType >(B->literal, true)
	);
}
