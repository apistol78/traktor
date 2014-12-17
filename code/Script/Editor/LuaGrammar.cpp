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
#define YYNOCODE 85
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
#define YYNSTATE 155
#define YYNRULE 82
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
#define YY_ACTTAB_COUNT (605)
static const YYACTIONTYPE yy_action[] = {
 /*     0 */    44,   13,   28,   25,   24,   24,   24,   24,   24,   24,
 /*    10 */    23,   22,   22,   21,   21,   21,  155,   15,   20,   28,
 /*    20 */    25,   24,   24,   24,   24,   24,   24,   23,   22,   22,
 /*    30 */    21,   21,   21,   41,   11,   20,   21,   21,   21,    4,
 /*    40 */   150,   20,    5,  125,   28,   25,   24,   24,   24,   24,
 /*    50 */    24,   24,   23,   22,   22,   21,   21,   21,  143,    4,
 /*    60 */    20,   92,  106,   51,   51,   32,  120,   76,   20,  142,
 /*    70 */    28,   25,   24,   24,   24,   24,   24,   24,   23,   22,
 /*    80 */    22,   21,   21,   21,    8,  122,   20,  133,   90,   89,
 /*    90 */     2,  149,  140,   85,   97,   28,   25,   24,   24,   24,
 /*   100 */    24,   24,   24,   23,   22,   22,   21,   21,   21,  238,
 /*   110 */   107,   20,   51,   40,   82,   81,  105,  139,   51,  129,
 /*   120 */    28,   25,   24,   24,   24,   24,   24,   24,   23,   22,
 /*   130 */    22,   21,   21,   21,   18,   16,   20,   12,   28,   25,
 /*   140 */    24,   24,   24,   24,   24,   24,   23,   22,   22,   21,
 /*   150 */    21,   21,  136,   42,   20,   28,   25,   24,   24,   24,
 /*   160 */    24,   24,   24,   23,   22,   22,   21,   21,   21,   19,
 /*   170 */   128,   20,   23,   22,   22,   21,   21,   21,   39,  141,
 /*   180 */    20,   98,   58,   19,  101,   50,   51,  114,   49,   48,
 /*   190 */    94,  143,   39,   43,   77,   98,   70,  147,   33,  146,
 /*   200 */   144,   43,   26,   32,   34,   73,   26,   26,   37,  148,
 /*   210 */   148,  148,  148,  148,  148,   17,   26,   78,    2,  132,
 /*   220 */    26,   26,   19,  148,  148,  148,  148,  148,  148,   17,
 /*   230 */   121,   39,    2,  134,  145,   25,   24,   24,   24,   24,
 /*   240 */    24,   24,   23,   22,   22,   21,   21,   21,  100,  214,
 /*   250 */    20,   93,  138,   51,  118,   26,   51,  139,   99,   26,
 /*   260 */    26,   14,  148,  148,  148,  148,  148,  148,  214,  214,
 /*   270 */   124,    2,   24,   24,   24,   24,   24,   24,   23,   22,
 /*   280 */    22,   21,   21,   21,   19,  137,   20,  119,   10,    5,
 /*   290 */   135,  108,  214,   39,  214,   19,  145,  214,   19,   45,
 /*   300 */    30,   29,   31,    1,   38,   43,   47,  145,   95,   96,
 /*   310 */   145,  209,  109,    7,   46,  113,  112,   26,  116,    9,
 /*   320 */    51,   26,   26,  209,  148,  148,  148,  148,  148,  148,
 /*   330 */   138,   54,  122,    2,   27,   91,   65,    2,    6,   15,
 /*   340 */   143,   36,    3,   79,  115,  143,  147,   33,  146,  144,
 /*   350 */    85,  147,   33,  146,  144,  123,   54,  126,   72,  130,
 /*   360 */   153,  123,  154,  127,  152,  143,  151,   62,  208,  117,
 /*   370 */    75,  147,   33,  146,  144,   88,  143,   87,   53,   35,
 /*   380 */   208,   74,  147,   33,  146,  144,  111,  143,  110,   62,
 /*   390 */   239,  239,  239,  147,   33,  146,  144,   86,  143,  239,
 /*   400 */   239,   62,  239,  239,  147,   33,  146,  144,  239,   84,
 /*   410 */   143,  239,   62,  239,  239,  239,  147,   33,  146,  144,
 /*   420 */    83,  143,  239,  239,  239,  239,   62,  147,   33,  146,
 /*   430 */   144,  239,  239,  239,   80,  143,   65,  239,  239,  239,
 /*   440 */   239,  147,   33,  146,  144,  143,   66,  239,  239,  239,
 /*   450 */   239,  147,   33,  146,  144,  143,   55,  239,  239,  131,
 /*   460 */   239,  147,   33,  146,  144,  143,  104,  239,  239,  239,
 /*   470 */    67,  147,   33,  146,  144,  143,  239,  239,  239,  143,
 /*   480 */    69,  147,   33,  146,  144,  147,   33,  146,  144,  143,
 /*   490 */    68,  239,  239,  239,  239,  147,   33,  146,  144,  143,
 /*   500 */   239,  239,   71,  239,  239,  147,   33,  146,  144,  239,
 /*   510 */   239,  143,  103,  239,  239,  239,  102,  147,   33,  146,
 /*   520 */   144,  143,  239,  239,  239,  143,   57,  147,   33,  146,
 /*   530 */   144,  147,   33,  146,  144,  143,   64,  239,  239,  239,
 /*   540 */    56,  147,   33,  146,  144,  143,  239,  239,  239,  143,
 /*   550 */    63,  147,   33,  146,  144,  147,   33,  146,  144,  143,
 /*   560 */    61,  239,  239,  239,  239,  147,   33,  146,  144,  143,
 /*   570 */   239,  239,   60,  239,  239,  147,   33,  146,  144,  239,
 /*   580 */   239,  143,   52,  239,  239,  239,   59,  147,   33,  146,
 /*   590 */   144,  143,  239,  239,  239,  143,  239,  147,   33,  146,
 /*   600 */   144,  147,   33,  146,  144,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */     5,   23,   24,   25,   26,   27,   28,   29,   30,   31,
 /*    10 */    32,   33,   34,   35,   36,   37,    0,   23,   40,   24,
 /*    20 */    25,   26,   27,   28,   29,   30,   31,   32,   33,   34,
 /*    30 */    35,   36,   37,   15,   16,   40,   35,   36,   37,    3,
 /*    40 */     6,   40,    2,   49,   24,   25,   26,   27,   28,   29,
 /*    50 */    30,   31,   32,   33,   34,   35,   36,   37,   69,   23,
 /*    60 */    40,   54,   54,   56,   56,   76,   59,   78,   40,   49,
 /*    70 */    24,   25,   26,   27,   28,   29,   30,   31,   32,   33,
 /*    80 */    34,   35,   36,   37,   14,   45,   40,   51,   21,   22,
 /*    90 */    50,    6,    6,   23,   48,   24,   25,   26,   27,   28,
 /*   100 */    29,   30,   31,   32,   33,   34,   35,   36,   37,   53,
 /*   110 */    54,   40,   56,   17,   10,   23,   54,   13,   56,   48,
 /*   120 */    24,   25,   26,   27,   28,   29,   30,   31,   32,   33,
 /*   130 */    34,   35,   36,   37,   11,   11,   40,   23,   24,   25,
 /*   140 */    26,   27,   28,   29,   30,   31,   32,   33,   34,   35,
 /*   150 */    36,   37,   49,    5,   40,   24,   25,   26,   27,   28,
 /*   160 */    29,   30,   31,   32,   33,   34,   35,   36,   37,    1,
 /*   170 */    13,   40,   32,   33,   34,   35,   36,   37,   10,   66,
 /*   180 */    40,   13,   60,    1,   54,   58,   56,   66,   61,   62,
 /*   190 */    63,   69,   10,   80,   67,   13,   69,   75,   76,   77,
 /*   200 */    78,   80,   34,   76,   13,   78,   38,   39,   65,   41,
 /*   210 */    42,   43,   44,   45,   46,   47,   34,   74,   50,   51,
 /*   220 */    38,   39,    1,   41,   42,   43,   44,   45,   46,   47,
 /*   230 */     6,   10,   50,   51,   13,   25,   26,   27,   28,   29,
 /*   240 */    30,   31,   32,   33,   34,   35,   36,   37,   71,    2,
 /*   250 */    40,   54,   13,   56,   54,   34,   56,   13,   81,   38,
 /*   260 */    39,    4,   41,   42,   43,   44,   45,   46,   21,   22,
 /*   270 */    49,   50,   26,   27,   28,   29,   30,   31,   32,   33,
 /*   280 */    34,   35,   36,   37,    1,   46,   40,    6,   11,    2,
 /*   290 */    46,   66,   45,   10,   47,    1,   13,   50,    1,    5,
 /*   300 */    23,    7,    8,    9,   10,   80,   12,   13,   21,   22,
 /*   310 */    13,   11,   18,   19,   20,   13,   13,   34,   54,   11,
 /*   320 */    56,   38,   39,   23,   41,   42,   43,   44,   45,   46,
 /*   330 */    13,   60,   45,   50,   47,   64,   60,   50,   11,   23,
 /*   340 */    69,   13,   57,   72,   73,   69,   75,   76,   77,   78,
 /*   350 */    23,   75,   76,   77,   78,   77,   60,   79,   82,   83,
 /*   360 */    55,   77,    3,   79,   55,   69,   55,   60,   11,   73,
 /*   370 */    71,   75,   76,   77,   78,   68,   69,   13,   60,    2,
 /*   380 */    23,   71,   75,   76,   77,   78,   13,   69,   70,   60,
 /*   390 */    84,   84,   84,   75,   76,   77,   78,   68,   69,   84,
 /*   400 */    84,   60,   84,   84,   75,   76,   77,   78,   84,   68,
 /*   410 */    69,   84,   60,   84,   84,   84,   75,   76,   77,   78,
 /*   420 */    68,   69,   84,   84,   84,   84,   60,   75,   76,   77,
 /*   430 */    78,   84,   84,   84,   68,   69,   60,   84,   84,   84,
 /*   440 */    84,   75,   76,   77,   78,   69,   60,   84,   84,   84,
 /*   450 */    84,   75,   76,   77,   78,   69,   60,   84,   84,   83,
 /*   460 */    84,   75,   76,   77,   78,   69,   60,   84,   84,   84,
 /*   470 */    60,   75,   76,   77,   78,   69,   84,   84,   84,   69,
 /*   480 */    60,   75,   76,   77,   78,   75,   76,   77,   78,   69,
 /*   490 */    60,   84,   84,   84,   84,   75,   76,   77,   78,   69,
 /*   500 */    84,   84,   60,   84,   84,   75,   76,   77,   78,   84,
 /*   510 */    84,   69,   60,   84,   84,   84,   60,   75,   76,   77,
 /*   520 */    78,   69,   84,   84,   84,   69,   60,   75,   76,   77,
 /*   530 */    78,   75,   76,   77,   78,   69,   60,   84,   84,   84,
 /*   540 */    60,   75,   76,   77,   78,   69,   84,   84,   84,   69,
 /*   550 */    60,   75,   76,   77,   78,   75,   76,   77,   78,   69,
 /*   560 */    60,   84,   84,   84,   84,   75,   76,   77,   78,   69,
 /*   570 */    84,   84,   60,   84,   84,   75,   76,   77,   78,   84,
 /*   580 */    84,   69,   60,   84,   84,   84,   60,   75,   76,   77,
 /*   590 */    78,   69,   84,   84,   84,   69,   84,   75,   76,   77,
 /*   600 */    78,   75,   76,   77,   78,
};
#define YY_SHIFT_USE_DFLT (-23)
#define YY_SHIFT_COUNT (107)
#define YY_SHIFT_MIN   (-22)
#define YY_SHIFT_MAX   (377)
static const short yy_shift_ofst[] = {
 /*     0 */   -23,  283,  182,  294,  168,  221,  283,  283,  283,  283,
 /*    10 */   283,  283,  283,  283,  283,  283,  283,  283,  283,  283,
 /*    20 */   283,  283,  283,  283,  283,  283,  283,  283,  283,  283,
 /*    30 */   297,  -23,  287,  287,   40,  244,  377,  377,  373,  377,
 /*    40 */   -23,  -23,  -23,  -23,  -23,  -23,  104,  364,  359,  359,
 /*    50 */   359,  -23,  114,  -22,   96,   71,   46,   20,   -5,  131,
 /*    60 */   131,  131,  131,  131,  131,  131,  210,  246,  140,  140,
 /*    70 */   247,    1,   36,  357,  327,   70,  300,  277,   67,   18,
 /*    80 */    -6,  239,  328,  316,  316,  317,  316,  308,  316,  303,
 /*    90 */   302,  281,  257,  224,  148,  191,  157,  124,  123,  103,
 /*   100 */    92,   86,   28,   28,   28,   85,   34,   16,
};
#define YY_REDUCE_USE_DFLT (-12)
#define YY_REDUCE_COUNT (51)
#define YY_REDUCE_MIN   (-11)
#define YY_REDUCE_MAX   (526)
static const short yy_reduce_ofst[] = {
 /*     0 */    56,  271,  276,  127,  376,  366,  352,  341,  329,  318,
 /*    10 */   307,  296,  526,  522,  512,  500,  490,  480,  476,  466,
 /*    20 */   456,  452,  442,  430,  420,  410,  406,  396,  386,  122,
 /*    30 */   -11,    7,  284,  284,  278,  177,  225,  121,  143,  113,
 /*    40 */   264,  200,  197,  130,   62,    8,  310,  299,  311,  309,
 /*    50 */   305,  285,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   161,  237,  237,  158,  237,  237,  237,  181,  237,  237,
 /*    10 */   237,  237,  237,  237,  237,  237,  237,  237,  237,  237,
 /*    20 */   237,  237,  237,  237,  237,  237,  237,  237,  237,  237,
 /*    30 */   237,  161,  237,  198,  237,  225,  237,  237,  237,  237,
 /*    40 */   161,  161,  161,  161,  161,  161,  237,  237,  157,  157,
 /*    50 */   157,  163,  194,  237,  237,  237,  237,  237,  237,  195,
 /*    60 */   160,  193,  192,  236,  235,  234,  201,  202,  204,  203,
 /*    70 */   172,  205,  237,  213,  183,  237,  213,  237,  186,  175,
 /*    80 */   237,  237,  237,  184,  182,  237,  174,  190,  171,  237,
 /*    90 */   237,  237,  237,  237,  237,  237,  237,  237,  210,  237,
 /*   100 */   226,  237,  207,  206,  200,  237,  237,  237,  185,  180,
 /*   110 */   173,  188,  189,  187,  170,  177,  179,  178,  176,  169,
 /*   120 */   168,  167,  221,  220,  218,  219,  217,  216,  212,  211,
 /*   130 */   232,  233,  231,  230,  229,  227,  224,  228,  191,  190,
 /*   140 */   223,  222,  215,  214,  213,  210,  199,  197,  196,  166,
 /*   150 */   165,  164,  162,  159,  156,
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
  "NUMBER",        "STRING",        "DOT_DOT_DOT",   "LEFT_BRACKET",
  "RIGHT_BRACKET",  "RIGHT_PARANTHESIS",  "LEFT_BRACE",    "RIGHT_BRACE", 
  "error",         "chunk",         "block",         "semi",        
  "scope",         "statlist",      "laststat",      "ublock",      
  "exp",           "binding",       "stat",          "repetition",  
  "conds",         "funcname",      "funcbody",      "setlist",     
  "explist1",      "functioncall",  "explist23",     "namelist",    
  "condlist",      "cond",          "dottedname",    "function",    
  "prefixexp",     "tableconstructor",  "var",           "args",        
  "params",        "parlist",       "fieldlist",     "field",       
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
 /*  50 */ "exp ::= exp PLUS|MINUS exp",
 /*  51 */ "exp ::= exp MULTIPLY|DIVIDE|MODULO exp",
 /*  52 */ "exp ::= exp RAISE exp",
 /*  53 */ "setlist ::= var",
 /*  54 */ "setlist ::= setlist COMMA var",
 /*  55 */ "var ::= NAME",
 /*  56 */ "var ::= prefixexp LEFT_BRACKET exp RIGHT_BRACKET",
 /*  57 */ "var ::= prefixexp DOT NAME",
 /*  58 */ "prefixexp ::= var",
 /*  59 */ "prefixexp ::= functioncall",
 /*  60 */ "prefixexp ::= OPEN exp RIGHT_PARANTHESIS",
 /*  61 */ "functioncall ::= prefixexp args",
 /*  62 */ "functioncall ::= prefixexp COLON NAME args",
 /*  63 */ "args ::= LEFT_PARANTHESIS RIGHT_PARANTHESIS",
 /*  64 */ "args ::= LEFT_PARANTHESIS explist1 RIGHT_PARANTHESIS",
 /*  65 */ "args ::= tableconstructor",
 /*  66 */ "args ::= STRING",
 /*  67 */ "function ::= FUNCTION funcbody",
 /*  68 */ "funcbody ::= params block END",
 /*  69 */ "params ::= LEFT_PARANTHESIS parlist RIGHT_PARANTHESIS",
 /*  70 */ "parlist ::=",
 /*  71 */ "parlist ::= namelist",
 /*  72 */ "parlist ::= DOT_DOT_DOT",
 /*  73 */ "parlist ::= namelist COMMA DOT_DOT_DOT",
 /*  74 */ "tableconstructor ::= LEFT_BRACE RIGHT_BRACE",
 /*  75 */ "tableconstructor ::= LEFT_BRACE fieldlist RIGHT_BRACE",
 /*  76 */ "tableconstructor ::= LEFT_BRACE fieldlist COMMA|SEMICOLON RIGHT_BRACE",
 /*  77 */ "fieldlist ::= field",
 /*  78 */ "fieldlist ::= fieldlist COMMA|SEMICOLON field",
 /*  79 */ "field ::= exp",
 /*  80 */ "field ::= NAME EQUAL exp",
 /*  81 */ "field ::= LEFT_BRACKET exp RIGHT_BRACKET EQUAL exp",
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
    case 47: /* LEFT_BRACKET */
    case 48: /* RIGHT_BRACKET */
    case 49: /* RIGHT_PARANTHESIS */
    case 50: /* LEFT_BRACE */
    case 51: /* RIGHT_BRACE */
{
#line 3 "LuaGrammar.y"
 delete (yypminor->yy0); 
#line 725 "LuaGrammar.cpp"
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
  { 53, 1 },
  { 55, 1 },
  { 55, 0 },
  { 54, 2 },
  { 54, 4 },
  { 59, 3 },
  { 56, 0 },
  { 56, 4 },
  { 57, 0 },
  { 57, 3 },
  { 62, 3 },
  { 62, 5 },
  { 62, 4 },
  { 62, 2 },
  { 62, 3 },
  { 62, 3 },
  { 62, 3 },
  { 62, 1 },
  { 63, 4 },
  { 63, 4 },
  { 64, 1 },
  { 64, 3 },
  { 72, 1 },
  { 72, 3 },
  { 73, 3 },
  { 58, 1 },
  { 58, 1 },
  { 58, 2 },
  { 61, 2 },
  { 61, 4 },
  { 61, 4 },
  { 65, 1 },
  { 65, 3 },
  { 74, 1 },
  { 74, 3 },
  { 71, 1 },
  { 71, 3 },
  { 68, 1 },
  { 68, 3 },
  { 70, 3 },
  { 70, 5 },
  { 60, 1 },
  { 60, 1 },
  { 60, 1 },
  { 60, 1 },
  { 60, 2 },
  { 60, 3 },
  { 60, 3 },
  { 60, 3 },
  { 60, 3 },
  { 60, 3 },
  { 60, 3 },
  { 60, 3 },
  { 67, 1 },
  { 67, 3 },
  { 78, 1 },
  { 78, 4 },
  { 78, 3 },
  { 76, 1 },
  { 76, 1 },
  { 76, 3 },
  { 69, 2 },
  { 69, 4 },
  { 79, 2 },
  { 79, 3 },
  { 79, 1 },
  { 79, 1 },
  { 75, 2 },
  { 66, 3 },
  { 80, 3 },
  { 81, 0 },
  { 81, 1 },
  { 81, 1 },
  { 81, 3 },
  { 77, 2 },
  { 77, 3 },
  { 77, 4 },
  { 82, 1 },
  { 82, 3 },
  { 83, 1 },
  { 83, 3 },
  { 83, 5 },
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
#line 1102 "LuaGrammar.cpp"
        break;
      case 1: /* semi ::= SEMICOLON */
#line 88 "LuaGrammar.y"
{
  yy_destructor(yypParser,3,&yymsp[0].minor);
}
#line 1109 "LuaGrammar.cpp"
        break;
      case 3: /* block ::= scope statlist */
#line 91 "LuaGrammar.y"
{ yygotominor.yy0 = merge(yymsp[-1].minor.yy0, yymsp[0].minor.yy0); }
#line 1114 "LuaGrammar.cpp"
        break;
      case 4: /* block ::= scope statlist laststat semi */
      case 7: /* scope ::= scope statlist binding semi */ yytestcase(yyruleno==7);
#line 92 "LuaGrammar.y"
{ yygotominor.yy0 = merge(merge(yymsp[-3].minor.yy0, yymsp[-2].minor.yy0), yymsp[-1].minor.yy0); }
#line 1120 "LuaGrammar.cpp"
        break;
      case 5: /* ublock ::= block UNTIL exp */
#line 93 "LuaGrammar.y"
{ yygotominor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[0].minor.yy0);   yy_destructor(yypParser,4,&yymsp[-1].minor);
}
#line 1126 "LuaGrammar.cpp"
        break;
      case 6: /* scope ::= */
      case 8: /* statlist ::= */ yytestcase(yyruleno==8);
      case 41: /* exp ::= NIL|TRUE|FALSE|NUMBER|STRING|DOT_DOT_DOT */ yytestcase(yyruleno==41);
      case 44: /* exp ::= tableconstructor */ yytestcase(yyruleno==44);
      case 65: /* args ::= tableconstructor */ yytestcase(yyruleno==65);
#line 95 "LuaGrammar.y"
{ yygotominor.yy0 = null(); }
#line 1135 "LuaGrammar.cpp"
        break;
      case 9: /* statlist ::= statlist stat semi */
#line 99 "LuaGrammar.y"
{ yygotominor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[-1].minor.yy0); }
#line 1140 "LuaGrammar.cpp"
        break;
      case 10: /* stat ::= DO block END */
      case 12: /* stat ::= repetition DO block END */ yytestcase(yyruleno==12);
