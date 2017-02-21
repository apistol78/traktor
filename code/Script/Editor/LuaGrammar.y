%token_prefix TOKEN_
%token_type { traktor::script::LuaGrammarToken* }
%token_destructor { delete $$; }
%fallback  OPEN LEFT_PARANTHESIS .
%extra_argument { const traktor::script::ScriptOutlineLua* outline }

%include
{

#include <assert.h>
#include "Core/Log/Log.h"
#include "Script/Editor/LuaGrammarToken.h"
#include "Script/Editor/ScriptOutlineLua.h"

using namespace traktor;
using namespace traktor::script;

LuaGrammarToken* null()
{
	return new LuaGrammarToken();
}

LuaGrammarToken* copy(LuaGrammarToken* A)
{
	if (A)
		return new LuaGrammarToken(*A);
	else
		return 0;
}

bool is_node(const LuaGrammarToken* A)
{
	return A != 0 && A->node != 0;
}

LuaGrammarToken* merge(LuaGrammarToken* A, LuaGrammarToken* B)
{
	LuaGrammarToken* R = new LuaGrammarToken();

	if (is_node(A) && is_node(B))
	{
		A->node->setNextTail(B->node);
		R->line = A->line;
		R->node = A->node;
	}
	else if (is_node(A))
	{
		R->line = A->line;
		R->node = A->node;
	}
	else if (is_node(B))
	{
		R->line = B->line;
		R->node = B->node;
	}

	return R;
}

std::wstring as_string(const LuaGrammarToken* A)
{
	if (is_node(A))
		return L"[node]";
	else if (A)
		return A->text;
	else
		return L"[null]";
}

void dump(const LuaGrammarToken* A)
{
	log::info << L"text = " << A->text << Endl;
	log::info << L"node = " << type_name(A->node) << Endl;
	log::info << L"line = " << A->line << Endl;
}

}

%syntax_error
{

outline->syntaxError();

}

chunk      ::= block(A) .							{ outline->parseResult(A->node); }

semi       ::= SEMICOLON .
semi       ::= .

block(A)   ::= scope(B) statlist(C) .					{ A = merge(B, C); }
block(A)   ::= scope(B) statlist(C) laststat(D) semi .	{ A = merge(merge(B, C), D); }
ublock(A)  ::= block(B) UNTIL exp(C) .					{ A = merge(B, C); }

scope(A)   ::= .										{ A = null(); }
scope(A)   ::= scope(B) statlist(C) binding(D) semi.	{ A = merge(merge(B, C), D); }
           
statlist(A)   ::= .									{ A = null(); }
statlist(A)   ::= statlist(B) stat(C) semi .		{ A = merge(B, C); }

stat(A)    ::= DO block(B) END .					{ A = copy(B); }
stat(A)    ::= WHILE exp DO block(B) END .			{ A = copy(B); }
stat(A)    ::= repetition DO block(B) END .			{ A = copy(B); }
stat(A)    ::= REPEAT ublock(B) .					{ A = copy(B); }
stat(A)    ::= IF conds(B) END .					{ A = copy(B); }
stat(A)    ::= FUNCTION funcname(B) funcbody(C) .	{ A = new LuaGrammarToken(new IScriptOutline::FunctionNode(B->line, B->text, false, C->node), B->line); }
stat(A)    ::= setlist EQUAL explist1 .				{ A = null(); }
stat(A)    ::= functioncall(B) .					{ A = copy(B); }

repetition ::= FOR NAME EQUAL explist23 .
repetition ::= FOR namelist IN explist1 .
           
conds(A)    ::= condlist(B) .						{ A = copy(B); }
conds(A)    ::= condlist(B) ELSE block(C) .			{ A = merge(B, C); }
condlist(A) ::= cond(B) .							{ A = copy(B); }
condlist(A) ::= condlist(B) ELSEIF cond(C) .		{ A = merge(B, C); }
cond(A)     ::= exp(B) THEN block(C) .				{ A = merge(B, C); }
           
laststat(A) ::= BREAK .								{ A = null(); }
laststat(A) ::= RETURN .							{ A = null(); }
laststat(A) ::= RETURN explist1(B) .				{ A = copy(B); }

binding(A) ::= LOCAL namelist .						{ A = null(); }
binding(A) ::= LOCAL namelist EQUAL explist1(B) .	{ A = copy(B); }
binding(A) ::= LOCAL FUNCTION NAME(B) funcbody(C) .	{ A = new LuaGrammarToken(new IScriptOutline::FunctionNode(B->line, B->text, true, C->node), B->line); }

funcname(A) ::= dottedname(B) .						{ A = copy(B); }
funcname(A) ::= dottedname(B) COLON NAME(C) .		{ A = new LuaGrammarToken(B->text + L":" + C->text, C->line); }

dottedname(A) ::= NAME(B) .							{ A = copy(B); }
dottedname(A) ::= dottedname(B) DOT NAME(C) .		{ A = new LuaGrammarToken(B->text + L"." + C->text, C->line); }

