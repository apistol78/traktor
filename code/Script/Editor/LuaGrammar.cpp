/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
/* Driver template for the LEMON parser generator.
** The author disclaims copyright to this source code.
*/
/* First off, code is included that follows the "include" declaration
** in the input grammar file. */
#include <stdio.h>
#line 8 "LuaGrammar.y"


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

#line 78 "LuaGrammar.cpp"
/* Next is all token values, in a form suitable for use by makeheaders.
** This section will be null unless lemon is run with the -m switch.
*/
/* 
** These constants (all generated automatically by the parser generator)
** specify the various kinds of tokens (terminals) that the parser
** understands. 
**
** Each symbol here is a terminal symbol in the grammar.
*/
/* Make sure the INTERFACE macro is defined.
*/
#ifndef INTERFACE
# define INTERFACE 1
#endif
/* The next thing included is series of defines which control
** various aspects of the generated parser.
**    YYCODETYPE         is the data type used for storing terminal
**                       and nonterminal numbers.  "unsigned char" is
**                       used if there are fewer than 250 terminals
**                       and nonterminals.  "int" is used otherwise.
**    YYNOCODE           is a number of type YYCODETYPE which corresponds
**                       to no legal terminal or nonterminal number.  This
**                       number is used to fill in empty slots of the hash 
**                       table.
**    YYFALLBACK         If defined, this indicates that one or more tokens
**                       have fall-back values which should be used if the
**                       original value of the token will not parse.
**    YYACTIONTYPE       is the data type used for storing terminal
**                       and nonterminal numbers.  "unsigned char" is
**                       used if there are fewer than 250 rules and
**                       states combined.  "int" is used otherwise.
**    ParseTOKENTYPE     is the data type used for minor tokens given 
**                       directly to the parser from the tokenizer.
**    YYMINORTYPE        is the data type used for all minor tokens.
**                       This is typically a union of many types, one of
**                       which is ParseTOKENTYPE.  The entry in the union
**                       for base tokens is called "yy0".
**    YYSTACKDEPTH       is the maximum depth of the parser's stack.  If
**                       zero the stack is dynamically sized using realloc()
**    ParseARG_SDECL     A static variable declaration for the %extra_argument
**    ParseARG_PDECL     A parameter declaration for the %extra_argument
**    ParseARG_STORE     Code to store %extra_argument into yypParser
**    ParseARG_FETCH     Code to extract %extra_argument from yypParser
**    YYNSTATE           the combined number of states.
**    YYNRULE            the number of rules in the grammar
**    YYERRORSYMBOL      is the code number of the error symbol.  If not
**                       defined, then do no error processing.
*/
#define YYCODETYPE unsigned char
#define YYNOCODE 88
#define YYACTIONTYPE unsigned char
#define ParseTOKENTYPE  traktor::script::LuaGrammarToken* 
typedef union {
  int yyinit;
  ParseTOKENTYPE yy0;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 100
#endif
#define ParseARG_SDECL  const traktor::script::ScriptOutlineLua* outline ;
#define ParseARG_PDECL , const traktor::script::ScriptOutlineLua* outline 
#define ParseARG_FETCH  const traktor::script::ScriptOutlineLua* outline  = yypParser->outline 
#define ParseARG_STORE yypParser->outline  = outline 
#define YYNSTATE 164
#define YYNRULE 85
#define YYFALLBACK 1
#define YY_NO_ACTION      (YYNSTATE+YYNRULE+2)
#define YY_ACCEPT_ACTION  (YYNSTATE+YYNRULE+1)
#define YY_ERROR_ACTION   (YYNSTATE+YYNRULE)

/* The yyzerominor constant is used to initialize instances of
** YYMINORTYPE objects to zero. */
static const YYMINORTYPE yyzerominor = { 0 };

/* Define the yytestcase() macro to be a no-op if is not already defined
** otherwise.
**
** Applications can choose to define yytestcase() in the %include section
** to a macro that can assist in verifying code coverage.  For production
** code the yytestcase() macro should be turned off.  But it is useful
** for testing.
*/
#ifndef yytestcase
# define yytestcase(X)
#endif


/* Next are the tables used to determine what action to take based on the
** current state and lookahead token.  These tables are used to implement
** functions that take a state number and lookahead value and return an
** action integer.  
**
** Suppose the action integer is N.  Then the action is determined as
** follows
**
**   0 <= N < YYNSTATE                  Shift N.  That is, push the lookahead
**                                      token onto the stack and goto state N.
**
**   YYNSTATE <= N < YYNSTATE+YYNRULE   Reduce by rule N-YYNSTATE.
**
**   N == YYNSTATE+YYNRULE              A syntax error has occurred.
**
**   N == YYNSTATE+YYNRULE+1            The parser accepts its input.
**
**   N == YYNSTATE+YYNRULE+2            No such action.  Denotes unused
**                                      slots in the yy_action[] table.
**
** The action table is constructed as a single large table named yy_action[].
** Given state S and lookahead X, the action is computed as
**
**      yy_action[ yy_shift_ofst[S] + X ]
**
** If the index value yy_shift_ofst[S]+X is out of range or if the value
** yy_lookahead[yy_shift_ofst[S]+X] is not equal to X or if yy_shift_ofst[S]
** is equal to YY_SHIFT_USE_DFLT, it means that the action is not in the table
** and that yy_default[S] should be used instead.  
**
** The formula above is for computing the action when the lookahead is
** a terminal symbol.  If the lookahead is a non-terminal (as occurs after
** a reduce action) then the yy_reduce_ofst[] array is used in place of
** the yy_shift_ofst[] array and YY_REDUCE_USE_DFLT is used in place of
** YY_SHIFT_USE_DFLT.
**
** The following are the tables generated in this section:
**
**  yy_action[]        A single table containing all actions.
**  yy_lookahead[]     A table containing the lookahead for each entry in
**                     yy_action.  Used to detect hash collisions.
**  yy_shift_ofst[]    For each state, the offset into yy_action for
**                     shifting terminals.
**  yy_reduce_ofst[]   For each state, the offset into yy_action for
**                     shifting non-terminals after a reduce.
**  yy_default[]       Default action for each state.
*/
#define YY_ACTTAB_COUNT (760)
static const YYACTIONTYPE yy_action[] = {
 /*     0 */    47,  147,   20,    9,  250,  116,   48,   54,   32,   34,
 /*    10 */     1,   41,   16,   50,  154,   97,   27,  164,   27,  118,
 /*    20 */     8,   49,   31,   28,   27,   27,   27,   27,   26,   23,
 /*    30 */    23,   22,   22,   22,   90,  146,   21,  148,  134,  115,
 /*    40 */    27,   54,   27,   25,   24,  159,   31,   28,   27,   27,
 /*    50 */    27,   27,   26,   23,   23,   22,   22,   22,   44,   12,
 /*    60 */    21,   22,   22,   22,  158,  112,   21,   25,   24,  137,
 /*    70 */    27,  151,   27,   25,   24,  111,   31,   28,   27,   27,
 /*    80 */    27,   27,   26,   23,   23,   22,   22,   22,   21,  104,
 /*    90 */    21,   54,  102,  101,  129,   25,   24,   25,   24,   27,
 /*   100 */   109,   27,    4,   11,  149,   31,   28,   27,   27,   27,
 /*   110 */    27,   26,   23,   23,   22,   22,   22,  221,   33,   21,
 /*   120 */   114,  113,   54,   54,   43,    4,   25,   24,   27,  138,
 /*   130 */    27,  220,  221,  145,   31,   28,   27,   27,   27,   27,
 /*   140 */    26,   23,   23,   22,   22,   22,  220,  105,   21,   54,
 /*   150 */    89,   19,   27,  142,   27,   25,   24,   14,   31,   28,
 /*   160 */    27,   27,   27,   27,   26,   23,   23,   22,   22,   22,
 /*   170 */    17,  127,   21,   54,   45,  226,   27,   37,   27,   25,
 /*   180 */    24,   13,   31,   28,   27,   27,   27,   27,   26,   23,
 /*   190 */    23,   22,   22,   22,  130,   15,   21,  226,  226,  125,
 /*   200 */    27,   54,   27,   25,   24,  122,   31,   28,   27,   27,
 /*   210 */    27,   27,   26,   23,   23,   22,   22,   22,    5,  226,
 /*   220 */    21,  128,   27,  226,   27,  121,  226,   25,   24,   28,
 /*   230 */    27,   27,   27,   27,   26,   23,   23,   22,   22,   22,
 /*   240 */   148,  132,   21,  135,   27,  132,   27,  136,   16,   25,
 /*   250 */    24,  150,   27,   27,   27,   27,   26,   23,   23,   22,
 /*   260 */    22,   22,  131,   10,   21,   46,   20,   40,  147,    2,
 /*   270 */   152,   25,   24,   93,  144,   42,   86,   35,  110,   84,
 /*   280 */    26,   23,   23,   22,   22,   22,   20,    7,   21,  123,
 /*   290 */    92,    6,   39,    3,  162,   25,   24,   94,  154,  163,
 /*   300 */    29,  161,   97,   46,   29,   29,  117,  157,  157,  157,
 /*   310 */   157,  157,  157,   20,  160,   18,   83,   57,    2,  141,
 /*   320 */    46,  103,   42,   99,   77,  110,  152,   38,  120,   87,
 /*   330 */   124,  251,  156,   36,  155,  153,  251,  251,  251,  251,
 /*   340 */   251,  251,  251,  251,  251,  251,  251,   29,  251,  251,
 /*   350 */   251,   29,   29,  251,  157,  157,  157,  157,  157,  157,
 /*   360 */    20,  251,   18,  251,  251,    2,  143,  251,   65,   42,
 /*   370 */    53,  251,  154,   52,   51,  106,  100,  152,  251,   85,
 /*   380 */   251,   75,  251,  156,   36,  155,  153,  251,   35,  251,
 /*   390 */    82,    5,  251,  251,   29,  251,  251,  251,   29,   29,
 /*   400 */   251,  157,  157,  157,  157,  157,  157,   20,  251,  251,
 /*   410 */   251,  133,    2,  107,  108,   68,   42,  251,  251,  154,
 /*   420 */   251,  251,  251,  251,  152,  251,  251,  251,  251,  251,
 /*   430 */   156,   36,  155,  153,  251,  131,  251,   78,  139,   30,
 /*   440 */   251,   29,    2,  251,  251,   29,   29,  251,  157,  157,
 /*   450 */   157,  157,  157,  157,   57,  251,  251,  251,  251,    2,
 /*   460 */   251,  251,  251,  152,  251,   56,  251,  126,  251,  156,
 /*   470 */    36,  155,  153,  251,  152,  119,   65,  251,  251,  251,
 /*   480 */   156,   36,  155,  153,   98,  152,  251,  251,   65,  251,
 /*   490 */   251,  156,   36,  155,  153,  251,   96,  152,  251,  251,
 /*   500 */   251,   65,  251,  156,   36,  155,  153,  251,  251,   95,
 /*   510 */   152,  251,   65,  251,  251,  251,  156,   36,  155,  153,
 /*   520 */    91,  152,  251,   65,  251,  251,  251,  156,   36,  155,
 /*   530 */   153,   88,  152,   68,  251,  251,  251,  251,  156,   36,
 /*   540 */   155,  153,  152,  251,   61,  251,  251,  251,  156,   36,
 /*   550 */   155,  153,  251,  152,  251,   71,  140,  251,  251,  156,
 /*   560 */    36,  155,  153,  251,  152,   58,  251,  251,  251,   81,
 /*   570 */   156,   36,  155,  153,  152,  251,  251,  251,  152,  251,
 /*   580 */   156,   36,  155,  153,  156,   36,  155,  153,  251,  251,
 /*   590 */   251,   72,  251,  251,  251,   74,  251,  251,  251,  251,
 /*   600 */   152,  251,  251,  251,  152,   73,  156,   36,  155,  153,
 /*   610 */   156,   36,  155,  153,  152,   70,  251,  251,  251,   69,
 /*   620 */   156,   36,  155,  153,  152,  251,  251,  251,  152,   76,
 /*   630 */   156,   36,  155,  153,  156,   36,  155,  153,  152,  251,
 /*   640 */   251,  251,   80,  251,  156,   36,  155,  153,  251,  251,
 /*   650 */   251,  152,  251,  251,  251,   79,  251,  156,   36,  155,
 /*   660 */   153,  251,  251,  251,  152,  251,  251,  251,   60,  251,
 /*   670 */   156,   36,  155,  153,  251,  251,  251,  152,   67,  251,
 /*   680 */   251,  251,  251,  156,   36,  155,  153,  152,   59,  251,
 /*   690 */   251,  251,   66,  156,   36,  155,  153,  152,  251,  251,
 /*   700 */   251,  152,   64,  156,   36,  155,  153,  156,   36,  155,
 /*   710 */   153,  152,  251,  251,  251,   63,  251,  156,   36,  155,
 /*   720 */   153,  251,  251,  251,  152,  251,  251,  251,   55,  251,
 /*   730 */   156,   36,  155,  153,  251,  251,  251,  152,  251,  251,
 /*   740 */   251,   62,  251,  156,   36,  155,  153,  251,  251,  251,
 /*   750 */   152,  251,  251,  251,  251,  251,  156,   36,  155,  153,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */     5,   13,    1,   14,   56,   57,    5,   59,    7,    8,
 /*    10 */     9,   10,   26,   12,   13,   26,   21,    0,   23,   18,
 /*    20 */    19,   20,   27,   28,   29,   30,   31,   32,   33,   34,
 /*    30 */    35,   36,   37,   38,   10,   47,   41,   13,   52,   57,
 /*    40 */    21,   59,   23,   48,   49,    6,   27,   28,   29,   30,
 /*    50 */    31,   32,   33,   34,   35,   36,   37,   38,   15,   16,
 /*    60 */    41,   36,   37,   38,    6,   74,   41,   48,   49,   13,
 /*    70 */    21,   52,   23,   48,   49,   84,   27,   28,   29,   30,
 /*    80 */    31,   32,   33,   34,   35,   36,   37,   38,   41,   57,
 /*    90 */    41,   59,   24,   25,   62,   48,   49,   48,   49,   21,
 /*   100 */    51,   23,    3,   11,    6,   27,   28,   29,   30,   31,
 /*   110 */    32,   33,   34,   35,   36,   37,   38,   11,   26,   41,
 /*   120 */    57,   57,   59,   59,   17,   26,   48,   49,   21,   51,
 /*   130 */    23,   11,   26,   52,   27,   28,   29,   30,   31,   32,
 /*   140 */    33,   34,   35,   36,   37,   38,   26,   57,   41,   59,
 /*   150 */    26,   11,   21,   54,   23,   48,   49,   26,   27,   28,
 /*   160 */    29,   30,   31,   32,   33,   34,   35,   36,   37,   38,
 /*   170 */    11,   57,   41,   59,    5,    2,   21,   13,   23,   48,
 /*   180 */    49,   26,   27,   28,   29,   30,   31,   32,   33,   34,
 /*   190 */    35,   36,   37,   38,    6,    4,   41,   24,   25,   57,
 /*   200 */    21,   59,   23,   48,   49,   13,   27,   28,   29,   30,
 /*   210 */    31,   32,   33,   34,   35,   36,   37,   38,    2,   46,
 /*   220 */    41,    6,   21,   50,   23,   13,   53,   48,   49,   28,
 /*   230 */    29,   30,   31,   32,   33,   34,   35,   36,   37,   38,
 /*   240 */    13,   80,   41,   82,   21,   80,   23,   82,   26,   48,
 /*   250 */    49,   69,   29,   30,   31,   32,   33,   34,   35,   36,
 /*   260 */    37,   38,   46,   11,   41,   83,    1,   68,   13,   53,
 /*   270 */    72,   48,   49,   22,   47,   10,   77,   79,   13,   81,
 /*   280 */    33,   34,   35,   36,   37,   38,    1,   11,   41,   69,
 /*   290 */    23,   11,   13,   60,   58,   48,   49,   21,   13,    3,
 /*   300 */    35,   58,   26,   83,   39,   40,   69,   42,   43,   44,
 /*   310 */    45,   46,   47,    1,   58,   50,   74,   63,   53,   54,
 /*   320 */    83,   67,   10,   13,   74,   13,   72,    2,   13,   75,
 /*   330 */    76,   87,   78,   79,   80,   81,   87,   87,   87,   87,
 /*   340 */    87,   87,   87,   87,   87,   87,   87,   35,   87,   87,
 /*   350 */    87,   39,   40,   87,   42,   43,   44,   45,   46,   47,
 /*   360 */     1,   87,   50,   87,   87,   53,   54,   87,   63,   10,
 /*   370 */    61,   87,   13,   64,   65,   66,   71,   72,   87,   70,
 /*   380 */    87,   72,   87,   78,   79,   80,   81,   87,   79,   87,
 /*   390 */    81,    2,   87,   87,   35,   87,   87,   87,   39,   40,
 /*   400 */    87,   42,   43,   44,   45,   46,   47,    1,   87,   87,
 /*   410 */    87,   52,   53,   24,   25,   63,   10,   87,   87,   13,
 /*   420 */    87,   87,   87,   87,   72,   87,   87,   87,   87,   87,
 /*   430 */    78,   79,   80,   81,   87,   46,   87,   85,   86,   50,
 /*   440 */    87,   35,   53,   87,   87,   39,   40,   87,   42,   43,
 /*   450 */    44,   45,   46,   47,   63,   87,   87,   87,   87,   53,
 /*   460 */    87,   87,   87,   72,   87,   63,   87,   76,   87,   78,
 /*   470 */    79,   80,   81,   87,   72,   73,   63,   87,   87,   87,
 /*   480 */    78,   79,   80,   81,   71,   72,   87,   87,   63,   87,
 /*   490 */    87,   78,   79,   80,   81,   87,   71,   72,   87,   87,
 /*   500 */    87,   63,   87,   78,   79,   80,   81,   87,   87,   71,
 /*   510 */    72,   87,   63,   87,   87,   87,   78,   79,   80,   81,
 /*   520 */    71,   72,   87,   63,   87,   87,   87,   78,   79,   80,
 /*   530 */    81,   71,   72,   63,   87,   87,   87,   87,   78,   79,
 /*   540 */    80,   81,   72,   87,   63,   87,   87,   87,   78,   79,
 /*   550 */    80,   81,   87,   72,   87,   63,   86,   87,   87,   78,
 /*   560 */    79,   80,   81,   87,   72,   63,   87,   87,   87,   63,
 /*   570 */    78,   79,   80,   81,   72,   87,   87,   87,   72,   87,
 /*   580 */    78,   79,   80,   81,   78,   79,   80,   81,   87,   87,
 /*   590 */    87,   63,   87,   87,   87,   63,   87,   87,   87,   87,
 /*   600 */    72,   87,   87,   87,   72,   63,   78,   79,   80,   81,
 /*   610 */    78,   79,   80,   81,   72,   63,   87,   87,   87,   63,
 /*   620 */    78,   79,   80,   81,   72,   87,   87,   87,   72,   63,
 /*   630 */    78,   79,   80,   81,   78,   79,   80,   81,   72,   87,
 /*   640 */    87,   87,   63,   87,   78,   79,   80,   81,   87,   87,
 /*   650 */    87,   72,   87,   87,   87,   63,   87,   78,   79,   80,
 /*   660 */    81,   87,   87,   87,   72,   87,   87,   87,   63,   87,
 /*   670 */    78,   79,   80,   81,   87,   87,   87,   72,   63,   87,
 /*   680 */    87,   87,   87,   78,   79,   80,   81,   72,   63,   87,
 /*   690 */    87,   87,   63,   78,   79,   80,   81,   72,   87,   87,
 /*   700 */    87,   72,   63,   78,   79,   80,   81,   78,   79,   80,
 /*   710 */    81,   72,   87,   87,   87,   63,   87,   78,   79,   80,
 /*   720 */    81,   87,   87,   87,   72,   87,   87,   87,   63,   87,
 /*   730 */    78,   79,   80,   81,   87,   87,   87,   72,   87,   87,
 /*   740 */    87,   63,   87,   78,   79,   80,   81,   87,   87,   87,
 /*   750 */    72,   87,   87,   87,   87,   87,   78,   79,   80,   81,
};
#define YY_SHIFT_USE_DFLT (-15)
#define YY_SHIFT_COUNT (116)
#define YY_SHIFT_MIN   (-14)
#define YY_SHIFT_MAX   (406)
static const short yy_shift_ofst[] = {
 /*     0 */   -15,  406,  312,    1,  265,  359,  406,  406,  406,  406,
 /*    10 */   406,  406,  406,  406,  406,  406,  406,  406,  406,  406,
 /*    20 */   406,  406,  406,  406,  406,  406,  406,  406,  406,  406,
 /*    30 */   406,  406,  406,  285,  -15,  389,  389,  216,  227,  325,
 /*    40 */   325,  315,  325,  -15,  -15,  -15,  -15,  -15,  -15,   24,
 /*    50 */   310,  296,  296,  296,  -15,  155,  131,  107,   78,   49,
 /*    60 */    19,   -5,  179,  179,  179,  179,  179,  179,  179,  179,
 /*    70 */   179,  201,  223,  247,  247,  173,   25,  276,   99,   47,
 /*    80 */    47,   47,  120,  -11,  106,   92,   68,   43,  -14,  -12,
 /*    90 */   279,  222,  280,  267,  251,  222,  222,  255,  222,  252,
 /*   100 */   222,  212,  192,  215,  191,  188,  169,  164,   56,  159,
 /*   110 */   140,   81,  124,   98,   58,   39,   17,
};
#define YY_REDUCE_USE_DFLT (-53)
#define YY_REDUCE_COUNT (54)
#define YY_REDUCE_MIN   (-52)
#define YY_REDUCE_MAX   (678)
static const short yy_reduce_ofst[] = {
 /*     0 */   -52,  254,  352,  309,  470,  460,  449,  438,  425,  413,
 /*    10 */   402,  305,  391,  678,  665,  652,  639,  629,  625,  615,
 /*    20 */   605,  592,  579,  566,  556,  552,  542,  532,  528,  506,
 /*    30 */   502,  492,  481,  198,   32,  165,  165,  161,   -9,  237,
 /*    40 */   220,  199,  182,  142,  114,   90,   64,   63,  -18,  250,
 /*    50 */   242,  256,  243,  236,  233,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   170,  249,  249,  167,  249,  249,  249,  249,  190,  249,
 /*    10 */   249,  249,  249,  249,  249,  249,  249,  249,  249,  249,
 /*    20 */   249,  249,  249,  249,  249,  249,  249,  249,  249,  249,
 /*    30 */   249,  249,  249,  249,  170,  249,  208,  249,  237,  249,
 /*    40 */   249,  249,  249,  170,  170,  170,  170,  170,  170,  249,
 /*    50 */   249,  166,  166,  166,  172,  204,  249,  249,  249,  249,
 /*    60 */   249,  249,  205,  169,  203,  202,  248,  247,  246,  216,
 /*    70 */   215,  211,  212,  214,  213,  181,  217,  192,  249,  219,
 /*    80 */   218,  210,  225,  249,  225,  249,  196,  184,  249,  249,
 /*    90 */   249,  194,  249,  249,  249,  193,  191,  249,  183,  200,
 /*   100 */   180,  249,  249,  249,  249,  249,  249,  249,  249,  249,
 /*   110 */   222,  249,  238,  249,  249,  249,  249,  195,  189,  182,
 /*   120 */   198,  199,  197,  179,  186,  188,  187,  185,  178,  177,
 /*   130 */   176,  233,  232,  230,  231,  229,  228,  224,  223,  244,
 /*   140 */   245,  243,  242,  241,  239,  236,  240,  201,  200,  235,
 /*   150 */   234,  227,  226,  225,  222,  209,  207,  206,  175,  174,
 /*   160 */   173,  171,  168,  165,
};

/* The next table maps tokens into fallback tokens.  If a construct
** like the following:
** 
**      %fallback ID X Y Z.
**
** appears in the grammar, then ID becomes a fallback token for X, Y,
** and Z.  Whenever one of the tokens X, Y, or Z is input to the parser
** but it does not parse, the type of the token is changed to ID and
** the parse is retried before an error is thrown.
*/
#ifdef YYFALLBACK
static const YYCODETYPE yyFallback[] = {
    0,  /*          $ => nothing */
    0,  /*       OPEN => nothing */
    1,  /* LEFT_PARANTHESIS => OPEN */
};
#endif /* YYFALLBACK */

/* The following structure represents a single element of the
** parser's stack.  Information stored includes:
**
**   +  The state number for the parser at this level of the stack.
**
**   +  The value of the token stored at this level of the stack.
**      (In other words, the "major" token.)
**
**   +  The semantic value stored at this level of the stack.  This is
**      the information used by the action routines in the grammar.
**      It is sometimes called the "minor" token.
*/
struct yyStackEntry {
  YYACTIONTYPE stateno;  /* The state-number */
  YYCODETYPE major;      /* The major token value.  This is the code
                         ** number for the token at this stack level */
  YYMINORTYPE minor;     /* The user-supplied minor token value.  This
                         ** is the value of the token  */
};
typedef struct yyStackEntry yyStackEntry;

/* The state of the parser is completely contained in an instance of
** the following structure */
struct yyParser {
  int yyidx;                    /* Index of top element in stack */
#ifdef YYTRACKMAXSTACKDEPTH
  int yyidxMax;                 /* Maximum value of yyidx */
#endif
  int yyerrcnt;                 /* Shifts left before out of the error */
  ParseARG_SDECL                /* A place to hold %extra_argument */
#if YYSTACKDEPTH<=0
  int yystksz;                  /* Current side of the stack */
  yyStackEntry *yystack;        /* The parser's stack */
#else
  yyStackEntry yystack[YYSTACKDEPTH];  /* The parser's stack */
#endif
};
typedef struct yyParser yyParser;

#ifndef NDEBUG
#include <stdio.h>
static FILE *yyTraceFILE = 0;
static char *yyTracePrompt = 0;
#endif /* NDEBUG */

#ifndef NDEBUG
/* 
** Turn parser tracing on by giving a stream to which to write the trace
** and a prompt to preface each trace message.  Tracing is turned off
** by making either argument NULL 
**
** Inputs:
** <ul>
** <li> A FILE* to which trace output should be written.
**      If NULL, then tracing is turned off.
** <li> A prefix string written at the beginning of every
**      line of trace output.  If NULL, then tracing is
**      turned off.
** </ul>
**
** Outputs:
** None.
*/
void ParseTrace(FILE *TraceFILE, char *zTracePrompt){
  yyTraceFILE = TraceFILE;
  yyTracePrompt = zTracePrompt;
  if( yyTraceFILE==0 ) yyTracePrompt = 0;
  else if( yyTracePrompt==0 ) yyTraceFILE = 0;
}
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing shifts, the names of all terminals and nonterminals
** are required.  The following table supplies these names */
static const char *const yyTokenName[] = { 
  "$",             "OPEN",          "LEFT_PARANTHESIS",  "SEMICOLON",   
  "UNTIL",         "DO",            "END",           "WHILE",       
  "REPEAT",        "IF",            "FUNCTION",      "EQUAL",       
  "FOR",           "NAME",          "IN",            "ELSE",        
  "ELSEIF",        "THEN",          "BREAK",         "RETURN",      
  "LOCAL",         "LESS",          "CONST",         "GREATER",     
  "COLON",         "DOT",           "COMMA",         "OR",          
  "AND",           "LESS_EQUAL",    "GREATER_EQUAL",  "EQUAL_EQUAL", 
  "NOT_EQUAL",     "DOT_DOT",       "PLUS",          "MINUS",       
  "MULTIPLY",      "DIVIDE",        "MODULO",        "NOT",         
  "HASH",          "RAISE",         "NIL",           "TRUE",        
  "FALSE",         "NUMBER",        "STRING",        "DOT_DOT_DOT", 
  "BITWISE_OR",    "BITWISE_AND",   "LEFT_BRACKET",  "RIGHT_BRACKET",
  "RIGHT_PARANTHESIS",  "LEFT_BRACE",    "RIGHT_BRACE",   "error",       
  "chunk",         "block",         "semi",          "scope",       
  "statlist",      "laststat",      "ublock",        "exp",         
  "binding",       "stat",          "repetition",    "conds",       
  "funcname",      "funcbody",      "setlist",       "explist1",    
  "functioncall",  "explist23",     "namelist",      "condlist",    
  "cond",          "dottedname",    "function",      "prefixexp",   
  "tableconstructor",  "var",           "args",          "params",      
  "parlist",       "fieldlist",     "field",       
};
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *const yyRuleName[] = {
 /*   0 */ "chunk ::= block",
 /*   1 */ "semi ::= SEMICOLON",
 /*   2 */ "semi ::=",
 /*   3 */ "block ::= scope statlist",
 /*   4 */ "block ::= scope statlist laststat semi",
 /*   5 */ "ublock ::= block UNTIL exp",
 /*   6 */ "scope ::=",
 /*   7 */ "scope ::= scope statlist binding semi",
 /*   8 */ "statlist ::=",
 /*   9 */ "statlist ::= statlist stat semi",
 /*  10 */ "stat ::= DO block END",
 /*  11 */ "stat ::= WHILE exp DO block END",
 /*  12 */ "stat ::= repetition DO block END",
 /*  13 */ "stat ::= REPEAT ublock",
 /*  14 */ "stat ::= IF conds END",
 /*  15 */ "stat ::= FUNCTION funcname funcbody",
 /*  16 */ "stat ::= setlist EQUAL explist1",
 /*  17 */ "stat ::= functioncall",
 /*  18 */ "repetition ::= FOR NAME EQUAL explist23",
 /*  19 */ "repetition ::= FOR namelist IN explist1",
 /*  20 */ "conds ::= condlist",
 /*  21 */ "conds ::= condlist ELSE block",
 /*  22 */ "condlist ::= cond",
 /*  23 */ "condlist ::= condlist ELSEIF cond",
 /*  24 */ "cond ::= exp THEN block",
 /*  25 */ "laststat ::= BREAK",
 /*  26 */ "laststat ::= RETURN",
 /*  27 */ "laststat ::= RETURN explist1",
 /*  28 */ "binding ::= LOCAL namelist",
 /*  29 */ "binding ::= LOCAL namelist EQUAL explist1",
 /*  30 */ "binding ::= LOCAL namelist LESS CONST GREATER EQUAL explist1",
 /*  31 */ "binding ::= LOCAL FUNCTION NAME funcbody",
 /*  32 */ "funcname ::= dottedname",
 /*  33 */ "funcname ::= dottedname COLON NAME",
 /*  34 */ "dottedname ::= NAME",
 /*  35 */ "dottedname ::= dottedname DOT NAME",
 /*  36 */ "namelist ::= NAME",
 /*  37 */ "namelist ::= namelist COMMA NAME",
 /*  38 */ "explist1 ::= exp",
 /*  39 */ "explist1 ::= explist1 COMMA exp",
 /*  40 */ "explist23 ::= exp COMMA exp",
 /*  41 */ "explist23 ::= exp COMMA exp COMMA exp",
 /*  42 */ "exp ::= NIL|TRUE|FALSE|NUMBER|STRING|DOT_DOT_DOT",
 /*  43 */ "exp ::= function",
 /*  44 */ "exp ::= prefixexp",
 /*  45 */ "exp ::= tableconstructor",
 /*  46 */ "exp ::= NOT|HASH|MINUS exp",
 /*  47 */ "exp ::= exp OR exp",
 /*  48 */ "exp ::= exp AND exp",
 /*  49 */ "exp ::= exp LESS|LESS_EQUAL|GREATER|GREATER_EQUAL|EQUAL_EQUAL|NOT_EQUAL exp",
 /*  50 */ "exp ::= exp DOT_DOT exp",
 /*  51 */ "exp ::= exp BITWISE_OR exp",
 /*  52 */ "exp ::= exp BITWISE_AND exp",
 /*  53 */ "exp ::= exp PLUS|MINUS exp",
 /*  54 */ "exp ::= exp MULTIPLY|DIVIDE|MODULO exp",
 /*  55 */ "exp ::= exp RAISE exp",
 /*  56 */ "setlist ::= var",
 /*  57 */ "setlist ::= setlist COMMA var",
 /*  58 */ "var ::= NAME",
 /*  59 */ "var ::= prefixexp LEFT_BRACKET exp RIGHT_BRACKET",
 /*  60 */ "var ::= prefixexp DOT NAME",
 /*  61 */ "prefixexp ::= var",
 /*  62 */ "prefixexp ::= functioncall",
 /*  63 */ "prefixexp ::= OPEN exp RIGHT_PARANTHESIS",
 /*  64 */ "functioncall ::= prefixexp args",
 /*  65 */ "functioncall ::= prefixexp COLON NAME args",
 /*  66 */ "args ::= LEFT_PARANTHESIS RIGHT_PARANTHESIS",
 /*  67 */ "args ::= LEFT_PARANTHESIS explist1 RIGHT_PARANTHESIS",
 /*  68 */ "args ::= tableconstructor",
 /*  69 */ "args ::= STRING",
 /*  70 */ "function ::= FUNCTION funcbody",
 /*  71 */ "funcbody ::= params block END",
 /*  72 */ "params ::= LEFT_PARANTHESIS parlist RIGHT_PARANTHESIS",
 /*  73 */ "parlist ::=",
 /*  74 */ "parlist ::= namelist",
 /*  75 */ "parlist ::= DOT_DOT_DOT",
 /*  76 */ "parlist ::= namelist COMMA DOT_DOT_DOT",
 /*  77 */ "tableconstructor ::= LEFT_BRACE RIGHT_BRACE",
 /*  78 */ "tableconstructor ::= LEFT_BRACE fieldlist RIGHT_BRACE",
 /*  79 */ "tableconstructor ::= LEFT_BRACE fieldlist COMMA|SEMICOLON RIGHT_BRACE",
 /*  80 */ "fieldlist ::= field",
 /*  81 */ "fieldlist ::= fieldlist COMMA|SEMICOLON field",
 /*  82 */ "field ::= exp",
 /*  83 */ "field ::= NAME EQUAL exp",
 /*  84 */ "field ::= LEFT_BRACKET exp RIGHT_BRACKET EQUAL exp",
};
#endif /* NDEBUG */


#if YYSTACKDEPTH<=0
/*
** Try to increase the size of the parser stack.
*/
static void yyGrowStack(yyParser *p){
  int newSize;
  yyStackEntry *pNew;

  newSize = p->yystksz*2 + 100;
  pNew = realloc(p->yystack, newSize*sizeof(pNew[0]));
  if( pNew ){
    p->yystack = pNew;
    p->yystksz = newSize;
#ifndef NDEBUG
    if( yyTraceFILE ){
      fprintf(yyTraceFILE,"%sStack grows to %d entries!\n",
              yyTracePrompt, p->yystksz);
    }
#endif
  }
}
#endif

/* 
** This function allocates a new parser.
** The only argument is a pointer to a function which works like
** malloc.
**
** Inputs:
** A pointer to the function used to allocate memory.
**
** Outputs:
** A pointer to a parser.  This pointer is used in subsequent calls
** to Parse and ParseFree.
*/
void *ParseAlloc(void *(*mallocProc)(size_t)){
  yyParser *pParser;
  pParser = (yyParser*)(*mallocProc)( (size_t)sizeof(yyParser) );
  if( pParser ){
    pParser->yyidx = -1;
#ifdef YYTRACKMAXSTACKDEPTH
    pParser->yyidxMax = 0;
#endif
#if YYSTACKDEPTH<=0
    pParser->yystack = NULL;
    pParser->yystksz = 0;
    yyGrowStack(pParser);
#endif
  }
  return pParser;
}

/* The following function deletes the value associated with a
** symbol.  The symbol can be either a terminal or nonterminal.
** "yymajor" is the symbol code, and "yypminor" is a pointer to
** the value.
*/
static void yy_destructor(
  yyParser *yypParser,    /* The parser */
  YYCODETYPE yymajor,     /* Type code for object to destroy */
  YYMINORTYPE *yypminor   /* The object to be destroyed */
){
  ParseARG_FETCH;
  switch( yymajor ){
    /* Here is inserted the actions which take place when a
    ** terminal or non-terminal is destroyed.  This can happen
    ** when the symbol is popped from the stack during a
    ** reduce or during error processing or when a parser is 
    ** being destroyed before it is finished parsing.
    **
    ** Note: during a reduce, the only symbols destroyed are those
    ** which appear on the RHS of the rule, but which are not used
    ** inside the C code.
    */
      /* TERMINAL Destructor */
    case 1: /* OPEN */
    case 2: /* LEFT_PARANTHESIS */
    case 3: /* SEMICOLON */
    case 4: /* UNTIL */
    case 5: /* DO */
    case 6: /* END */
    case 7: /* WHILE */
    case 8: /* REPEAT */
    case 9: /* IF */
    case 10: /* FUNCTION */
    case 11: /* EQUAL */
    case 12: /* FOR */
    case 13: /* NAME */
    case 14: /* IN */
    case 15: /* ELSE */
    case 16: /* ELSEIF */
    case 17: /* THEN */
    case 18: /* BREAK */
    case 19: /* RETURN */
    case 20: /* LOCAL */
    case 21: /* LESS */
    case 22: /* CONST */
    case 23: /* GREATER */
    case 24: /* COLON */
    case 25: /* DOT */
    case 26: /* COMMA */
    case 27: /* OR */
    case 28: /* AND */
    case 29: /* LESS_EQUAL */
    case 30: /* GREATER_EQUAL */
    case 31: /* EQUAL_EQUAL */
    case 32: /* NOT_EQUAL */
    case 33: /* DOT_DOT */
    case 34: /* PLUS */
    case 35: /* MINUS */
    case 36: /* MULTIPLY */
    case 37: /* DIVIDE */
    case 38: /* MODULO */
    case 39: /* NOT */
    case 40: /* HASH */
    case 41: /* RAISE */
    case 42: /* NIL */
    case 43: /* TRUE */
    case 44: /* FALSE */
    case 45: /* NUMBER */
    case 46: /* STRING */
    case 47: /* DOT_DOT_DOT */
    case 48: /* BITWISE_OR */
    case 49: /* BITWISE_AND */
    case 50: /* LEFT_BRACKET */
    case 51: /* RIGHT_BRACKET */
    case 52: /* RIGHT_PARANTHESIS */
    case 53: /* LEFT_BRACE */
    case 54: /* RIGHT_BRACE */
{
#line 3 "LuaGrammar.y"
 delete (yypminor->yy0); 
#line 764 "LuaGrammar.cpp"
}
      break;
    default:  break;   /* If no destructor action specified: do nothing */
  }
}

/*
** Pop the parser's stack once.
**
** If there is a destructor routine associated with the token which
** is popped from the stack, then call it.
**
** Return the major token number for the symbol popped.
*/
static int yy_pop_parser_stack(yyParser *pParser){
  YYCODETYPE yymajor;
  yyStackEntry *yytos = &pParser->yystack[pParser->yyidx];

  if( pParser->yyidx<0 ) return 0;
#ifndef NDEBUG
  if( yyTraceFILE && pParser->yyidx>=0 ){
    fprintf(yyTraceFILE,"%sPopping %s\n",
      yyTracePrompt,
      yyTokenName[yytos->major]);
  }
#endif
  yymajor = yytos->major;
  yy_destructor(pParser, yymajor, &yytos->minor);
  pParser->yyidx--;
  return yymajor;
}

/* 
** Deallocate and destroy a parser.  Destructors are all called for
** all stack elements before shutting the parser down.
**
** Inputs:
** <ul>
** <li>  A pointer to the parser.  This should be a pointer
**       obtained from ParseAlloc.
** <li>  A pointer to a function used to reclaim memory obtained
**       from malloc.
** </ul>
*/
void ParseFree(
  void *p,                    /* The parser to be deleted */
  void (*freeProc)(void*)     /* Function used to reclaim memory */
){
  yyParser *pParser = (yyParser*)p;
  if( pParser==0 ) return;
  while( pParser->yyidx>=0 ) yy_pop_parser_stack(pParser);
#if YYSTACKDEPTH<=0
  free(pParser->yystack);
#endif
  (*freeProc)((void*)pParser);
}

/*
** Return the peak depth of the stack for a parser.
*/
#ifdef YYTRACKMAXSTACKDEPTH
int ParseStackPeak(void *p){
  yyParser *pParser = (yyParser*)p;
  return pParser->yyidxMax;
}
#endif

/*
** Find the appropriate action for a parser given the terminal
** look-ahead token iLookAhead.
**
** If the look-ahead token is YYNOCODE, then check to see if the action is
** independent of the look-ahead.  If it is, return the action, otherwise
** return YY_NO_ACTION.
*/
static int yy_find_shift_action(
  yyParser *pParser,        /* The parser */
  YYCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
  int stateno = pParser->yystack[pParser->yyidx].stateno;
 
  if( stateno>YY_SHIFT_COUNT
   || (i = yy_shift_ofst[stateno])==YY_SHIFT_USE_DFLT ){
    return yy_default[stateno];
  }
  assert( iLookAhead!=YYNOCODE );
  i += iLookAhead;
  if( i<0 || i>=YY_ACTTAB_COUNT || yy_lookahead[i]!=iLookAhead ){
    if( iLookAhead>0 ){
#ifdef YYFALLBACK
      YYCODETYPE iFallback;            /* Fallback token */
      if( iLookAhead<sizeof(yyFallback)/sizeof(yyFallback[0])
             && (iFallback = yyFallback[iLookAhead])!=0 ){
#ifndef NDEBUG
        if( yyTraceFILE ){
          fprintf(yyTraceFILE, "%sFALLBACK %s => %s\n",
             yyTracePrompt, yyTokenName[iLookAhead], yyTokenName[iFallback]);
        }
#endif
        return yy_find_shift_action(pParser, iFallback);
      }
#endif
#ifdef YYWILDCARD
      {
        int j = i - iLookAhead + YYWILDCARD;
        if( 
#if YY_SHIFT_MIN+YYWILDCARD<0
          j>=0 &&
#endif
#if YY_SHIFT_MAX+YYWILDCARD>=YY_ACTTAB_COUNT
          j<YY_ACTTAB_COUNT &&
#endif
          yy_lookahead[j]==YYWILDCARD
        ){
#ifndef NDEBUG
          if( yyTraceFILE ){
            fprintf(yyTraceFILE, "%sWILDCARD %s => %s\n",
               yyTracePrompt, yyTokenName[iLookAhead], yyTokenName[YYWILDCARD]);
          }
#endif /* NDEBUG */
          return yy_action[j];
        }
      }
#endif /* YYWILDCARD */
    }
    return yy_default[stateno];
  }else{
    return yy_action[i];
  }
}

/*
** Find the appropriate action for a parser given the non-terminal
** look-ahead token iLookAhead.
**
** If the look-ahead token is YYNOCODE, then check to see if the action is
** independent of the look-ahead.  If it is, return the action, otherwise
** return YY_NO_ACTION.
*/
static int yy_find_reduce_action(
  int stateno,              /* Current state number */
  YYCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
#ifdef YYERRORSYMBOL
  if( stateno>YY_REDUCE_COUNT ){
    return yy_default[stateno];
  }
#else
  assert( stateno<=YY_REDUCE_COUNT );
#endif
  i = yy_reduce_ofst[stateno];
  assert( i!=YY_REDUCE_USE_DFLT );
  assert( iLookAhead!=YYNOCODE );
  i += iLookAhead;
#ifdef YYERRORSYMBOL
  if( i<0 || i>=YY_ACTTAB_COUNT || yy_lookahead[i]!=iLookAhead ){
    return yy_default[stateno];
  }
#else
  assert( i>=0 && i<YY_ACTTAB_COUNT );
  assert( yy_lookahead[i]==iLookAhead );
#endif
  return yy_action[i];
}

/*
** The following routine is called if the stack overflows.
*/
static void yyStackOverflow(yyParser *yypParser, YYMINORTYPE *yypMinor){
   ParseARG_FETCH;
   yypParser->yyidx--;
#ifndef NDEBUG
   if( yyTraceFILE ){
     fprintf(yyTraceFILE,"%sStack Overflow!\n",yyTracePrompt);
   }
#endif
   while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
   /* Here code is inserted which will execute if the parser
   ** stack every overflows */
   ParseARG_STORE; /* Suppress warning about unused %extra_argument var */
}

/*
** Perform a shift action.
*/
static void yy_shift(
  yyParser *yypParser,          /* The parser to be shifted */
  int yyNewState,               /* The new state to shift in */
  int yyMajor,                  /* The major token to shift in */
  YYMINORTYPE *yypMinor         /* Pointer to the minor token to shift in */
){
  yyStackEntry *yytos;
  yypParser->yyidx++;
#ifdef YYTRACKMAXSTACKDEPTH
  if( yypParser->yyidx>yypParser->yyidxMax ){
    yypParser->yyidxMax = yypParser->yyidx;
  }
#endif
#if YYSTACKDEPTH>0 
  if( yypParser->yyidx>=YYSTACKDEPTH ){
    yyStackOverflow(yypParser, yypMinor);
    return;
  }
#else
  if( yypParser->yyidx>=yypParser->yystksz ){
    yyGrowStack(yypParser);
    if( yypParser->yyidx>=yypParser->yystksz ){
      yyStackOverflow(yypParser, yypMinor);
      return;
    }
  }
#endif
  yytos = &yypParser->yystack[yypParser->yyidx];
  yytos->stateno = (YYACTIONTYPE)yyNewState;
  yytos->major = (YYCODETYPE)yyMajor;
  yytos->minor = *yypMinor;
#ifndef NDEBUG
  if( yyTraceFILE && yypParser->yyidx>0 ){
    int i;
    fprintf(yyTraceFILE,"%sShift %d\n",yyTracePrompt,yyNewState);
    fprintf(yyTraceFILE,"%sStack:",yyTracePrompt);
    for(i=1; i<=yypParser->yyidx; i++)
      fprintf(yyTraceFILE," %s",yyTokenName[yypParser->yystack[i].major]);
    fprintf(yyTraceFILE,"\n");
  }
#endif
}

/* The following table contains information about every rule that
** is used during the reduce.
*/
static const struct {
  YYCODETYPE lhs;         /* Symbol on the left-hand side of the rule */
  unsigned char nrhs;     /* Number of right-hand side symbols in the rule */
} yyRuleInfo[] = {
  { 56, 1 },
  { 58, 1 },
  { 58, 0 },
  { 57, 2 },
  { 57, 4 },
  { 62, 3 },
  { 59, 0 },
  { 59, 4 },
  { 60, 0 },
  { 60, 3 },
  { 65, 3 },
  { 65, 5 },
  { 65, 4 },
  { 65, 2 },
  { 65, 3 },
  { 65, 3 },
  { 65, 3 },
  { 65, 1 },
  { 66, 4 },
  { 66, 4 },
  { 67, 1 },
  { 67, 3 },
  { 75, 1 },
  { 75, 3 },
  { 76, 3 },
  { 61, 1 },
  { 61, 1 },
  { 61, 2 },
  { 64, 2 },
  { 64, 4 },
  { 64, 7 },
  { 64, 4 },
  { 68, 1 },
  { 68, 3 },
  { 77, 1 },
  { 77, 3 },
  { 74, 1 },
  { 74, 3 },
  { 71, 1 },
  { 71, 3 },
  { 73, 3 },
  { 73, 5 },
  { 63, 1 },
  { 63, 1 },
  { 63, 1 },
  { 63, 1 },
  { 63, 2 },
  { 63, 3 },
  { 63, 3 },
  { 63, 3 },
  { 63, 3 },
  { 63, 3 },
  { 63, 3 },
  { 63, 3 },
  { 63, 3 },
  { 63, 3 },
  { 70, 1 },
  { 70, 3 },
  { 81, 1 },
  { 81, 4 },
  { 81, 3 },
  { 79, 1 },
  { 79, 1 },
  { 79, 3 },
  { 72, 2 },
  { 72, 4 },
  { 82, 2 },
  { 82, 3 },
  { 82, 1 },
  { 82, 1 },
  { 78, 2 },
  { 69, 3 },
  { 83, 3 },
  { 84, 0 },
  { 84, 1 },
  { 84, 1 },
  { 84, 3 },
  { 80, 2 },
  { 80, 3 },
  { 80, 4 },
  { 85, 1 },
  { 85, 3 },
  { 86, 1 },
  { 86, 3 },
  { 86, 5 },
};

static void yy_accept(yyParser*);  /* Forward Declaration */

/*
** Perform a reduce action and the shift that must immediately
** follow the reduce.
*/
static void yy_reduce(
  yyParser *yypParser,         /* The parser */
  int yyruleno                 /* Number of the rule by which to reduce */
){
  int yygoto;                     /* The next state */
  int yyact;                      /* The next action */
  YYMINORTYPE yygotominor;        /* The LHS of the rule reduced */
  yyStackEntry *yymsp;            /* The top of the parser's stack */
  int yysize;                     /* Amount to pop the stack */
  ParseARG_FETCH;
  yymsp = &yypParser->yystack[yypParser->yyidx];
#ifndef NDEBUG
  if( yyTraceFILE && yyruleno>=0 
        && yyruleno<(int)(sizeof(yyRuleName)/sizeof(yyRuleName[0])) ){
    fprintf(yyTraceFILE, "%sReduce [%s].\n", yyTracePrompt,
      yyRuleName[yyruleno]);
  }
#endif /* NDEBUG */

  /* Silence complaints from purify about yygotominor being uninitialized
  ** in some cases when it is copied into the stack after the following
  ** switch.  yygotominor is uninitialized when a rule reduces that does
  ** not set the value of its left-hand side nonterminal.  Leaving the
  ** value of the nonterminal uninitialized is utterly harmless as long
  ** as the value is never used.  So really the only thing this code
  ** accomplishes is to quieten purify.  
  **
  ** 2007-01-16:  The wireshark project (www.wireshark.org) reports that
  ** without this code, their parser segfaults.  I'm not sure what there
  ** parser is doing to make this happen.  This is the second bug report
  ** from wireshark this week.  Clearly they are stressing Lemon in ways
  ** that it has not been previously stressed...  (SQLite ticket #2172)
  */
  /*memset(&yygotominor, 0, sizeof(yygotominor));*/
  yygotominor = yyzerominor;


  switch( yyruleno ){
  /* Beginning here are the reduction cases.  A typical example
  ** follows:
  **   case 0:
  **  #line <lineno> <grammarfile>
  **     { ... }           // User supplied code
  **  #line <lineno> <thisfile>
  **     break;
  */
      case 0: /* chunk ::= block */
#line 86 "LuaGrammar.y"
{ outline->parseResult(yymsp[0].minor.yy0->node); }
#line 1144 "LuaGrammar.cpp"
        break;
      case 1: /* semi ::= SEMICOLON */
#line 88 "LuaGrammar.y"
{
  yy_destructor(yypParser,3,&yymsp[0].minor);
}
#line 1151 "LuaGrammar.cpp"
        break;
      case 3: /* block ::= scope statlist */
#line 91 "LuaGrammar.y"
{ yygotominor.yy0 = merge(yymsp[-1].minor.yy0, yymsp[0].minor.yy0); }
#line 1156 "LuaGrammar.cpp"
        break;
      case 4: /* block ::= scope statlist laststat semi */
      case 7: /* scope ::= scope statlist binding semi */ yytestcase(yyruleno==7);
#line 92 "LuaGrammar.y"
{ yygotominor.yy0 = merge(merge(yymsp[-3].minor.yy0, yymsp[-2].minor.yy0), yymsp[-1].minor.yy0); }
#line 1162 "LuaGrammar.cpp"
        break;
      case 5: /* ublock ::= block UNTIL exp */
#line 93 "LuaGrammar.y"
{ yygotominor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[0].minor.yy0);   yy_destructor(yypParser,4,&yymsp[-1].minor);
}
#line 1168 "LuaGrammar.cpp"
        break;
      case 6: /* scope ::= */
      case 8: /* statlist ::= */ yytestcase(yyruleno==8);
      case 42: /* exp ::= NIL|TRUE|FALSE|NUMBER|STRING|DOT_DOT_DOT */ yytestcase(yyruleno==42);
      case 45: /* exp ::= tableconstructor */ yytestcase(yyruleno==45);
      case 68: /* args ::= tableconstructor */ yytestcase(yyruleno==68);
#line 95 "LuaGrammar.y"
{ yygotominor.yy0 = null(); }
#line 1177 "LuaGrammar.cpp"
        break;
      case 9: /* statlist ::= statlist stat semi */
#line 99 "LuaGrammar.y"
{ yygotominor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[-1].minor.yy0); }
#line 1182 "LuaGrammar.cpp"
        break;
      case 10: /* stat ::= DO block END */
      case 12: /* stat ::= repetition DO block END */ yytestcase(yyruleno==12);
#line 101 "LuaGrammar.y"
{ yygotominor.yy0 = copy(yymsp[-1].minor.yy0);   yy_destructor(yypParser,5,&yymsp[-2].minor);
  yy_destructor(yypParser,6,&yymsp[0].minor);
}
#line 1190 "LuaGrammar.cpp"
        break;
      case 11: /* stat ::= WHILE exp DO block END */
#line 102 "LuaGrammar.y"
{ yygotominor.yy0 = copy(yymsp[-1].minor.yy0);   yy_destructor(yypParser,7,&yymsp[-4].minor);
  yy_destructor(yypParser,5,&yymsp[-2].minor);
  yy_destructor(yypParser,6,&yymsp[0].minor);
}
#line 1198 "LuaGrammar.cpp"
        break;
      case 13: /* stat ::= REPEAT ublock */
#line 104 "LuaGrammar.y"
{ yygotominor.yy0 = copy(yymsp[0].minor.yy0);   yy_destructor(yypParser,8,&yymsp[-1].minor);
}
#line 1204 "LuaGrammar.cpp"
        break;
      case 14: /* stat ::= IF conds END */
#line 105 "LuaGrammar.y"
{ yygotominor.yy0 = copy(yymsp[-1].minor.yy0);   yy_destructor(yypParser,9,&yymsp[-2].minor);
  yy_destructor(yypParser,6,&yymsp[0].minor);
}
#line 1211 "LuaGrammar.cpp"
        break;
      case 15: /* stat ::= FUNCTION funcname funcbody */
#line 106 "LuaGrammar.y"
{ yygotominor.yy0 = new LuaGrammarToken(new IScriptOutline::FunctionNode(yymsp[-1].minor.yy0->line, yymsp[-1].minor.yy0->text, false, yymsp[0].minor.yy0->node), yymsp[-1].minor.yy0->line);   yy_destructor(yypParser,10,&yymsp[-2].minor);
}
#line 1217 "LuaGrammar.cpp"
        break;
      case 16: /* stat ::= setlist EQUAL explist1 */
#line 107 "LuaGrammar.y"
{ yygotominor.yy0 = null();   yy_destructor(yypParser,11,&yymsp[-1].minor);
}
#line 1223 "LuaGrammar.cpp"
        break;
      case 17: /* stat ::= functioncall */
      case 20: /* conds ::= condlist */ yytestcase(yyruleno==20);
      case 22: /* condlist ::= cond */ yytestcase(yyruleno==22);
      case 32: /* funcname ::= dottedname */ yytestcase(yyruleno==32);
      case 34: /* dottedname ::= NAME */ yytestcase(yyruleno==34);
      case 36: /* namelist ::= NAME */ yytestcase(yyruleno==36);
      case 38: /* explist1 ::= exp */ yytestcase(yyruleno==38);
      case 43: /* exp ::= function */ yytestcase(yyruleno==43);
      case 44: /* exp ::= prefixexp */ yytestcase(yyruleno==44);
      case 46: /* exp ::= NOT|HASH|MINUS exp */ yytestcase(yyruleno==46);
      case 58: /* var ::= NAME */ yytestcase(yyruleno==58);
      case 61: /* prefixexp ::= var */ yytestcase(yyruleno==61);
      case 62: /* prefixexp ::= functioncall */ yytestcase(yyruleno==62);
#line 108 "LuaGrammar.y"
{ yygotominor.yy0 = copy(yymsp[0].minor.yy0); }
#line 1240 "LuaGrammar.cpp"
        break;
      case 18: /* repetition ::= FOR NAME EQUAL explist23 */
#line 110 "LuaGrammar.y"
{
  yy_destructor(yypParser,12,&yymsp[-3].minor);
  yy_destructor(yypParser,13,&yymsp[-2].minor);
  yy_destructor(yypParser,11,&yymsp[-1].minor);
}
#line 1249 "LuaGrammar.cpp"
        break;
      case 19: /* repetition ::= FOR namelist IN explist1 */
#line 111 "LuaGrammar.y"
{
  yy_destructor(yypParser,12,&yymsp[-3].minor);
  yy_destructor(yypParser,14,&yymsp[-1].minor);
}
#line 1257 "LuaGrammar.cpp"
        break;
      case 21: /* conds ::= condlist ELSE block */
#line 114 "LuaGrammar.y"
{ yygotominor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[0].minor.yy0);   yy_destructor(yypParser,15,&yymsp[-1].minor);
}
#line 1263 "LuaGrammar.cpp"
        break;
      case 23: /* condlist ::= condlist ELSEIF cond */