#line 101 "LuaGrammar.y"
{ yygotominor.yy0 = copy(yymsp[-1].minor.yy0);   yy_destructor(yypParser,5,&yymsp[-2].minor);
  yy_destructor(yypParser,6,&yymsp[0].minor);
}
#line 1148 "LuaGrammar.cpp"
        break;
      case 11: /* stat ::= WHILE exp DO block END */
#line 102 "LuaGrammar.y"
{ yygotominor.yy0 = copy(yymsp[-1].minor.yy0);   yy_destructor(yypParser,7,&yymsp[-4].minor);
  yy_destructor(yypParser,5,&yymsp[-2].minor);
  yy_destructor(yypParser,6,&yymsp[0].minor);
}
#line 1156 "LuaGrammar.cpp"
        break;
      case 13: /* stat ::= REPEAT ublock */
#line 104 "LuaGrammar.y"
{ yygotominor.yy0 = copy(yymsp[0].minor.yy0);   yy_destructor(yypParser,8,&yymsp[-1].minor);
}
#line 1162 "LuaGrammar.cpp"
        break;
      case 14: /* stat ::= IF conds END */
#line 105 "LuaGrammar.y"
{ yygotominor.yy0 = copy(yymsp[-1].minor.yy0);   yy_destructor(yypParser,9,&yymsp[-2].minor);
  yy_destructor(yypParser,6,&yymsp[0].minor);
}
#line 1169 "LuaGrammar.cpp"
        break;
      case 15: /* stat ::= FUNCTION funcname funcbody */
