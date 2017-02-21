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
#define YYNOCODE 87
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
#define YYNSTATE 159
#define YYNRULE 84
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
#define YY_ACTTAB_COUNT (731)
static const YYACTIONTYPE yy_action[] = {
 /*     0 */    46,   25,   22,   22,   21,   21,   21,   20,  110,   20,
 /*    10 */    53,   43,   11,    4,   24,   23,   24,   23,  154,   30,
 /*    20 */    27,   26,   26,   26,   26,   26,   26,   25,   22,   22,
 /*    30 */    21,   21,   21,    4,   99,   20,   53,  244,  111,  124,
 /*    40 */    53,   15,   24,   23,   30,   27,   26,   26,   26,   26,
 /*    50 */    26,   26,   25,   22,   22,   21,   21,   21,  142,  143,
 /*    60 */    20,   97,   96,  137,  109,  159,   53,   24,   23,  129,
 /*    70 */   220,  146,   30,   27,   26,   26,   26,   26,   26,   26,
 /*    80 */    25,   22,   22,   21,   21,   21,  153,  144,   20,  220,
 /*    90 */   220,  141,  139,   88,   89,   24,   23,  143,  104,   30,
 /*   100 */    27,   26,   26,   26,   26,   26,   26,   25,   22,   22,
 /*   110 */    21,   21,   21,  220,  108,   20,   53,  220,  140,   42,
 /*   120 */   220,   18,   24,   23,   16,  133,   30,   27,   26,   26,
 /*   130 */    26,   26,   26,   26,   25,   22,   22,   21,   21,   21,
 /*   140 */   132,  100,   20,   53,   36,  122,  120,   53,   53,   24,
 /*   150 */    23,   13,   30,   27,   26,   26,   26,   26,   26,   26,
 /*   160 */    25,   22,   22,   21,   21,   21,   44,   15,   20,  125,
 /*   170 */   127,  127,  130,  131,   14,   24,   23,   12,   30,   27,
 /*   180 */    26,   26,   26,   26,   26,   26,   25,   22,   22,   21,
 /*   190 */    21,   21,  123,  117,   20,  116,    9,    3,  157,  158,
 /*   200 */   142,   24,   23,   30,   27,   26,   26,   26,   26,   26,
 /*   210 */    26,   25,   22,   22,   21,   21,   21,   38,   82,   20,
 /*   220 */    94,  156,    5,  155,   81,   37,   24,   23,   27,   26,
 /*   230 */    26,   26,   26,   26,   26,   25,   22,   22,   21,   21,
 /*   240 */    21,  115,  245,   20,  245,  245,  245,  245,  245,  245,
 /*   250 */    24,   23,  145,   26,   26,   26,   26,   26,   26,   25,
 /*   260 */    22,   22,   21,   21,   21,  126,   45,   20,  245,   19,
 /*   270 */   245,  245,    2,  245,   24,   23,   19,   10,   41,    5,
 /*   280 */    47,  105,   31,   33,    1,   40,  245,   49,  149,   32,
 /*   290 */   107,  215,    8,  113,    7,   48,    6,  245,  102,  103,
 /*   300 */   106,   92,   28,  215,  214,  245,   28,   28,   92,  152,
 /*   310 */   152,  152,  152,  152,  152,   19,  214,   17,  245,   56,
 /*   320 */     2,  136,  126,   98,   41,  245,   29,  105,  147,    2,
 /*   330 */   118,   86,  119,  112,  151,   35,  150,  148,   21,   21,
 /*   340 */    21,  245,   19,   20,   45,  245,  245,   45,   28,  245,
 /*   350 */    24,   23,   28,   28,  149,  152,  152,  152,  152,  152,
 /*   360 */   152,   19,  245,   17,  245,  245,    2,  138,  245,   64,
 /*   370 */    41,   52,  245,  149,   51,   50,  101,   95,  147,  245,
 /*   380 */    84,  147,   74,   39,  151,   35,  150,  148,   34,   34,
 /*   390 */    83,   80,   85,  245,   28,  245,  245,  245,   28,   28,
 /*   400 */   245,  152,  152,  152,  152,  152,  152,   19,  245,  245,
 /*   410 */   245,  128,    2,  245,  245,   67,   41,  245,  245,  149,
 /*   420 */   245,  245,  245,  245,  147,  245,  245,  245,  245,  245,
 /*   430 */   151,   35,  150,  148,  245,  245,  245,   76,  134,  245,
 /*   440 */    28,  245,  245,  245,   28,   28,  245,  152,  152,  152,
 /*   450 */   152,  152,  152,  245,   56,  245,  245,  245,    2,  245,
 /*   460 */   245,  245,  245,  147,   55,  245,  245,  121,  245,  151,
 /*   470 */    35,  150,  148,  147,  114,   64,  245,  245,  245,  151,
 /*   480 */    35,  150,  148,   93,  147,  245,  245,   64,  245,  245,
 /*   490 */   151,   35,  150,  148,  245,   91,  147,  245,  245,  245,
 /*   500 */    64,  245,  151,   35,  150,  148,  245,  245,   90,  147,
 /*   510 */   245,   64,  245,  245,  245,  151,   35,  150,  148,   87,
 /*   520 */   147,   67,  245,  245,  245,  245,  151,   35,  150,  148,
 /*   530 */   147,  245,   60,  245,  245,  245,  151,   35,  150,  148,
 /*   540 */   245,  147,   70,  245,  135,  245,   57,  151,   35,  150,
 /*   550 */   148,  147,  245,  245,  245,  147,   79,  151,   35,  150,
 /*   560 */   148,  151,   35,  150,  148,  147,   71,  245,  245,  245,
 /*   570 */   245,  151,   35,  150,  148,  147,  245,  245,   73,  245,
 /*   580 */   245,  151,   35,  150,  148,  245,  245,  147,  245,   72,
 /*   590 */   245,  245,  245,  151,   35,  150,  148,  245,  147,   69,
 /*   600 */   245,  245,  245,  245,  151,   35,  150,  148,  147,   68,
 /*   610 */   245,  245,  245,   75,  151,   35,  150,  148,  147,  245,
 /*   620 */   245,  245,  147,   78,  151,   35,  150,  148,  151,   35,
 /*   630 */   150,  148,  147,  245,   77,  245,  245,  245,  151,   35,
 /*   640 */   150,  148,  245,  147,   59,  245,  245,  245,  245,  151,
 /*   650 */    35,  150,  148,  147,  245,  245,   66,  245,  245,  151,
 /*   660 */    35,  150,  148,  245,  245,  147,  245,   58,  245,  245,
 /*   670 */   245,  151,   35,  150,  148,  245,  147,   65,  245,  245,
 /*   680 */   245,   63,  151,   35,  150,  148,  147,  245,  245,  245,
 /*   690 */   147,   62,  151,   35,  150,  148,  151,   35,  150,  148,
 /*   700 */   147,   54,  245,  245,  245,  245,  151,   35,  150,  148,
 /*   710 */   147,  245,   61,  245,  245,  245,  151,   35,  150,  148,
 /*   720 */   245,  147,  245,  245,  245,  245,  245,  151,   35,  150,
 /*   730 */   148,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */     5,   32,   33,   34,   35,   36,   37,   40,   56,   40,
 /*    10 */    58,   15,   16,    3,   47,   48,   47,   48,    6,   24,
 /*    20 */    25,   26,   27,   28,   29,   30,   31,   32,   33,   34,
 /*    30 */    35,   36,   37,   23,   56,   40,   58,   55,   56,   61,
 /*    40 */    58,   23,   47,   48,   24,   25,   26,   27,   28,   29,
 /*    50 */    30,   31,   32,   33,   34,   35,   36,   37,   13,   13,
 /*    60 */    40,   21,   22,   53,   56,    0,   58,   47,   48,   51,
 /*    70 */     2,   51,   24,   25,   26,   27,   28,   29,   30,   31,
 /*    80 */    32,   33,   34,   35,   36,   37,    6,    6,   40,   21,
 /*    90 */    22,   46,   46,   23,   10,   47,   48,   13,   50,   24,
 /*   100 */    25,   26,   27,   28,   29,   30,   31,   32,   33,   34,
 /*   110 */    35,   36,   37,   45,   56,   40,   58,   49,   51,   17,
 /*   120 */    52,   11,   47,   48,   11,   50,   24,   25,   26,   27,
 /*   130 */    28,   29,   30,   31,   32,   33,   34,   35,   36,   37,
 /*   140 */    13,   56,   40,   58,   13,   56,   56,   58,   58,   47,
 /*   150 */    48,   23,   24,   25,   26,   27,   28,   29,   30,   31,
 /*   160 */    32,   33,   34,   35,   36,   37,    5,   23,   40,    6,
 /*   170 */    79,   79,   81,   81,    4,   47,   48,   23,   24,   25,
 /*   180 */    26,   27,   28,   29,   30,   31,   32,   33,   34,   35,
 /*   190 */    36,   37,    6,   13,   40,   13,   11,   59,   57,    3,
 /*   200 */    13,   47,   48,   24,   25,   26,   27,   28,   29,   30,
 /*   210 */    31,   32,   33,   34,   35,   36,   37,   13,   73,   40,
 /*   220 */    13,   57,    2,   57,   73,    2,   47,   48,   25,   26,
 /*   230 */    27,   28,   29,   30,   31,   32,   33,   34,   35,   36,
 /*   240 */    37,   13,   86,   40,   86,   86,   86,   86,   86,   86,
 /*   250 */    47,   48,   68,   26,   27,   28,   29,   30,   31,   32,
 /*   260 */    33,   34,   35,   36,   37,   45,   82,   40,   86,    1,
 /*   270 */    86,   86,   52,   86,   47,   48,    1,   11,   10,    2,
 /*   280 */     5,   13,    7,    8,    9,   10,   86,   12,   13,   23,
 /*   290 */    73,   11,   14,   18,   19,   20,   11,   86,   21,   22,
 /*   300 */    83,   23,   34,   23,   11,   86,   38,   39,   23,   41,
 /*   310 */    42,   43,   44,   45,   46,    1,   23,   49,   86,   62,
 /*   320 */    52,   53,   45,   66,   10,   86,   49,   13,   71,   52,
 /*   330 */    68,   74,   75,   68,   77,   78,   79,   80,   35,   36,
 /*   340 */    37,   86,    1,   40,   82,   86,   86,   82,   34,   86,
 /*   350 */    47,   48,   38,   39,   13,   41,   42,   43,   44,   45,
 /*   360 */    46,    1,   86,   49,   86,   86,   52,   53,   86,   62,
 /*   370 */    10,   60,   86,   13,   63,   64,   65,   70,   71,   86,
 /*   380 */    69,   71,   71,   67,   77,   78,   79,   80,   78,   78,
 /*   390 */    80,   80,   76,   86,   34,   86,   86,   86,   38,   39,
 /*   400 */    86,   41,   42,   43,   44,   45,   46,    1,   86,   86,
 /*   410 */    86,   51,   52,   86,   86,   62,   10,   86,   86,   13,
 /*   420 */    86,   86,   86,   86,   71,   86,   86,   86,   86,   86,
 /*   430 */    77,   78,   79,   80,   86,   86,   86,   84,   85,   86,
 /*   440 */    34,   86,   86,   86,   38,   39,   86,   41,   42,   43,
 /*   450 */    44,   45,   46,   86,   62,   86,   86,   86,   52,   86,
 /*   460 */    86,   86,   86,   71,   62,   86,   86,   75,   86,   77,
 /*   470 */    78,   79,   80,   71,   72,   62,   86,   86,   86,   77,
 /*   480 */    78,   79,   80,   70,   71,   86,   86,   62,   86,   86,
 /*   490 */    77,   78,   79,   80,   86,   70,   71,   86,   86,   86,
 /*   500 */    62,   86,   77,   78,   79,   80,   86,   86,   70,   71,
 /*   510 */    86,   62,   86,   86,   86,   77,   78,   79,   80,   70,
 /*   520 */    71,   62,   86,   86,   86,   86,   77,   78,   79,   80,
 /*   530 */    71,   86,   62,   86,   86,   86,   77,   78,   79,   80,
 /*   540 */    86,   71,   62,   86,   85,   86,   62,   77,   78,   79,
 /*   550 */    80,   71,   86,   86,   86,   71,   62,   77,   78,   79,
 /*   560 */    80,   77,   78,   79,   80,   71,   62,   86,   86,   86,
 /*   570 */    86,   77,   78,   79,   80,   71,   86,   86,   62,   86,
 /*   580 */    86,   77,   78,   79,   80,   86,   86,   71,   86,   62,
 /*   590 */    86,   86,   86,   77,   78,   79,   80,   86,   71,   62,
 /*   600 */    86,   86,   86,   86,   77,   78,   79,   80,   71,   62,
 /*   610 */    86,   86,   86,   62,   77,   78,   79,   80,   71,   86,
 /*   620 */    86,   86,   71,   62,   77,   78,   79,   80,   77,   78,
 /*   630 */    79,   80,   71,   86,   62,   86,   86,   86,   77,   78,
 /*   640 */    79,   80,   86,   71,   62,   86,   86,   86,   86,   77,
 /*   650 */    78,   79,   80,   71,   86,   86,   62,   86,   86,   77,
 /*   660 */    78,   79,   80,   86,   86,   71,   86,   62,   86,   86,
 /*   670 */    86,   77,   78,   79,   80,   86,   71,   62,   86,   86,
 /*   680 */    86,   62,   77,   78,   79,   80,   71,   86,   86,   86,
 /*   690 */    71,   62,   77,   78,   79,   80,   77,   78,   79,   80,
 /*   700 */    71,   62,   86,   86,   86,   86,   77,   78,   79,   80,
 /*   710 */    71,   86,   62,   86,   86,   86,   77,   78,   79,   80,
 /*   720 */    86,   71,   86,   86,   86,   86,   86,   77,   78,   79,
 /*   730 */    80,
};
#define YY_SHIFT_USE_DFLT (-34)
#define YY_SHIFT_COUNT (111)
#define YY_SHIFT_MIN   (-33)
#define YY_SHIFT_MAX   (406)
static const short yy_shift_ofst[] = {
 /*     0 */   -34,  406,  314,  275,  268,  360,  406,  406,  406,  406,
 /*    10 */   406,  406,  406,  406,  406,  406,  406,  406,  406,  406,
 /*    20 */   406,  406,  406,  406,  406,  406,  406,  406,  406,  406,
 /*    30 */   406,  406,  341,  -34,  277,  277,  220,   46,  223,  223,
 /*    40 */   228,  223,  -34,  -34,  -34,  -34,  -34,  -34,   84,  207,
 /*    50 */   196,  196,  196,  -34,  154,  128,  102,   75,   48,   20,
 /*    60 */    -5,  179,  179,  179,  179,  179,  179,  179,  179,  179,
 /*    70 */   203,  227,  -31,  -31,   68,  303,   10,  -33,  -33,  -33,
 /*    80 */   293,  285,  278,  280,  266,   40,   -4,   18,   45,  204,
 /*    90 */   144,  144,  187,  144,  185,  144,  182,  180,  186,  170,
 /*   100 */   163,  161,  131,  127,  113,  110,   67,   70,   81,   80,
 /*   110 */    12,   65,
};
#define YY_REDUCE_USE_DFLT (-49)
#define YY_REDUCE_COUNT (53)
#define YY_REDUCE_MIN   (-48)
#define YY_REDUCE_MAX   (650)
static const short yy_reduce_ofst[] = {
 /*     0 */   -18,  257,  353,  311,  459,  449,  438,  425,  413,  402,
 /*    10 */   307,  392,  650,  639,  629,  619,  615,  605,  594,  582,
 /*    20 */   572,  561,  551,  547,  537,  527,  516,  504,  494,  484,
 /*    30 */   480,  470,  310,  -22,   92,   92,   91,  217,  265,  262,
 /*    40 */   316,  184,   90,   89,   85,   58,    8,  -48,  151,  145,
 /*    50 */   166,  164,  141,  138,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   165,  243,  243,  162,  243,  243,  243,  185,  243,  243,
 /*    10 */   243,  243,  243,  243,  243,  243,  243,  243,  243,  243,
 /*    20 */   243,  243,  243,  243,  243,  243,  243,  243,  243,  243,
 /*    30 */   243,  243,  243,  165,  243,  202,  243,  231,  243,  243,
 /*    40 */   243,  243,  165,  165,  165,  165,  165,  165,  243,  243,
 /*    50 */   161,  161,  161,  167,  198,  243,  243,  243,  243,  243,
 /*    60 */   243,  199,  164,  197,  196,  242,  241,  240,  210,  209,
 /*    70 */   205,  206,  208,  207,  176,  211,  243,  213,  212,  204,
 /*    80 */   219,  187,  243,  219,  243,  190,  179,  243,  243,  243,
 /*    90 */   188,  186,  243,  178,  194,  175,  243,  243,  243,  243,
 /*   100 */   243,  243,  243,  243,  243,  216,  243,  232,  243,  243,
 /*   110 */   243,  243,  189,  184,  177,  192,  193,  191,  174,  181,
 /*   120 */   183,  182,  180,  173,  172,  171,  227,  226,  224,  225,
 /*   130 */   223,  222,  218,  217,  238,  239,  237,  236,  235,  233,
 /*   140 */   230,  234,  195,  194,  229,  228,  221,  220,  219,  216,
 /*   150 */   203,  201,  200,  170,  169,  168,  166,  163,  160,
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
  "LOCAL",         "COLON",         "DOT",           "COMMA",       
  "OR",            "AND",           "LESS",          "LESS_EQUAL",  
  "GREATER",       "GREATER_EQUAL",  "EQUAL_EQUAL",   "NOT_EQUAL",   
  "DOT_DOT",       "PLUS",          "MINUS",         "MULTIPLY",    
  "DIVIDE",        "MODULO",        "NOT",           "HASH",        
  "RAISE",         "NIL",           "TRUE",          "FALSE",       
  "NUMBER",        "STRING",        "DOT_DOT_DOT",   "BITWISE_OR",  
  "BITWISE_AND",   "LEFT_BRACKET",  "RIGHT_BRACKET",  "RIGHT_PARANTHESIS",
  "LEFT_BRACE",    "RIGHT_BRACE",   "error",         "chunk",       
  "block",         "semi",          "scope",         "statlist",    
  "laststat",      "ublock",        "exp",           "binding",     
  "stat",          "repetition",    "conds",         "funcname",    
  "funcbody",      "setlist",       "explist1",      "functioncall",
  "explist23",     "namelist",      "condlist",      "cond",        
  "dottedname",    "function",      "prefixexp",     "tableconstructor",
  "var",           "args",          "params",        "parlist",     
  "fieldlist",     "field",       
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
 /*  30 */ "binding ::= LOCAL FUNCTION NAME funcbody",
 /*  31 */ "funcname ::= dottedname",
 /*  32 */ "funcname ::= dottedname COLON NAME",
 /*  33 */ "dottedname ::= NAME",
 /*  34 */ "dottedname ::= dottedname DOT NAME",
 /*  35 */ "namelist ::= NAME",
 /*  36 */ "namelist ::= namelist COMMA NAME",
 /*  37 */ "explist1 ::= exp",
 /*  38 */ "explist1 ::= explist1 COMMA exp",
 /*  39 */ "explist23 ::= exp COMMA exp",
 /*  40 */ "explist23 ::= exp COMMA exp COMMA exp",
 /*  41 */ "exp ::= NIL|TRUE|FALSE|NUMBER|STRING|DOT_DOT_DOT",
 /*  42 */ "exp ::= function",
 /*  43 */ "exp ::= prefixexp",
 /*  44 */ "exp ::= tableconstructor",
 /*  45 */ "exp ::= NOT|HASH|MINUS exp",
 /*  46 */ "exp ::= exp OR exp",
 /*  47 */ "exp ::= exp AND exp",
 /*  48 */ "exp ::= exp LESS|LESS_EQUAL|GREATER|GREATER_EQUAL|EQUAL_EQUAL|NOT_EQUAL exp",
 /*  49 */ "exp ::= exp DOT_DOT exp",
 /*  50 */ "exp ::= exp BITWISE_OR exp",
 /*  51 */ "exp ::= exp BITWISE_AND exp",
 /*  52 */ "exp ::= exp PLUS|MINUS exp",
 /*  53 */ "exp ::= exp MULTIPLY|DIVIDE|MODULO exp",
 /*  54 */ "exp ::= exp RAISE exp",
 /*  55 */ "setlist ::= var",
 /*  56 */ "setlist ::= setlist COMMA var",
 /*  57 */ "var ::= NAME",
 /*  58 */ "var ::= prefixexp LEFT_BRACKET exp RIGHT_BRACKET",
 /*  59 */ "var ::= prefixexp DOT NAME",
 /*  60 */ "prefixexp ::= var",
 /*  61 */ "prefixexp ::= functioncall",
 /*  62 */ "prefixexp ::= OPEN exp RIGHT_PARANTHESIS",
 /*  63 */ "functioncall ::= prefixexp args",
 /*  64 */ "functioncall ::= prefixexp COLON NAME args",
 /*  65 */ "args ::= LEFT_PARANTHESIS RIGHT_PARANTHESIS",
 /*  66 */ "args ::= LEFT_PARANTHESIS explist1 RIGHT_PARANTHESIS",
 /*  67 */ "args ::= tableconstructor",
 /*  68 */ "args ::= STRING",
 /*  69 */ "function ::= FUNCTION funcbody",
 /*  70 */ "funcbody ::= params block END",
 /*  71 */ "params ::= LEFT_PARANTHESIS parlist RIGHT_PARANTHESIS",
 /*  72 */ "parlist ::=",
 /*  73 */ "parlist ::= namelist",
 /*  74 */ "parlist ::= DOT_DOT_DOT",
 /*  75 */ "parlist ::= namelist COMMA DOT_DOT_DOT",
 /*  76 */ "tableconstructor ::= LEFT_BRACE RIGHT_BRACE",
 /*  77 */ "tableconstructor ::= LEFT_BRACE fieldlist RIGHT_BRACE",
 /*  78 */ "tableconstructor ::= LEFT_BRACE fieldlist COMMA|SEMICOLON RIGHT_BRACE",
 /*  79 */ "fieldlist ::= field",
 /*  80 */ "fieldlist ::= fieldlist COMMA|SEMICOLON field",
 /*  81 */ "field ::= exp",
 /*  82 */ "field ::= NAME EQUAL exp",
 /*  83 */ "field ::= LEFT_BRACKET exp RIGHT_BRACKET EQUAL exp",
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
    case 21: /* COLON */
    case 22: /* DOT */
    case 23: /* COMMA */
    case 24: /* OR */
    case 25: /* AND */
    case 26: /* LESS */
    case 27: /* LESS_EQUAL */
    case 28: /* GREATER */
    case 29: /* GREATER_EQUAL */
    case 30: /* EQUAL_EQUAL */
    case 31: /* NOT_EQUAL */
    case 32: /* DOT_DOT */
    case 33: /* PLUS */
    case 34: /* MINUS */
    case 35: /* MULTIPLY */
    case 36: /* DIVIDE */
    case 37: /* MODULO */
    case 38: /* NOT */
    case 39: /* HASH */
    case 40: /* RAISE */
    case 41: /* NIL */
    case 42: /* TRUE */
    case 43: /* FALSE */
    case 44: /* NUMBER */
    case 45: /* STRING */
    case 46: /* DOT_DOT_DOT */
    case 47: /* BITWISE_OR */
    case 48: /* BITWISE_AND */
    case 49: /* LEFT_BRACKET */
    case 50: /* RIGHT_BRACKET */
    case 51: /* RIGHT_PARANTHESIS */
    case 52: /* LEFT_BRACE */
    case 53: /* RIGHT_BRACE */
{
#line 3 "LuaGrammar.y"
 delete (yypminor->yy0); 
#line 757 "LuaGrammar.cpp"
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
  { 55, 1 },
  { 57, 1 },
  { 57, 0 },
  { 56, 2 },
  { 56, 4 },
  { 61, 3 },
  { 58, 0 },
  { 58, 4 },
  { 59, 0 },
  { 59, 3 },
  { 64, 3 },
  { 64, 5 },
  { 64, 4 },
  { 64, 2 },
  { 64, 3 },
  { 64, 3 },
  { 64, 3 },
  { 64, 1 },
  { 65, 4 },
  { 65, 4 },
  { 66, 1 },
  { 66, 3 },
  { 74, 1 },
  { 74, 3 },
  { 75, 3 },
  { 60, 1 },
  { 60, 1 },
  { 60, 2 },
  { 63, 2 },
  { 63, 4 },
  { 63, 4 },
  { 67, 1 },
  { 67, 3 },
  { 76, 1 },
  { 76, 3 },
  { 73, 1 },
  { 73, 3 },
  { 70, 1 },
  { 70, 3 },
  { 72, 3 },
  { 72, 5 },
  { 62, 1 },
  { 62, 1 },
  { 62, 1 },
  { 62, 1 },
  { 62, 2 },
  { 62, 3 },
  { 62, 3 },
  { 62, 3 },
  { 62, 3 },
  { 62, 3 },
  { 62, 3 },
  { 62, 3 },
  { 62, 3 },
  { 62, 3 },
  { 69, 1 },
  { 69, 3 },
  { 80, 1 },
  { 80, 4 },
  { 80, 3 },
  { 78, 1 },
  { 78, 1 },
  { 78, 3 },
  { 71, 2 },
  { 71, 4 },
  { 81, 2 },
  { 81, 3 },
  { 81, 1 },
  { 81, 1 },
  { 77, 2 },
  { 68, 3 },
  { 82, 3 },
  { 83, 0 },
  { 83, 1 },
  { 83, 1 },
  { 83, 3 },
  { 79, 2 },
  { 79, 3 },
  { 79, 4 },
  { 84, 1 },
  { 84, 3 },
  { 85, 1 },
  { 85, 3 },
  { 85, 5 },
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
#line 1136 "LuaGrammar.cpp"
        break;
      case 1: /* semi ::= SEMICOLON */
#line 88 "LuaGrammar.y"
{
  yy_destructor(yypParser,3,&yymsp[0].minor);
}
#line 1143 "LuaGrammar.cpp"
        break;
      case 3: /* block ::= scope statlist */
#line 91 "LuaGrammar.y"
{ yygotominor.yy0 = merge(yymsp[-1].minor.yy0, yymsp[0].minor.yy0); }
#line 1148 "LuaGrammar.cpp"
        break;
      case 4: /* block ::= scope statlist laststat semi */
      case 7: /* scope ::= scope statlist binding semi */ yytestcase(yyruleno==7);
#line 92 "LuaGrammar.y"
{ yygotominor.yy0 = merge(merge(yymsp[-3].minor.yy0, yymsp[-2].minor.yy0), yymsp[-1].minor.yy0); }
#line 1154 "LuaGrammar.cpp"
        break;
      case 5: /* ublock ::= block UNTIL exp */
#line 93 "LuaGrammar.y"
{ yygotominor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[0].minor.yy0);   yy_destructor(yypParser,4,&yymsp[-1].minor);
}
#line 1160 "LuaGrammar.cpp"
        break;
      case 6: /* scope ::= */
      case 8: /* statlist ::= */ yytestcase(yyruleno==8);
      case 41: /* exp ::= NIL|TRUE|FALSE|NUMBER|STRING|DOT_DOT_DOT */ yytestcase(yyruleno==41);
      case 44: /* exp ::= tableconstructor */ yytestcase(yyruleno==44);
      case 67: /* args ::= tableconstructor */ yytestcase(yyruleno==67);
#line 95 "LuaGrammar.y"
{ yygotominor.yy0 = null(); }
#line 1169 "LuaGrammar.cpp"
        break;
      case 9: /* statlist ::= statlist stat semi */
#line 99 "LuaGrammar.y"
{ yygotominor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[-1].minor.yy0); }
#line 1174 "LuaGrammar.cpp"
        break;
      case 10: /* stat ::= DO block END */
      case 12: /* stat ::= repetition DO block END */ yytestcase(yyruleno==12);