#line 116 "LuaGrammar.y"
{ yygotominor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[0].minor.yy0);   yy_destructor(yypParser,16,&yymsp[-1].minor);
}
#line 1269 "LuaGrammar.cpp"
        break;
      case 24: /* cond ::= exp THEN block */
#line 117 "LuaGrammar.y"
{ yygotominor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[0].minor.yy0);   yy_destructor(yypParser,17,&yymsp[-1].minor);
}
#line 1275 "LuaGrammar.cpp"
        break;
      case 25: /* laststat ::= BREAK */
#line 119 "LuaGrammar.y"
{ yygotominor.yy0 = null();   yy_destructor(yypParser,18,&yymsp[0].minor);
}
#line 1281 "LuaGrammar.cpp"
        break;
      case 26: /* laststat ::= RETURN */
#line 120 "LuaGrammar.y"
{ yygotominor.yy0 = null();   yy_destructor(yypParser,19,&yymsp[0].minor);
}
#line 1287 "LuaGrammar.cpp"
        break;
      case 27: /* laststat ::= RETURN explist1 */
#line 121 "LuaGrammar.y"
{ yygotominor.yy0 = copy(yymsp[0].minor.yy0);   yy_destructor(yypParser,19,&yymsp[-1].minor);
}
#line 1293 "LuaGrammar.cpp"
        break;
      case 28: /* binding ::= LOCAL namelist */