#line 106 "LuaGrammar.y"
{ yygotominor.yy0 = new LuaGrammarToken(new IScriptOutline::FunctionNode(yymsp[-1].minor.yy0->line, yymsp[-1].minor.yy0->text, false, yymsp[0].minor.yy0->node), yymsp[-1].minor.yy0->line);   yy_destructor(yypParser,10,&yymsp[-2].minor);
}
#line 1175 "LuaGrammar.cpp"
        break;
      case 16: /* stat ::= setlist EQUAL explist1 */
#line 107 "LuaGrammar.y"
{ yygotominor.yy0 = null();   yy_destructor(yypParser,11,&yymsp[-1].minor);
}
#line 1181 "LuaGrammar.cpp"
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
      case 55: /* var ::= NAME */ yytestcase(yyruleno==55);
      case 58: /* prefixexp ::= var */ yytestcase(yyruleno==58);
      case 59: /* prefixexp ::= functioncall */ yytestcase(yyruleno==59);
#line 108 "LuaGrammar.y"
{ yygotominor.yy0 = copy(yymsp[0].minor.yy0); }
#line 1198 "LuaGrammar.cpp"
        break;
      case 18: /* repetition ::= FOR NAME EQUAL explist23 */
#line 110 "LuaGrammar.y"
{
  yy_destructor(yypParser,12,&yymsp[-3].minor);
  yy_destructor(yypParser,13,&yymsp[-2].minor);
  yy_destructor(yypParser,11,&yymsp[-1].minor);
}
#line 1207 "LuaGrammar.cpp"
        break;
      case 19: /* repetition ::= FOR namelist IN explist1 */
