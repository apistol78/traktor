
%token_prefix TOKEN_
%token_type { Token* }
%default_type { Token* }
%token_destructor { delete $$; }
%extra_argument { ddc::ParseState* state }
 
%include
{

#include <cassert>
#include <Core/Log/Log.h>
#include "Ddc/DfnAlias.h"
#include "Ddc/DfnClass.h"
#include "Ddc/DfnImport.h"
#include "Ddc/DfnMember.h"
#include "Ddc/DfnNamespace.h"
#include "Ddc/DfnType.h"
#include "Ddc/ParseState.h"
#include "Ddc/Token.h"

using namespace traktor;
using namespace ddc;

}

%syntax_error
{

log::error << L"Syntax error; unable to generate data definition class" << Endl;

}

program ::= statements(A).
{
	state->root = A->node;
}

statements(A) ::= statement(B) statements(C).
{
	B->node->setNext(C->node);
	A = new Token(B->node);
}
	
statements(A) ::= statement(B).
{
	A = new Token(B->node);
}

statement(A) ::= IMPORT qualified_literal(B) SEMI_COLON.
{
	A = new Token(
		new DfnImport(
			B->literal
		)
	);
}

statement(A) ::= ALIAS STRING(B) alias_type(C) OPEN_BRACE STRING(D) COMMA STRING(E) COMMA STRING(F) CLOSE_BRACE.
{
	A = new Token(
		new DfnAlias(
			B->literal,	// language
			C->node,	// type
			D->literal,	// language declare type
			E->literal,	// language in/out type
			F->literal	// language member
		)
	);
}

alias_type(A) ::= LITERAL(B).
{
	A = new Token(new DfnType(B->literal, 0));
}

alias_type(A) ::= LITERAL(B) LESS alias_type_subst(C) GREATER.
{
	A = new Token(new DfnType(B->literal, C->node));
}

alias_type(A) ::= LITERAL(B) OPEN_BRACKET CLOSE_BRACKET.
{
	A = new Token(new DfnType(B->literal, 0, true));
}

alias_type(A) ::= LITERAL(B) OPEN_BRACKET LITERAL(C) CLOSE_BRACKET.
{
	A = new Token(new DfnType(B->literal, 0, true));
}

alias_type(A) ::= LITERAL(B) LESS alias_type_subst(C) GREATER OPEN_BRACKET CLOSE_BRACKET.
{
	A = new Token(new DfnType(B->literal, C->node, true));
}

alias_type(A) ::= LITERAL(B) LESS alias_type_subst(C) GREATER OPEN_BRACKET LITERAL(D) CLOSE_BRACKET.
{
	A = new Token(new DfnType(B->literal, C->node, true));
}

alias_type_subst(A) ::= alias_type_subst(B) COMMA alias_type(C).
{
	B->node->setNext(C->node);
	A = new Token(B->node);
}

alias_type_subst(A) ::= alias_type(B).
{
	A = new Token(B->node);
}

statement(A) ::= NAMESPACE LITERAL(B) OPEN_BRACE statements(C) CLOSE_BRACE.
{
	A = new Token(
		new DfnNamespace(
			B->literal,
			C->node
		)
	);
}

statement(A) ::= CLASS LITERAL(B) OPEN_BRACE members(C) CLOSE_BRACE.
{
	A = new Token(
		new DfnClass(
			DfnClass::AccPrivate,
			B->literal,
			C->node
		)
	);
}

statement(A) ::= PUBLIC CLASS LITERAL(B) OPEN_BRACE members(C) CLOSE_BRACE.
{
	A = new Token(
		new DfnClass(
			DfnClass::AccPublic,
			B->literal,
			C->node
		)
	);
}

statement(A) ::= PRIVATE CLASS LITERAL(B) OPEN_BRACE members(C) CLOSE_BRACE.
{
	A = new Token(
		new DfnClass(
			DfnClass::AccPrivate,
			B->literal,
			C->node
		)
	);
}

statement(A) ::= CLASS LITERAL(B) COLON qualified_literal(C) OPEN_BRACE members(D) CLOSE_BRACE.
{
	A = new Token(
		new DfnClass(
			DfnClass::AccPrivate,
			B->literal,
			C->literal,
			D->node
		)
	);
}

statement(A) ::= PUBLIC CLASS LITERAL(B) COLON qualified_literal(C) OPEN_BRACE members(D) CLOSE_BRACE.
{
	A = new Token(
		new DfnClass(
			DfnClass::AccPublic,
			B->literal,
			C->literal,
			D->node
		)
	);
}

statement(A) ::= PRIVATE CLASS LITERAL(B) COLON qualified_literal(C) OPEN_BRACE members(D) CLOSE_BRACE.
{
	A = new Token(
		new DfnClass(
			DfnClass::AccPrivate,
			B->literal,
			C->literal,
			D->node
		)
	);
}

members(A) ::= member(B) members(C).
{
	B->node->setNext(C->node);
	A = new Token(B->node);
}

members(A) ::= member(B).
{
	A = new Token(
		B->node
	);
}

member(A) ::= member_type(B) LITERAL(C) SEMI_COLON.
{
	A = new Token(
		new DfnMember(B->node, C->literal)
	);
}

member_type(A) ::= qualified_literal(B).
{
	A = new Token(new DfnType(B->literal, 0));
}

member_type(A) ::= qualified_literal(B) LESS member_type_subst(C) GREATER.
{
	A = new Token(new DfnType(B->literal, C->node));
}

member_type(A) ::= qualified_literal(B) OPEN_BRACKET CLOSE_BRACKET.
{
	A = new Token(new DfnType(B->literal, 0, true));
}

member_type(A) ::= qualified_literal(B) OPEN_BRACKET NUMBER(C) CLOSE_BRACKET.
{
	A = new Token(new DfnType(B->literal, 0, true, int32_t(C->number)));
}

member_type(A) ::= qualified_literal(B) LESS member_type_subst(C) GREATER OPEN_BRACKET CLOSE_BRACKET.
{
	A = new Token(new DfnType(B->literal, C->node, true));
}

member_type(A) ::= qualified_literal(B) LESS member_type_subst(C) GREATER OPEN_BRACKET NUMBER(D) CLOSE_BRACKET.
{
	A = new Token(new DfnType(B->literal, C->node, true, int32_t(D->number)));
}

member_type_subst(A) ::= member_type_subst(B) COMMA member_type(C).
{
	B->node->setNext(C->node);
	A = new Token(B->node);
}

member_type_subst(A) ::= member_type(B).
{
	A = new Token(B->node);
}

qualified_literal(A) ::= LITERAL(B) DOT qualified_literal(C).
{
	A = new Token(B->literal + L"." + C->literal);
}

qualified_literal(A) ::= LITERAL(B).
{
	A = B;
}