#line 123 "LuaGrammar.y"
{ yygotominor.yy0 = null();   yy_destructor(yypParser,20,&yymsp[-1].minor);
}
#line 1299 "LuaGrammar.cpp"
        break;
      case 29: /* binding ::= LOCAL namelist EQUAL explist1 */
#line 124 "LuaGrammar.y"
{ yygotominor.yy0 = copy(yymsp[0].minor.yy0);   yy_destructor(yypParser,20,&yymsp[-3].minor);
  yy_destructor(yypParser,11,&yymsp[-1].minor);
}
#line 1306 "LuaGrammar.cpp"
        break;
      case 30: /* binding ::= LOCAL namelist LESS CONST GREATER EQUAL explist1 */
#line 125 "LuaGrammar.y"
{ yygotominor.yy0 = copy(yymsp[0].minor.yy0);   yy_destructor(yypParser,20,&yymsp[-6].minor);
  yy_destructor(yypParser,21,&yymsp[-4].minor);
  yy_destructor(yypParser,22,&yymsp[-3].minor);
  yy_destructor(yypParser,23,&yymsp[-2].minor);
  yy_destructor(yypParser,11,&yymsp[-1].minor);
}
#line 1316 "LuaGrammar.cpp"
        break;
      case 31: /* binding ::= LOCAL FUNCTION NAME funcbody */