#line 101 "LuaGrammar.y"
{ yygotominor.yy0 = copy(yymsp[-1].minor.yy0);   yy_destructor(yypParser,5,&yymsp[-2].minor);
  yy_destructor(yypParser,6,&yymsp[0].minor);
}
#line 1182 "LuaGrammar.cpp"
        break;
      case 11: /* stat ::= WHILE exp DO block END */
#line 102 "LuaGrammar.y"
{ yygotominor.yy0 = copy(yymsp[-1].minor.yy0);   yy_destructor(yypParser,7,&yymsp[-4].minor);
  yy_destructor(yypParser,5,&yymsp[-2].minor);
  yy_destructor(yypParser,6,&yymsp[0].minor);
}
#line 1190 "LuaGrammar.cpp"
        break;
      case 13: /* stat ::= REPEAT ublock */
#line 104 "LuaGrammar.y"
{ yygotominor.yy0 = copy(yymsp[0].minor.yy0);   yy_destructor(yypParser,8,&yymsp[-1].minor);
}
#line 1196 "LuaGrammar.cpp"
        break;
      case 14: /* stat ::= IF conds END */
#line 105 "LuaGrammar.y"
{ yygotominor.yy0 = copy(yymsp[-1].minor.yy0);   yy_destructor(yypParser,9,&yymsp[-2].minor);
  yy_destructor(yypParser,6,&yymsp[0].minor);
}
#line 1203 "LuaGrammar.cpp"
        break;
      case 15: /* stat ::= FUNCTION funcname funcbody */