#line 111 "LuaGrammar.y"
{
  yy_destructor(yypParser,12,&yymsp[-3].minor);
  yy_destructor(yypParser,14,&yymsp[-1].minor);
}
#line 1215 "LuaGrammar.cpp"
        break;
      case 21: /* conds ::= condlist ELSE block */
#line 114 "LuaGrammar.y"
{ yygotominor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[0].minor.yy0);   yy_destructor(yypParser,15,&yymsp[-1].minor);
}
#line 1221 "LuaGrammar.cpp"
        break;
      case 23: /* condlist ::= condlist ELSEIF cond */
#line 116 "LuaGrammar.y"
{ yygotominor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[0].minor.yy0);   yy_destructor(yypParser,16,&yymsp[-1].minor);
}
#line 1227 "LuaGrammar.cpp"
        break;
      case 24: /* cond ::= exp THEN block */
#line 117 "LuaGrammar.y"
{ yygotominor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[0].minor.yy0);   yy_destructor(yypParser,17,&yymsp[-1].minor);
}
#line 1233 "LuaGrammar.cpp"
        break;
      case 25: /* laststat ::= BREAK */
#line 119 "LuaGrammar.y"
{ yygotominor.yy0 = null();   yy_destructor(yypParser,18,&yymsp[0].minor);
}
#line 1239 "LuaGrammar.cpp"
        break;
      case 26: /* laststat ::= RETURN */