#line 126 "LuaGrammar.y"
{ yygotominor.yy0 = new LuaGrammarToken(new IScriptOutline::FunctionNode(yymsp[-1].minor.yy0->line, yymsp[-1].minor.yy0->text, true, yymsp[0].minor.yy0->node), yymsp[-1].minor.yy0->line);   yy_destructor(yypParser,20,&yymsp[-3].minor);
  yy_destructor(yypParser,10,&yymsp[-2].minor);
}
#line 1323 "LuaGrammar.cpp"
        break;
      case 33: /* funcname ::= dottedname COLON NAME */
#line 129 "LuaGrammar.y"
{ yygotominor.yy0 = new LuaGrammarToken(yymsp[-2].minor.yy0->text + L":" + yymsp[0].minor.yy0->text, yymsp[0].minor.yy0->line);   yy_destructor(yypParser,24,&yymsp[-1].minor);
}
#line 1329 "LuaGrammar.cpp"
        break;
      case 35: /* dottedname ::= dottedname DOT NAME */
#line 132 "LuaGrammar.y"
{ yygotominor.yy0 = new LuaGrammarToken(yymsp[-2].minor.yy0->text + L"." + yymsp[0].minor.yy0->text, yymsp[0].minor.yy0->line);   yy_destructor(yypParser,25,&yymsp[-1].minor);
}
#line 1335 "LuaGrammar.cpp"
        break;
      case 37: /* namelist ::= namelist COMMA NAME */