#line 106 "LuaGrammar.y"
{ yygotominor.yy0 = new LuaGrammarToken(new IScriptOutline::FunctionNode(yymsp[-1].minor.yy0->line, yymsp[-1].minor.yy0->text, false, yymsp[0].minor.yy0->node), yymsp[-1].minor.yy0->line);   yy_destructor(yypParser,10,&yymsp[-2].minor);
}
#line 1209 "LuaGrammar.cpp"
        break;
      case 16: /* stat ::= setlist EQUAL explist1 */
#line 107 "LuaGrammar.y"
{ yygotominor.yy0 = null();   yy_destructor(yypParser,11,&yymsp[-1].minor);
}
#line 1215 "LuaGrammar.cpp"
        break;
      case 17: /* stat ::= functioncall */
      case 20: /* conds ::= condlist */ yytestcase(yyruleno==20);
      case 22: /* condlist ::= cond */ yytestcase(yyruleno==22);
      case 31: /* funcname ::= dottedname */ yytestcase(yyruleno==31);
      case 33: /* dottedname ::= NAME */ yytestcase(yyruleno==33);
      case 35: /* namelist ::= NAME */ yytestcase(yyruleno==35);
      case 37: /* explist1 ::= exp */ yytestcase(yyruleno==37);
      case 42: /* exp ::= function */ yytestcase(yyruleno==42);
      case 43: /* exp ::= prefixexp */ yytestcase(yyruleno==43);
      case 45: /* exp ::= NOT|HASH|MINUS exp */ yytestcase(yyruleno==45);
      case 57: /* var ::= NAME */ yytestcase(yyruleno==57);
      case 60: /* prefixexp ::= var */ yytestcase(yyruleno==60);
      case 61: /* prefixexp ::= functioncall */ yytestcase(yyruleno==61);