#line 120 "LuaGrammar.y"
{ yygotominor.yy0 = null();   yy_destructor(yypParser,19,&yymsp[0].minor);
}
#line 1245 "LuaGrammar.cpp"
        break;
      case 27: /* laststat ::= RETURN explist1 */
#line 121 "LuaGrammar.y"
{ yygotominor.yy0 = copy(yymsp[0].minor.yy0);   yy_destructor(yypParser,19,&yymsp[-1].minor);
}
#line 1251 "LuaGrammar.cpp"
        break;
      case 28: /* binding ::= LOCAL namelist */
#line 123 "LuaGrammar.y"
{ yygotominor.yy0 = null();   yy_destructor(yypParser,20,&yymsp[-1].minor);
}
#line 1257 "LuaGrammar.cpp"
        break;
      case 29: /* binding ::= LOCAL namelist EQUAL explist1 */
#line 124 "LuaGrammar.y"
{ yygotominor.yy0 = copy(yymsp[0].minor.yy0);   yy_destructor(yypParser,20,&yymsp[-3].minor);
  yy_destructor(yypParser,11,&yymsp[-1].minor);
}
#line 1264 "LuaGrammar.cpp"
        break;
      case 30: /* binding ::= LOCAL FUNCTION NAME funcbody */
#line 125 "LuaGrammar.y"
{ yygotominor.yy0 = new LuaGrammarToken(new IScriptOutline::FunctionNode(yymsp[-1].minor.yy0->line, yymsp[-1].minor.yy0->text, true, yymsp[0].minor.yy0->node), yymsp[-1].minor.yy0->line);   yy_destructor(yypParser,20,&yymsp[-3].minor);
  yy_destructor(yypParser,10,&yymsp[-2].minor);
}
#line 1271 "LuaGrammar.cpp"
        break;
      case 32: /* funcname ::= dottedname COLON NAME */