#line 135 "LuaGrammar.y"
{ yygotominor.yy0 = new LuaGrammarToken(yymsp[-2].minor.yy0->text + L"," + yymsp[0].minor.yy0->text, yymsp[0].minor.yy0->line);   yy_destructor(yypParser,26,&yymsp[-1].minor);
}
#line 1341 "LuaGrammar.cpp"
        break;
      case 39: /* explist1 ::= explist1 COMMA exp */
      case 40: /* explist23 ::= exp COMMA exp */ yytestcase(yyruleno==40);
#line 138 "LuaGrammar.y"
{ yygotominor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[0].minor.yy0);   yy_destructor(yypParser,26,&yymsp[-1].minor);
}
#line 1348 "LuaGrammar.cpp"
        break;
      case 41: /* explist23 ::= exp COMMA exp COMMA exp */
#line 140 "LuaGrammar.y"
{ yygotominor.yy0 = merge(merge(yymsp[-4].minor.yy0, yymsp[-2].minor.yy0), yymsp[0].minor.yy0);   yy_destructor(yypParser,26,&yymsp[-3].minor);
  yy_destructor(yypParser,26,&yymsp[-1].minor);
}
#line 1355 "LuaGrammar.cpp"
        break;
      case 47: /* exp ::= exp OR exp */