#line 108 "LuaGrammar.y"
{ yygotominor.yy0 = copy(yymsp[0].minor.yy0); }
#line 1232 "LuaGrammar.cpp"
        break;
      case 18: /* repetition ::= FOR NAME EQUAL explist23 */
#line 110 "LuaGrammar.y"
{
  yy_destructor(yypParser,12,&yymsp[-3].minor);
  yy_destructor(yypParser,13,&yymsp[-2].minor);
  yy_destructor(yypParser,11,&yymsp[-1].minor);
}
#line 1241 "LuaGrammar.cpp"
        break;
      case 19: /* repetition ::= FOR namelist IN explist1 */
#line 111 "LuaGrammar.y"
{
  yy_destructor(yypParser,12,&yymsp[-3].minor);
  yy_destructor(yypParser,14,&yymsp[-1].minor);
}
#line 1249 "LuaGrammar.cpp"
        break;
      case 21: /* conds ::= condlist ELSE block */
#line 114 "LuaGrammar.y"
{ yygotominor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[0].minor.yy0);   yy_destructor(yypParser,15,&yymsp[-1].minor);
}
#line 1255 "LuaGrammar.cpp"
        break;
      case 23: /* condlist ::= condlist ELSEIF cond */
#line 116 "LuaGrammar.y"
{ yygotominor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[0].minor.yy0);   yy_destructor(yypParser,16,&yymsp[-1].minor);
}
#line 1261 "LuaGrammar.cpp"
        break;
      case 24: /* cond ::= exp THEN block */