#line 128 "LuaGrammar.y"
{ yygotominor.yy0 = new LuaGrammarToken(yymsp[-2].minor.yy0->text + L":" + yymsp[0].minor.yy0->text, yymsp[0].minor.yy0->line);   yy_destructor(yypParser,21,&yymsp[-1].minor);
}
#line 1277 "LuaGrammar.cpp"
        break;
      case 34: /* dottedname ::= dottedname DOT NAME */
#line 131 "LuaGrammar.y"
{ yygotominor.yy0 = new LuaGrammarToken(yymsp[-2].minor.yy0->text + L"." + yymsp[0].minor.yy0->text, yymsp[0].minor.yy0->line);   yy_destructor(yypParser,22,&yymsp[-1].minor);
}
#line 1283 "LuaGrammar.cpp"
        break;
      case 36: /* namelist ::= namelist COMMA NAME */
#line 134 "LuaGrammar.y"
{ yygotominor.yy0 = new LuaGrammarToken(yymsp[-2].minor.yy0->text + L"," + yymsp[0].minor.yy0->text, yymsp[0].minor.yy0->line);   yy_destructor(yypParser,23,&yymsp[-1].minor);
}
#line 1289 "LuaGrammar.cpp"
        break;
      case 38: /* explist1 ::= explist1 COMMA exp */
      case 39: /* explist23 ::= exp COMMA exp */ yytestcase(yyruleno==39);