#line 156 "LuaGrammar.y"
{ yygotominor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[0].minor.yy0);   yy_destructor(yypParser,27,&yymsp[-1].minor);
}
#line 1361 "LuaGrammar.cpp"
        break;
      case 48: /* exp ::= exp AND exp */
#line 157 "LuaGrammar.y"
{ yygotominor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[0].minor.yy0);   yy_destructor(yypParser,28,&yymsp[-1].minor);
}
#line 1367 "LuaGrammar.cpp"
        break;
      case 49: /* exp ::= exp LESS|LESS_EQUAL|GREATER|GREATER_EQUAL|EQUAL_EQUAL|NOT_EQUAL exp */
      case 53: /* exp ::= exp PLUS|MINUS exp */ yytestcase(yyruleno==53);
      case 54: /* exp ::= exp MULTIPLY|DIVIDE|MODULO exp */ yytestcase(yyruleno==54);
#line 158 "LuaGrammar.y"
{ yygotominor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[0].minor.yy0); }
#line 1374 "LuaGrammar.cpp"
        break;
      case 50: /* exp ::= exp DOT_DOT exp */
#line 159 "LuaGrammar.y"
{ yygotominor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[0].minor.yy0);   yy_destructor(yypParser,33,&yymsp[-1].minor);
}
#line 1380 "LuaGrammar.cpp"
        break;
      case 51: /* exp ::= exp BITWISE_OR exp */