#line 117 "LuaGrammar.y"
{ yygotominor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[0].minor.yy0);   yy_destructor(yypParser,17,&yymsp[-1].minor);
}
#line 1267 "LuaGrammar.cpp"
        break;
      case 25: /* laststat ::= BREAK */
#line 119 "LuaGrammar.y"
{ yygotominor.yy0 = null();   yy_destructor(yypParser,18,&yymsp[0].minor);
}
#line 1273 "LuaGrammar.cpp"
        break;
      case 26: /* laststat ::= RETURN */
#line 120 "LuaGrammar.y"
{ yygotominor.yy0 = null();   yy_destructor(yypParser,19,&yymsp[0].minor);
}
#line 1279 "LuaGrammar.cpp"
        break;
      case 27: /* laststat ::= RETURN explist1 */
#line 121 "LuaGrammar.y"
{ yygotominor.yy0 = copy(yymsp[0].minor.yy0);   yy_destructor(yypParser,19,&yymsp[-1].minor);
}
#line 1285 "LuaGrammar.cpp"
        break;
      case 28: /* binding ::= LOCAL namelist */
#line 123 "LuaGrammar.y"
{ yygotominor.yy0 = null();   yy_destructor(yypParser,20,&yymsp[-1].minor);
}
#line 1291 "LuaGrammar.cpp"
        break;
      case 29: /* binding ::= LOCAL namelist EQUAL explist1 */