namelist(A) ::= NAME(B) .							{ A = copy(B); }
namelist(A) ::= namelist(B) COMMA NAME(C) .			{ A = new LuaGrammarToken(B->text + L"," + C->text, C->line); }

explist1(A)   ::= exp(B) .								{ A = copy(B); }
explist1(A)   ::= explist1(B) COMMA exp(C) .			{ A = merge(B, C); }
explist23(A)  ::= exp(B) COMMA exp(C) .					{ A = merge(B, C); }
explist23(A)  ::= exp(B) COMMA exp(C) COMMA exp(D) .	{ A = merge(merge(B, C), D); }

%left      OR .
%left      AND .
%left      LESS LESS_EQUAL GREATER GREATER_EQUAL EQUAL_EQUAL NOT_EQUAL .
%right     DOT_DOT .
%left      PLUS MINUS .
%left      MULTIPLY DIVIDE MODULO .
%right     NOT HASH .
%right     RAISE .

exp(A)     ::= NIL|TRUE|FALSE|NUMBER|STRING|DOT_DOT_DOT .									{ A = null(); }
exp(A)     ::= function(B) .																{ A = copy(B); }
exp(A)     ::= prefixexp(B) .																{ A = copy(B); }
exp(A)     ::= tableconstructor .															{ A = null(); }
exp(A)     ::= NOT|HASH|MINUS exp(B) .       [NOT]											{ A = copy(B); }
exp(A)     ::= exp(B) OR exp(C) .															{ A = merge(B, C); }
exp(A)     ::= exp(B) AND exp(C) .															{ A = merge(B, C); }
exp(A)     ::= exp(B) LESS|LESS_EQUAL|GREATER|GREATER_EQUAL|EQUAL_EQUAL|NOT_EQUAL exp(C) .	{ A = merge(B, C); }
exp(A)     ::= exp(B) DOT_DOT exp(C) .														{ A = merge(B, C); }
exp(A)     ::= exp(B) BITWISE_OR exp(C) .                                                   { A = merge(B, C); }
exp(A)     ::= exp(B) BITWISE_AND exp(C) .                                                  { A = merge(B, C); }
exp(A)     ::= exp(B) PLUS|MINUS exp(C) .													{ A = merge(B, C); }
exp(A)     ::= exp(B) MULTIPLY|DIVIDE|MODULO exp(C) .										{ A = merge(B, C); }
exp(A)     ::= exp(B) RAISE exp(C) .														{ A = merge(B, C); }
           
setlist    ::= var .
setlist    ::= setlist COMMA var .

var(A)     ::= NAME(B) .											{ A = copy(B); }
var(A)     ::= prefixexp(B) LEFT_BRACKET exp(C) RIGHT_BRACKET .		{ A = merge(B, C); }
var(A)     ::= prefixexp(B) DOT NAME(C) .							{ A = new LuaGrammarToken(as_string(B) + L"." + C->text, C->line); }

prefixexp(A)  ::= var(B) .										{ A = copy(B); }
prefixexp(A)  ::= functioncall(B) .								{ A = copy(B); }
prefixexp(A)  ::= OPEN exp(B) RIGHT_PARANTHESIS .				{ A = copy(B); }

functioncall(A) ::= prefixexp(B) args(C) .							{ A = merge(new LuaGrammarToken(new IScriptOutline::FunctionReferenceNode(B->line, as_string(B)), B->line), C); }
functioncall(A) ::= prefixexp(B) COLON NAME(C) args(D) . 			{ A = merge(new LuaGrammarToken(new IScriptOutline::FunctionReferenceNode(C->line, as_string(B) + L":" + C->text), C->line), D); }

args(A)     ::= LEFT_PARANTHESIS RIGHT_PARANTHESIS .				{ A = null(); }
args(A)     ::= LEFT_PARANTHESIS explist1(B) RIGHT_PARANTHESIS .	{ A = copy(B); }
args(A)     ::= tableconstructor .									{ A = null(); }
args(A)     ::= STRING .											{ A = null(); }

function(A) ::= FUNCTION funcbody(B) .							{ A = copy(B); }

funcbody(A) ::= params block(B) END .							{ A = copy(B); }

params      ::= LEFT_PARANTHESIS parlist RIGHT_PARANTHESIS .

parlist     ::= .
parlist     ::= namelist .
parlist     ::= DOT_DOT_DOT .
parlist     ::= namelist COMMA DOT_DOT_DOT .

tableconstructor ::= LEFT_BRACE RIGHT_BRACE .
tableconstructor ::= LEFT_BRACE fieldlist RIGHT_BRACE .
tableconstructor ::= LEFT_BRACE fieldlist COMMA|SEMICOLON RIGHT_BRACE .

fieldlist   ::= field .
fieldlist   ::= fieldlist COMMA|SEMICOLON field .
            
field       ::= exp .
field       ::= NAME EQUAL exp .
field       ::= LEFT_BRACKET exp RIGHT_BRACKET EQUAL exp .