#line 160 "LuaGrammar.y"
{ yygotominor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[0].minor.yy0);   yy_destructor(yypParser,48,&yymsp[-1].minor);
}
#line 1386 "LuaGrammar.cpp"
        break;
      case 52: /* exp ::= exp BITWISE_AND exp */
#line 161 "LuaGrammar.y"
{ yygotominor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[0].minor.yy0);   yy_destructor(yypParser,49,&yymsp[-1].minor);
}
#line 1392 "LuaGrammar.cpp"
        break;
      case 55: /* exp ::= exp RAISE exp */
#line 164 "LuaGrammar.y"
{ yygotominor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[0].minor.yy0);   yy_destructor(yypParser,41,&yymsp[-1].minor);
}
#line 1398 "LuaGrammar.cpp"
        break;
      case 57: /* setlist ::= setlist COMMA var */
#line 167 "LuaGrammar.y"
{
  yy_destructor(yypParser,26,&yymsp[-1].minor);
}
#line 1405 "LuaGrammar.cpp"
        break;
      case 59: /* var ::= prefixexp LEFT_BRACKET exp RIGHT_BRACKET */
#line 170 "LuaGrammar.y"
{ yygotominor.yy0 = merge(yymsp[-3].minor.yy0, yymsp[-1].minor.yy0);   yy_destructor(yypParser,50,&yymsp[-2].minor);
  yy_destructor(yypParser,51,&yymsp[0].minor);
}
#line 1412 "LuaGrammar.cpp"
        break;
      case 60: /* var ::= prefixexp DOT NAME */
#line 171 "LuaGrammar.y"
{ yygotominor.yy0 = new LuaGrammarToken(as_string(yymsp[-2].minor.yy0) + L"." + yymsp[0].minor.yy0->text, yymsp[0].minor.yy0->line);   yy_destructor(yypParser,25,&yymsp[-1].minor);
}
#line 1418 "LuaGrammar.cpp"
        break;
      case 63: /* prefixexp ::= OPEN exp RIGHT_PARANTHESIS */
#line 175 "LuaGrammar.y"
{ yygotominor.yy0 = copy(yymsp[-1].minor.yy0);   yy_destructor(yypParser,1,&yymsp[-2].minor);
  yy_destructor(yypParser,52,&yymsp[0].minor);
}
#line 1425 "LuaGrammar.cpp"
        break;
      case 64: /* functioncall ::= prefixexp args */
#line 177 "LuaGrammar.y"
{ yygotominor.yy0 = merge(new LuaGrammarToken(new IScriptOutline::FunctionReferenceNode(yymsp[-1].minor.yy0->line, as_string(yymsp[-1].minor.yy0)), yymsp[-1].minor.yy0->line), yymsp[0].minor.yy0); }
#line 1430 "LuaGrammar.cpp"
        break;
      case 65: /* functioncall ::= prefixexp COLON NAME args */
#line 178 "LuaGrammar.y"
{ yygotominor.yy0 = merge(new LuaGrammarToken(new IScriptOutline::FunctionReferenceNode(yymsp[-1].minor.yy0->line, as_string(yymsp[-3].minor.yy0) + L":" + yymsp[-1].minor.yy0->text), yymsp[-1].minor.yy0->line), yymsp[0].minor.yy0);   yy_destructor(yypParser,24,&yymsp[-2].minor);
}
#line 1436 "LuaGrammar.cpp"
        break;
      case 66: /* args ::= LEFT_PARANTHESIS RIGHT_PARANTHESIS */
#line 180 "LuaGrammar.y"
{ yygotominor.yy0 = null();   yy_destructor(yypParser,2,&yymsp[-1].minor);
  yy_destructor(yypParser,52,&yymsp[0].minor);
}
#line 1443 "LuaGrammar.cpp"
        break;
      case 67: /* args ::= LEFT_PARANTHESIS explist1 RIGHT_PARANTHESIS */
#line 181 "LuaGrammar.y"
{ yygotominor.yy0 = copy(yymsp[-1].minor.yy0);   yy_destructor(yypParser,2,&yymsp[-2].minor);
  yy_destructor(yypParser,52,&yymsp[0].minor);
}
#line 1450 "LuaGrammar.cpp"
        break;
      case 69: /* args ::= STRING */
#line 183 "LuaGrammar.y"
{ yygotominor.yy0 = null();   yy_destructor(yypParser,46,&yymsp[0].minor);
}
#line 1456 "LuaGrammar.cpp"
        break;
      case 70: /* function ::= FUNCTION funcbody */
#line 185 "LuaGrammar.y"
{ yygotominor.yy0 = copy(yymsp[0].minor.yy0);   yy_destructor(yypParser,10,&yymsp[-1].minor);
}
#line 1462 "LuaGrammar.cpp"
        break;
      case 71: /* funcbody ::= params block END */
#line 187 "LuaGrammar.y"
{ yygotominor.yy0 = copy(yymsp[-1].minor.yy0);   yy_destructor(yypParser,6,&yymsp[0].minor);
}
#line 1468 "LuaGrammar.cpp"
        break;
      case 72: /* params ::= LEFT_PARANTHESIS parlist RIGHT_PARANTHESIS */
#line 189 "LuaGrammar.y"
{
  yy_destructor(yypParser,2,&yymsp[-2].minor);
  yy_destructor(yypParser,52,&yymsp[0].minor);
}
#line 1476 "LuaGrammar.cpp"
        break;
      case 75: /* parlist ::= DOT_DOT_DOT */
#line 193 "LuaGrammar.y"
{
  yy_destructor(yypParser,47,&yymsp[0].minor);
}
#line 1483 "LuaGrammar.cpp"
        break;
      case 76: /* parlist ::= namelist COMMA DOT_DOT_DOT */
#line 194 "LuaGrammar.y"
{
  yy_destructor(yypParser,26,&yymsp[-1].minor);
  yy_destructor(yypParser,47,&yymsp[0].minor);
}
#line 1491 "LuaGrammar.cpp"
        break;
      case 77: /* tableconstructor ::= LEFT_BRACE RIGHT_BRACE */