#line 124 "LuaGrammar.y"
{ yygotominor.yy0 = copy(yymsp[0].minor.yy0);   yy_destructor(yypParser,20,&yymsp[-3].minor);
  yy_destructor(yypParser,11,&yymsp[-1].minor);
}
#line 1298 "LuaGrammar.cpp"
        break;
      case 30: /* binding ::= LOCAL FUNCTION NAME funcbody */
#line 125 "LuaGrammar.y"
{ yygotominor.yy0 = new LuaGrammarToken(new IScriptOutline::FunctionNode(yymsp[-1].minor.yy0->line, yymsp[-1].minor.yy0->text, true, yymsp[0].minor.yy0->node), yymsp[-1].minor.yy0->line);   yy_destructor(yypParser,20,&yymsp[-3].minor);
  yy_destructor(yypParser,10,&yymsp[-2].minor);
}
#line 1305 "LuaGrammar.cpp"
        break;
      case 32: /* funcname ::= dottedname COLON NAME */
#line 128 "LuaGrammar.y"
{ yygotominor.yy0 = new LuaGrammarToken(yymsp[-2].minor.yy0->text + L":" + yymsp[0].minor.yy0->text, yymsp[0].minor.yy0->line);   yy_destructor(yypParser,21,&yymsp[-1].minor);
}
#line 1311 "LuaGrammar.cpp"
        break;
      case 34: /* dottedname ::= dottedname DOT NAME */