#line 137 "LuaGrammar.y"
{ yygotominor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[0].minor.yy0);   yy_destructor(yypParser,23,&yymsp[-1].minor);
}
#line 1296 "LuaGrammar.cpp"
        break;
      case 40: /* explist23 ::= exp COMMA exp COMMA exp */
#line 139 "LuaGrammar.y"
{ yygotominor.yy0 = merge(merge(yymsp[-4].minor.yy0, yymsp[-2].minor.yy0), yymsp[0].minor.yy0);   yy_destructor(yypParser,23,&yymsp[-3].minor);
  yy_destructor(yypParser,23,&yymsp[-1].minor);
}
#line 1303 "LuaGrammar.cpp"
        break;
      case 46: /* exp ::= exp OR exp */
#line 155 "LuaGrammar.y"
{ yygotominor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[0].minor.yy0);   yy_destructor(yypParser,24,&yymsp[-1].minor);
}
#line 1309 "LuaGrammar.cpp"
        break;
      case 47: /* exp ::= exp AND exp */
#line 156 "LuaGrammar.y"
{ yygotominor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[0].minor.yy0);   yy_destructor(yypParser,25,&yymsp[-1].minor);
}
#line 1315 "LuaGrammar.cpp"
        break;
      case 48: /* exp ::= exp LESS|LESS_EQUAL|GREATER|GREATER_EQUAL|EQUAL_EQUAL|NOT_EQUAL exp */
      case 50: /* exp ::= exp PLUS|MINUS exp */ yytestcase(yyruleno==50);
      case 51: /* exp ::= exp MULTIPLY|DIVIDE|MODULO exp */ yytestcase(yyruleno==51);
#line 157 "LuaGrammar.y"
{ yygotominor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[0].minor.yy0); }
#line 1322 "LuaGrammar.cpp"
        break;
      case 49: /* exp ::= exp DOT_DOT exp */
#line 158 "LuaGrammar.y"
{ yygotominor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[0].minor.yy0);   yy_destructor(yypParser,32,&yymsp[-1].minor);
}
#line 1328 "LuaGrammar.cpp"
        break;
      case 52: /* exp ::= exp RAISE exp */
#line 161 "LuaGrammar.y"
{ yygotominor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[0].minor.yy0);   yy_destructor(yypParser,40,&yymsp[-1].minor);
}
#line 1334 "LuaGrammar.cpp"
        break;
      case 54: /* setlist ::= setlist COMMA var */
#line 164 "LuaGrammar.y"
{
  yy_destructor(yypParser,23,&yymsp[-1].minor);
}
#line 1341 "LuaGrammar.cpp"
        break;
      case 56: /* var ::= prefixexp LEFT_BRACKET exp RIGHT_BRACKET */
#line 167 "LuaGrammar.y"
{ yygotominor.yy0 = merge(yymsp[-3].minor.yy0, yymsp[-1].minor.yy0);   yy_destructor(yypParser,47,&yymsp[-2].minor);
  yy_destructor(yypParser,48,&yymsp[0].minor);
}
#line 1348 "LuaGrammar.cpp"
        break;
      case 57: /* var ::= prefixexp DOT NAME */
#line 168 "LuaGrammar.y"
{ yygotominor.yy0 = new LuaGrammarToken(as_string(yymsp[-2].minor.yy0) + L"." + yymsp[0].minor.yy0->text, yymsp[0].minor.yy0->line);   yy_destructor(yypParser,22,&yymsp[-1].minor);
}
#line 1354 "LuaGrammar.cpp"
        break;
      case 60: /* prefixexp ::= OPEN exp RIGHT_PARANTHESIS */
#line 172 "LuaGrammar.y"
{ yygotominor.yy0 = copy(yymsp[-1].minor.yy0);   yy_destructor(yypParser,1,&yymsp[-2].minor);
  yy_destructor(yypParser,49,&yymsp[0].minor);
}
#line 1361 "LuaGrammar.cpp"
        break;
      case 61: /* functioncall ::= prefixexp args */
#line 174 "LuaGrammar.y"
{ yygotominor.yy0 = merge(new LuaGrammarToken(new IScriptOutline::FunctionReferenceNode(yymsp[-1].minor.yy0->line, as_string(yymsp[-1].minor.yy0)), yymsp[-1].minor.yy0->line), yymsp[0].minor.yy0); }
#line 1366 "LuaGrammar.cpp"
        break;
      case 62: /* functioncall ::= prefixexp COLON NAME args */
#line 175 "LuaGrammar.y"
{ yygotominor.yy0 = merge(new LuaGrammarToken(new IScriptOutline::FunctionReferenceNode(yymsp[-1].minor.yy0->line, as_string(yymsp[-3].minor.yy0) + L":" + yymsp[-1].minor.yy0->text), yymsp[-1].minor.yy0->line), yymsp[0].minor.yy0);   yy_destructor(yypParser,21,&yymsp[-2].minor);
}
#line 1372 "LuaGrammar.cpp"
        break;
      case 63: /* args ::= LEFT_PARANTHESIS RIGHT_PARANTHESIS */