#line 196 "LuaGrammar.y"
{
  yy_destructor(yypParser,53,&yymsp[-1].minor);
  yy_destructor(yypParser,54,&yymsp[0].minor);
}
#line 1499 "LuaGrammar.cpp"
        break;
      case 78: /* tableconstructor ::= LEFT_BRACE fieldlist RIGHT_BRACE */
#line 197 "LuaGrammar.y"
{
  yy_destructor(yypParser,53,&yymsp[-2].minor);
  yy_destructor(yypParser,54,&yymsp[0].minor);
}
#line 1507 "LuaGrammar.cpp"
        break;
      case 79: /* tableconstructor ::= LEFT_BRACE fieldlist COMMA|SEMICOLON RIGHT_BRACE */
#line 198 "LuaGrammar.y"
{
  yy_destructor(yypParser,53,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[0].minor);
}
#line 1515 "LuaGrammar.cpp"
        break;
      case 83: /* field ::= NAME EQUAL exp */
#line 204 "LuaGrammar.y"
{
  yy_destructor(yypParser,13,&yymsp[-2].minor);
  yy_destructor(yypParser,11,&yymsp[-1].minor);
}
#line 1523 "LuaGrammar.cpp"
        break;
      case 84: /* field ::= LEFT_BRACKET exp RIGHT_BRACKET EQUAL exp */
#line 205 "LuaGrammar.y"
{
  yy_destructor(yypParser,50,&yymsp[-4].minor);
  yy_destructor(yypParser,51,&yymsp[-2].minor);
  yy_destructor(yypParser,11,&yymsp[-1].minor);
}
#line 1532 "LuaGrammar.cpp"
        break;
      default:
      /* (2) semi ::= */ yytestcase(yyruleno==2);
      /* (56) setlist ::= var */ yytestcase(yyruleno==56);
      /* (73) parlist ::= */ yytestcase(yyruleno==73);
      /* (74) parlist ::= namelist */ yytestcase(yyruleno==74);
      /* (80) fieldlist ::= field */ yytestcase(yyruleno==80);
      /* (81) fieldlist ::= fieldlist COMMA|SEMICOLON field */ yytestcase(yyruleno==81);
      /* (82) field ::= exp */ yytestcase(yyruleno==82);
        break;
  };
  yygoto = yyRuleInfo[yyruleno].lhs;
  yysize = yyRuleInfo[yyruleno].nrhs;
  yypParser->yyidx -= yysize;
  yyact = yy_find_reduce_action(yymsp[-yysize].stateno,(YYCODETYPE)yygoto);
  if( yyact < YYNSTATE ){
#ifdef NDEBUG
    /* If we are not debugging and the reduce action popped at least
    ** one element off the stack, then we can push the new element back
    ** onto the stack here, and skip the stack overflow test in yy_shift().
    ** That gives a significant speed improvement. */
    if( yysize ){
      yypParser->yyidx++;
      yymsp -= yysize-1;
      yymsp->stateno = (YYACTIONTYPE)yyact;
      yymsp->major = (YYCODETYPE)yygoto;
      yymsp->minor = yygotominor;
    }else
#endif
    {
      yy_shift(yypParser,yyact,yygoto,&yygotominor);
    }
  }else{
    assert( yyact == YYNSTATE + YYNRULE + 1 );
    yy_accept(yypParser);
  }
}

/*
** The following code executes when the parse fails
*/
#ifndef YYNOERRORRECOVERY
static void yy_parse_failed(
  yyParser *yypParser           /* The parser */
){
  ParseARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sFail!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser fails */
  ParseARG_STORE; /* Suppress warning about unused %extra_argument variable */
}
#endif /* YYNOERRORRECOVERY */

/*
** The following code executes when a syntax error first occurs.
*/
static void yy_syntax_error(
  yyParser *yypParser,           /* The parser */
  int yymajor,                   /* The major type of the error token */
  YYMINORTYPE yyminor            /* The minor type of the error token */
){
  ParseARG_FETCH;
#define TOKEN (yyminor.yy0)
#line 80 "LuaGrammar.y"


outline->syntaxError();

#line 1606 "LuaGrammar.cpp"
  ParseARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/*
** The following is executed when the parser accepts
*/
static void yy_accept(
  yyParser *yypParser           /* The parser */
){
  ParseARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sAccept!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser accepts */
  ParseARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/* The main parser program.
** The first argument is a pointer to a structure obtained from
** "ParseAlloc" which describes the current state of the parser.
** The second argument is the major token number.  The third is
** the minor token.  The fourth optional argument is whatever the
** user wants (and specified in the grammar) and is available for
** use by the action routines.
**
** Inputs:
** <ul>
** <li> A pointer to the parser (an opaque structure.)
** <li> The major token number.
** <li> The minor token number.
** <li> An option argument of a grammar-specified type.
** </ul>
**
** Outputs:
** None.
*/
void Parse(
  void *yyp,                   /* The parser */
  int yymajor,                 /* The major token code number */
  ParseTOKENTYPE yyminor       /* The value for the token */
  ParseARG_PDECL               /* Optional %extra_argument parameter */
){
  YYMINORTYPE yyminorunion;
  int yyact;            /* The parser action. */
  int yyendofinput;     /* True if we are at the end of input */
#ifdef YYERRORSYMBOL
  int yyerrorhit = 0;   /* True if yymajor has invoked an error */
#endif
  yyParser *yypParser;  /* The parser */

  /* (re)initialize the parser, if necessary */
  yypParser = (yyParser*)yyp;
  if( yypParser->yyidx<0 ){
#if YYSTACKDEPTH<=0
    if( yypParser->yystksz <=0 ){
      /*memset(&yyminorunion, 0, sizeof(yyminorunion));*/
      yyminorunion = yyzerominor;
      yyStackOverflow(yypParser, &yyminorunion);
      return;
    }
#endif
    yypParser->yyidx = 0;
    yypParser->yyerrcnt = -1;
    yypParser->yystack[0].stateno = 0;
    yypParser->yystack[0].major = 0;
  }
  yyminorunion.yy0 = yyminor;
  yyendofinput = (yymajor==0);
  ParseARG_STORE;

#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sInput %s\n",yyTracePrompt,yyTokenName[yymajor]);
  }
#endif

  do{
    yyact = yy_find_shift_action(yypParser,(YYCODETYPE)yymajor);
    if( yyact<YYNSTATE ){
      assert( !yyendofinput );  /* Impossible to shift the $ token */
      yy_shift(yypParser,yyact,yymajor,&yyminorunion);
      yypParser->yyerrcnt--;
      yymajor = YYNOCODE;
    }else if( yyact < YYNSTATE + YYNRULE ){
      yy_reduce(yypParser,yyact-YYNSTATE);
    }else{
      assert( yyact == YY_ERROR_ACTION );
#ifdef YYERRORSYMBOL
      int yymx;
#endif
#ifndef NDEBUG
      if( yyTraceFILE ){
        fprintf(yyTraceFILE,"%sSyntax Error!\n",yyTracePrompt);
      }
#endif
#ifdef YYERRORSYMBOL
      /* A syntax error has occurred.
      ** The response to an error depends upon whether or not the
      ** grammar defines an error token "ERROR".  
      **
      ** This is what we do if the grammar does define ERROR:
      **
      **  * Call the %syntax_error function.
      **
      **  * Begin popping the stack until we enter a state where
      **    it is legal to shift the error symbol, then shift
      **    the error symbol.
      **
      **  * Set the error count to three.
      **
      **  * Begin accepting and shifting new tokens.  No new error
      **    processing will occur until three tokens have been
      **    shifted successfully.
      **
      */
      if( yypParser->yyerrcnt<0 ){
        yy_syntax_error(yypParser,yymajor,yyminorunion);
      }
      yymx = yypParser->yystack[yypParser->yyidx].major;
      if( yymx==YYERRORSYMBOL || yyerrorhit ){
#ifndef NDEBUG
        if( yyTraceFILE ){
          fprintf(yyTraceFILE,"%sDiscard input token %s\n",
             yyTracePrompt,yyTokenName[yymajor]);
        }
#endif
        yy_destructor(yypParser, (YYCODETYPE)yymajor,&yyminorunion);
        yymajor = YYNOCODE;
      }else{
         while(
          yypParser->yyidx >= 0 &&
          yymx != YYERRORSYMBOL &&
          (yyact = yy_find_reduce_action(
                        yypParser->yystack[yypParser->yyidx].stateno,
                        YYERRORSYMBOL)) >= YYNSTATE
        ){
          yy_pop_parser_stack(yypParser);
        }
        if( yypParser->yyidx < 0 || yymajor==0 ){
          yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
          yy_parse_failed(yypParser);
          yymajor = YYNOCODE;
        }else if( yymx!=YYERRORSYMBOL ){
          YYMINORTYPE u2;
          u2.YYERRSYMDT = 0;
          yy_shift(yypParser,yyact,YYERRORSYMBOL,&u2);
        }
      }
      yypParser->yyerrcnt = 3;
      yyerrorhit = 1;
#elif defined(YYNOERRORRECOVERY)
      /* If the YYNOERRORRECOVERY macro is defined, then do not attempt to
      ** do any kind of error recovery.  Instead, simply invoke the syntax
      ** error routine and continue going as if nothing had happened.
      **
      ** Applications can set this macro (for example inside %include) if
      ** they intend to abandon the parse upon the first syntax error seen.
      */
      yy_syntax_error(yypParser,yymajor,yyminorunion);
      yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
      yymajor = YYNOCODE;
      
#else  /* YYERRORSYMBOL is not defined */
      /* This is what we do if the grammar does not define ERROR:
      **
      **  * Report an error message, and throw away the input token.
      **
      **  * If the input token is $, then fail the parse.
      **
      ** As before, subsequent error messages are suppressed until
      ** three input tokens have been successfully shifted.
      */
      if( yypParser->yyerrcnt<=0 ){
        yy_syntax_error(yypParser,yymajor,yyminorunion);
      }
      yypParser->yyerrcnt = 3;
      yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
      if( yyendofinput ){
        yy_parse_failed(yypParser);
      }
      yymajor = YYNOCODE;
#endif
    }
  }while( yymajor!=YYNOCODE && yypParser->yyidx>=0 );
  return;
}