#line 131 "LuaGrammar.y"
{ yygotominor.yy0 = new LuaGrammarToken(yymsp[-2].minor.yy0->text + L"." + yymsp[0].minor.yy0->text, yymsp[0].minor.yy0->line);   yy_destructor(yypParser,22,&yymsp[-1].minor);
}
#line 1317 "LuaGrammar.cpp"
        break;
      case 36: /* namelist ::= namelist COMMA NAME */
#line 134 "LuaGrammar.y"
{ yygotominor.yy0 = new LuaGrammarToken(yymsp[-2].minor.yy0->text + L"," + yymsp[0].minor.yy0->text, yymsp[0].minor.yy0->line);   yy_destructor(yypParser,23,&yymsp[-1].minor);
}
#line 1323 "LuaGrammar.cpp"
        break;
      case 38: /* explist1 ::= explist1 COMMA exp */
      case 39: /* explist23 ::= exp COMMA exp */ yytestcase(yyruleno==39);
#line 137 "LuaGrammar.y"
{ yygotominor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[0].minor.yy0);   yy_destructor(yypParser,23,&yymsp[-1].minor);
}
#line 1330 "LuaGrammar.cpp"
        break;
      case 40: /* explist23 ::= exp COMMA exp COMMA exp */
#line 139 "LuaGrammar.y"
{ yygotominor.yy0 = merge(merge(yymsp[-4].minor.yy0, yymsp[-2].minor.yy0), yymsp[0].minor.yy0);   yy_destructor(yypParser,23,&yymsp[-3].minor);
  yy_destructor(yypParser,23,&yymsp[-1].minor);
}
#line 1337 "LuaGrammar.cpp"
        break;
      case 46: /* exp ::= exp OR exp */
#line 155 "LuaGrammar.y"
{ yygotominor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[0].minor.yy0);   yy_destructor(yypParser,24,&yymsp[-1].minor);
}
#line 1343 "LuaGrammar.cpp"
        break;
      case 47: /* exp ::= exp AND exp */
#line 156 "LuaGrammar.y"
{ yygotominor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[0].minor.yy0);   yy_destructor(yypParser,25,&yymsp[-1].minor);
}
#line 1349 "LuaGrammar.cpp"
        break;
      case 48: /* exp ::= exp LESS|LESS_EQUAL|GREATER|GREATER_EQUAL|EQUAL_EQUAL|NOT_EQUAL exp */
      case 52: /* exp ::= exp PLUS|MINUS exp */ yytestcase(yyruleno==52);
      case 53: /* exp ::= exp MULTIPLY|DIVIDE|MODULO exp */ yytestcase(yyruleno==53);
#line 157 "LuaGrammar.y"
{ yygotominor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[0].minor.yy0); }
#line 1356 "LuaGrammar.cpp"
        break;
      case 49: /* exp ::= exp DOT_DOT exp */
#line 158 "LuaGrammar.y"
{ yygotominor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[0].minor.yy0);   yy_destructor(yypParser,32,&yymsp[-1].minor);
}
#line 1362 "LuaGrammar.cpp"
        break;
      case 50: /* exp ::= exp BITWISE_OR exp */
#line 159 "LuaGrammar.y"
{ yygotominor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[0].minor.yy0);   yy_destructor(yypParser,47,&yymsp[-1].minor);
}
#line 1368 "LuaGrammar.cpp"
        break;
      case 51: /* exp ::= exp BITWISE_AND exp */
#line 160 "LuaGrammar.y"
{ yygotominor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[0].minor.yy0);   yy_destructor(yypParser,48,&yymsp[-1].minor);
}
#line 1374 "LuaGrammar.cpp"
        break;
      case 54: /* exp ::= exp RAISE exp */
#line 163 "LuaGrammar.y"
{ yygotominor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[0].minor.yy0);   yy_destructor(yypParser,40,&yymsp[-1].minor);
}
#line 1380 "LuaGrammar.cpp"
        break;
      case 56: /* setlist ::= setlist COMMA var */
#line 166 "LuaGrammar.y"
{
  yy_destructor(yypParser,23,&yymsp[-1].minor);
}
#line 1387 "LuaGrammar.cpp"
        break;
      case 58: /* var ::= prefixexp LEFT_BRACKET exp RIGHT_BRACKET */
#line 169 "LuaGrammar.y"
{ yygotominor.yy0 = merge(yymsp[-3].minor.yy0, yymsp[-1].minor.yy0);   yy_destructor(yypParser,49,&yymsp[-2].minor);
  yy_destructor(yypParser,50,&yymsp[0].minor);
}
#line 1394 "LuaGrammar.cpp"
        break;
      case 59: /* var ::= prefixexp DOT NAME */
#line 170 "LuaGrammar.y"
{ yygotominor.yy0 = new LuaGrammarToken(as_string(yymsp[-2].minor.yy0) + L"." + yymsp[0].minor.yy0->text, yymsp[0].minor.yy0->line);   yy_destructor(yypParser,22,&yymsp[-1].minor);
}
#line 1400 "LuaGrammar.cpp"
        break;
      case 62: /* prefixexp ::= OPEN exp RIGHT_PARANTHESIS */
#line 174 "LuaGrammar.y"
{ yygotominor.yy0 = copy(yymsp[-1].minor.yy0);   yy_destructor(yypParser,1,&yymsp[-2].minor);
  yy_destructor(yypParser,51,&yymsp[0].minor);
}
#line 1407 "LuaGrammar.cpp"
        break;
      case 63: /* functioncall ::= prefixexp args */