#line 177 "LuaGrammar.y"
{ yygotominor.yy0 = null();   yy_destructor(yypParser,2,&yymsp[-1].minor);
  yy_destructor(yypParser,49,&yymsp[0].minor);
}
#line 1379 "LuaGrammar.cpp"
        break;
      case 64: /* args ::= LEFT_PARANTHESIS explist1 RIGHT_PARANTHESIS */
#line 178 "LuaGrammar.y"
{ yygotominor.yy0 = copy(yymsp[-1].minor.yy0);   yy_destructor(yypParser,2,&yymsp[-2].minor);
  yy_destructor(yypParser,49,&yymsp[0].minor);
}
#line 1386 "LuaGrammar.cpp"
        break;
      case 66: /* args ::= STRING */
#line 180 "LuaGrammar.y"
{ yygotominor.yy0 = null();   yy_destructor(yypParser,45,&yymsp[0].minor);
}
#line 1392 "LuaGrammar.cpp"
        break;
      case 67: /* function ::= FUNCTION funcbody */
#line 182 "LuaGrammar.y"
{ yygotominor.yy0 = copy(yymsp[0].minor.yy0);   yy_destructor(yypParser,10,&yymsp[-1].minor);
}
#line 1398 "LuaGrammar.cpp"
        break;
      case 68: /* funcbody ::= params block END */
#line 184 "LuaGrammar.y"
{ yygotominor.yy0 = copy(yymsp[-1].minor.yy0);   yy_destructor(yypParser,6,&yymsp[0].minor);
}
#line 1404 "LuaGrammar.cpp"
        break;
      case 69: /* params ::= LEFT_PARANTHESIS parlist RIGHT_PARANTHESIS */
#line 186 "LuaGrammar.y"
{
  yy_destructor(yypParser,2,&yymsp[-2].minor);
  yy_destructor(yypParser,49,&yymsp[0].minor);
}
#line 1412 "LuaGrammar.cpp"
        break;
      case 72: /* parlist ::= DOT_DOT_DOT */
#line 190 "LuaGrammar.y"
{
  yy_destructor(yypParser,46,&yymsp[0].minor);
}
#line 1419 "LuaGrammar.cpp"
        break;
      case 73: /* parlist ::= namelist COMMA DOT_DOT_DOT */
#line 191 "LuaGrammar.y"
{
  yy_destructor(yypParser,23,&yymsp[-1].minor);
  yy_destructor(yypParser,46,&yymsp[0].minor);
}
#line 1427 "LuaGrammar.cpp"
        break;
      case 74: /* tableconstructor ::= LEFT_BRACE RIGHT_BRACE */
#line 193 "LuaGrammar.y"
{
  yy_destructor(yypParser,50,&yymsp[-1].minor);
  yy_destructor(yypParser,51,&yymsp[0].minor);
}
#line 1435 "LuaGrammar.cpp"
        break;
      case 75: /* tableconstructor ::= LEFT_BRACE fieldlist RIGHT_BRACE */
#line 194 "LuaGrammar.y"
{
  yy_destructor(yypParser,50,&yymsp[-2].minor);
  yy_destructor(yypParser,51,&yymsp[0].minor);
}
#line 1443 "LuaGrammar.cpp"
        break;
      case 76: /* tableconstructor ::= LEFT_BRACE fieldlist COMMA|SEMICOLON RIGHT_BRACE */
#line 195 "LuaGrammar.y"
{
  yy_destructor(yypParser,50,&yymsp[-3].minor);
  yy_destructor(yypParser,51,&yymsp[0].minor);
}
#line 1451 "LuaGrammar.cpp"
        break;
      case 80: /* field ::= NAME EQUAL exp */
#line 201 "LuaGrammar.y"
{
  yy_destructor(yypParser,13,&yymsp[-2].minor);
  yy_destructor(yypParser,11,&yymsp[-1].minor);
}
#line 1459 "LuaGrammar.cpp"
        break;
      case 81: /* field ::= LEFT_BRACKET exp RIGHT_BRACKET EQUAL exp */
#line 202 "LuaGrammar.y"
{
  yy_destructor(yypParser,47,&yymsp[-4].minor);
  yy_destructor(yypParser,48,&yymsp[-2].minor);
  yy_destructor(yypParser,11,&yymsp[-1].minor);
}
#line 1468 "LuaGrammar.cpp"
        break;
      default:
      /* (2) semi ::= */ yytestcase(yyruleno==2);
      /* (53) setlist ::= var */ yytestcase(yyruleno==53);
      /* (70) parlist ::= */ yytestcase(yyruleno==70);
      /* (71) parlist ::= namelist */ yytestcase(yyruleno==71);
      /* (77) fieldlist ::= field */ yytestcase(yyruleno==77);
      /* (78) fieldlist ::= fieldlist COMMA|SEMICOLON field */ yytestcase(yyruleno==78);
      /* (79) field ::= exp */ yytestcase(yyruleno==79);
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

#line 1542 "LuaGrammar.cpp"
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