#line 176 "LuaGrammar.y"
{ yygotominor.yy0 = merge(new LuaGrammarToken(new IScriptOutline::FunctionReferenceNode(yymsp[-1].minor.yy0->line, as_string(yymsp[-1].minor.yy0)), yymsp[-1].minor.yy0->line), yymsp[0].minor.yy0); }
#line 1412 "LuaGrammar.cpp"
        break;
      case 64: /* functioncall ::= prefixexp COLON NAME args */
#line 177 "LuaGrammar.y"
{ yygotominor.yy0 = merge(new LuaGrammarToken(new IScriptOutline::FunctionReferenceNode(yymsp[-1].minor.yy0->line, as_string(yymsp[-3].minor.yy0) + L":" + yymsp[-1].minor.yy0->text), yymsp[-1].minor.yy0->line), yymsp[0].minor.yy0);   yy_destructor(yypParser,21,&yymsp[-2].minor);
}
#line 1418 "LuaGrammar.cpp"
        break;
      case 65: /* args ::= LEFT_PARANTHESIS RIGHT_PARANTHESIS */
#line 179 "LuaGrammar.y"
{ yygotominor.yy0 = null();   yy_destructor(yypParser,2,&yymsp[-1].minor);
  yy_destructor(yypParser,51,&yymsp[0].minor);
}
#line 1425 "LuaGrammar.cpp"
        break;
      case 66: /* args ::= LEFT_PARANTHESIS explist1 RIGHT_PARANTHESIS */
#line 180 "LuaGrammar.y"
{ yygotominor.yy0 = copy(yymsp[-1].minor.yy0);   yy_destructor(yypParser,2,&yymsp[-2].minor);
  yy_destructor(yypParser,51,&yymsp[0].minor);
}
#line 1432 "LuaGrammar.cpp"
        break;
      case 68: /* args ::= STRING */
#line 182 "LuaGrammar.y"
{ yygotominor.yy0 = null();   yy_destructor(yypParser,45,&yymsp[0].minor);
}
#line 1438 "LuaGrammar.cpp"
        break;
      case 69: /* function ::= FUNCTION funcbody */
#line 184 "LuaGrammar.y"
{ yygotominor.yy0 = copy(yymsp[0].minor.yy0);   yy_destructor(yypParser,10,&yymsp[-1].minor);
}
#line 1444 "LuaGrammar.cpp"
        break;
      case 70: /* funcbody ::= params block END */
#line 186 "LuaGrammar.y"
{ yygotominor.yy0 = copy(yymsp[-1].minor.yy0);   yy_destructor(yypParser,6,&yymsp[0].minor);
}
#line 1450 "LuaGrammar.cpp"
        break;
      case 71: /* params ::= LEFT_PARANTHESIS parlist RIGHT_PARANTHESIS */
#line 188 "LuaGrammar.y"
{
  yy_destructor(yypParser,2,&yymsp[-2].minor);
  yy_destructor(yypParser,51,&yymsp[0].minor);
}
#line 1458 "LuaGrammar.cpp"
        break;
      case 74: /* parlist ::= DOT_DOT_DOT */
#line 192 "LuaGrammar.y"
{
  yy_destructor(yypParser,46,&yymsp[0].minor);
}
#line 1465 "LuaGrammar.cpp"
        break;
      case 75: /* parlist ::= namelist COMMA DOT_DOT_DOT */
#line 193 "LuaGrammar.y"
{
  yy_destructor(yypParser,23,&yymsp[-1].minor);
  yy_destructor(yypParser,46,&yymsp[0].minor);
}
#line 1473 "LuaGrammar.cpp"
        break;
      case 76: /* tableconstructor ::= LEFT_BRACE RIGHT_BRACE */
#line 195 "LuaGrammar.y"
{
  yy_destructor(yypParser,52,&yymsp[-1].minor);
  yy_destructor(yypParser,53,&yymsp[0].minor);
}
#line 1481 "LuaGrammar.cpp"
        break;
      case 77: /* tableconstructor ::= LEFT_BRACE fieldlist RIGHT_BRACE */
#line 196 "LuaGrammar.y"
{
  yy_destructor(yypParser,52,&yymsp[-2].minor);
  yy_destructor(yypParser,53,&yymsp[0].minor);
}
#line 1489 "LuaGrammar.cpp"
        break;
      case 78: /* tableconstructor ::= LEFT_BRACE fieldlist COMMA|SEMICOLON RIGHT_BRACE */
#line 197 "LuaGrammar.y"
{
  yy_destructor(yypParser,52,&yymsp[-3].minor);
  yy_destructor(yypParser,53,&yymsp[0].minor);
}
#line 1497 "LuaGrammar.cpp"
        break;
      case 82: /* field ::= NAME EQUAL exp */
#line 203 "LuaGrammar.y"
{
  yy_destructor(yypParser,13,&yymsp[-2].minor);
  yy_destructor(yypParser,11,&yymsp[-1].minor);
}
#line 1505 "LuaGrammar.cpp"
        break;
      case 83: /* field ::= LEFT_BRACKET exp RIGHT_BRACKET EQUAL exp */
#line 204 "LuaGrammar.y"
{
  yy_destructor(yypParser,49,&yymsp[-4].minor);
  yy_destructor(yypParser,50,&yymsp[-2].minor);
  yy_destructor(yypParser,11,&yymsp[-1].minor);
}
#line 1514 "LuaGrammar.cpp"
        break;
      default:
      /* (2) semi ::= */ yytestcase(yyruleno==2);
      /* (55) setlist ::= var */ yytestcase(yyruleno==55);
      /* (72) parlist ::= */ yytestcase(yyruleno==72);
      /* (73) parlist ::= namelist */ yytestcase(yyruleno==73);
      /* (79) fieldlist ::= field */ yytestcase(yyruleno==79);
      /* (80) fieldlist ::= fieldlist COMMA|SEMICOLON field */ yytestcase(yyruleno==80);
      /* (81) field ::= exp */ yytestcase(yyruleno==81);
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

#line 1588 "LuaGrammar.cpp"
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