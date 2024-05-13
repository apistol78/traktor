/*
** 2000-05-29
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** Driver template for the LEMON parser generator.
**
** The "lemon" program processes an LALR(1) input grammar file, then uses
** this template to construct a parser.  The "lemon" program inserts text
** at each "%%" line.  Also, any "P-a-r-s-e" identifer prefix (without the
** interstitial "-" characters) contained in this template is changed into
** the value of the %name directive from the grammar.  Otherwise, the content
** of this template is copied straight through into the generate parser
** source file.
**
** The following is the concatenation of all %include directives from the
** input grammar file:
*/
#include <stdio.h>
/************ Begin %include sections from the grammar ************************/


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
		return nullptr;
}

bool is_node(const LuaGrammarToken* A)
{
	return A != nullptr && A->node != nullptr;
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

/**************** End of %include directives **********************************/
/* These constants specify the various numeric values for terminal symbols
** in a format understandable to "makeheaders".  This section is blank unless
** "lemon" is run with the "-m" command-line option.
***************** Begin makeheaders token definitions *************************/
/**************** End makeheaders token definitions ***************************/

/* The next sections is a series of control #defines.
** various aspects of the generated parser.
**    YYCODETYPE         is the data type used to store the integer codes
**                       that represent terminal and non-terminal symbols.
**                       "unsigned char" is used if there are fewer than
**                       256 symbols.  Larger types otherwise.
**    YYNOCODE           is a number of type YYCODETYPE that is not used for
**                       any terminal or nonterminal symbol.
**    YYFALLBACK         If defined, this indicates that one or more tokens
**                       (also known as: "terminal symbols") have fall-back
**                       values which should be used if the original symbol
**                       would not parse.  This permits keywords to sometimes
**                       be used as identifiers, for example.
**    YYACTIONTYPE       is the data type used for "action codes" - numbers
**                       that indicate what to do in response to the next
**                       token.
**    ParseTOKENTYPE     is the data type used for minor type for terminal
**                       symbols.  Background: A "minor type" is a semantic
**                       value associated with a terminal or non-terminal
**                       symbols.  For example, for an "ID" terminal symbol,
**                       the minor type might be the name of the identifier.
**                       Each non-terminal can have a different minor type.
**                       Terminal symbols all have the same minor type, though.
**                       This macros defines the minor type for terminal 
**                       symbols.
**    YYMINORTYPE        is the data type used for all minor types.
**                       This is typically a union of many types, one of
**                       which is ParseTOKENTYPE.  The entry in the union
**                       for terminal symbols is called "yy0".
**    YYSTACKDEPTH       is the maximum depth of the parser's stack.  If
**                       zero the stack is dynamically sized using realloc()
**    ParseARG_SDECL     A static variable declaration for the %extra_argument
**    ParseARG_PDECL     A parameter declaration for the %extra_argument
**    ParseARG_PARAM     Code to pass %extra_argument as a subroutine parameter
**    ParseARG_STORE     Code to store %extra_argument into yypParser
**    ParseARG_FETCH     Code to extract %extra_argument from yypParser
**    ParseCTX_*         As ParseARG_ except for %extra_context
**    YYERRORSYMBOL      is the code number of the error symbol.  If not
**                       defined, then do no error processing.
**    YYNSTATE           the combined number of states.
**    YYNRULE            the number of rules in the grammar
**    YYNTOKEN           Number of terminal symbols
**    YY_MAX_SHIFT       Maximum value for shift actions
**    YY_MIN_SHIFTREDUCE Minimum value for shift-reduce actions
**    YY_MAX_SHIFTREDUCE Maximum value for shift-reduce actions
**    YY_ERROR_ACTION    The yy_action[] code for syntax error
**    YY_ACCEPT_ACTION   The yy_action[] code for accept
**    YY_NO_ACTION       The yy_action[] code for no-op
**    YY_MIN_REDUCE      Minimum value for reduce actions
**    YY_MAX_REDUCE      Maximum value for reduce actions
*/
#ifndef INTERFACE
# define INTERFACE 1
#endif
/************* Begin control #defines *****************************************/
#define YYCODETYPE unsigned char
#define YYNOCODE 86
#define YYACTIONTYPE unsigned short int
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
#define ParseARG_PARAM ,outline 
#define ParseARG_FETCH  const traktor::script::ScriptOutlineLua* outline =yypParser->outline ;
#define ParseARG_STORE yypParser->outline =outline ;
#define ParseCTX_SDECL
#define ParseCTX_PDECL
#define ParseCTX_PARAM
#define ParseCTX_FETCH
#define ParseCTX_STORE
#define YYFALLBACK 1
#define YYNSTATE             117
#define YYNRULE              85
#define YYNTOKEN             55
#define YY_MAX_SHIFT         116
#define YY_MIN_SHIFTREDUCE   164
#define YY_MAX_SHIFTREDUCE   248
#define YY_ERROR_ACTION      249
#define YY_ACCEPT_ACTION     250
#define YY_NO_ACTION         251
#define YY_MIN_REDUCE        252
#define YY_MAX_REDUCE        336
/************* End control #defines *******************************************/

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
**   0 <= N <= YY_MAX_SHIFT             Shift N.  That is, push the lookahead
**                                      token onto the stack and goto state N.
**
**   N between YY_MIN_SHIFTREDUCE       Shift to an arbitrary state then
**     and YY_MAX_SHIFTREDUCE           reduce by rule N-YY_MIN_SHIFTREDUCE.
**
**   N == YY_ERROR_ACTION               A syntax error has occurred.
**
**   N == YY_ACCEPT_ACTION              The parser accepts its input.
**
**   N == YY_NO_ACTION                  No such action.  Denotes unused
**                                      slots in the yy_action[] table.
**
**   N between YY_MIN_REDUCE            Reduce by rule N-YY_MIN_REDUCE
**     and YY_MAX_REDUCE
**
** The action table is constructed as a single large table named yy_action[].
** Given state S and lookahead X, the action is computed as either:
**
**    (A)   N = yy_action[ yy_shift_ofst[S] + X ]
**    (B)   N = yy_default[S]
**
** The (A) formula is preferred.  The B formula is used instead if
** yy_lookahead[yy_shift_ofst[S]+X] is not equal to X.
**
** The formulas above are for computing the action when the lookahead is
** a terminal symbol.  If the lookahead is a non-terminal (as occurs after
** a reduce action) then the yy_reduce_ofst[] array is used in place of
** the yy_shift_ofst[] array.
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
**
*********** Begin parsing tables **********************************************/
#define YY_ACTTAB_COUNT (803)
static const YYACTIONTYPE yy_action[] = {
 /*     0 */    57,  308,  250,  116,  103,   54,   38,   20,   35,  308,
 /*    10 */    84,   90,   87,  270,  196,  291,   36,  293,  307,  216,
 /*    20 */     5,   27,  314,   27,  310,  279,   13,   31,   28,   27,
 /*    30 */    27,   27,   27,   26,   23,   23,   22,   22,   22,   46,
 /*    40 */   194,   21,  107,  108,  196,   27,    4,   27,   25,   24,
 /*    50 */    14,   31,   28,   27,   27,   27,   27,   26,   23,   23,
 /*    60 */    22,   22,   22,   77,  227,   21,  102,  101,   30,    4,
 /*    70 */    43,    2,   25,   24,   27,  314,   27,  311,  239,   99,
 /*    80 */    31,   28,   27,   27,   27,   27,   26,   23,   23,   22,
 /*    90 */    22,   22,   83,  112,   21,  230,   16,  242,   27,    3,
 /*   100 */    27,   25,   24,  111,   31,   28,   27,   27,   27,   27,
 /*   110 */    26,   23,   23,   22,   22,   22,  265,  259,   21,   22,
 /*   120 */    22,   22,  225,   16,   21,   25,   24,   27,  217,   27,
 /*   130 */    46,   25,   24,   31,   28,   27,   27,   27,   27,   26,
 /*   140 */    23,   23,   22,   22,   22,  257,  104,   21,   54,   44,
 /*   150 */    12,  263,   21,  254,   25,   24,   27,  109,   27,   25,
 /*   160 */    24,  197,   31,   28,   27,   27,   27,   27,   26,   23,
 /*   170 */    23,   22,   22,   22,   47,  316,   21,   40,    7,  322,
 /*   180 */    93,    6,   92,   25,   24,   39,   86,  221,   94,   46,
 /*   190 */    27,  197,   27,   97,  322,  240,   31,   28,   27,   27,
 /*   200 */    27,   27,   26,   23,   23,   22,   22,   22,   10,  272,
 /*   210 */    21,   54,   15,  269,   27,   54,   27,   25,   24,  195,
 /*   220 */    31,   28,   27,   27,   27,   27,   26,   23,   23,   22,
 /*   230 */    22,   22,  193,  105,   21,   54,   27,  176,   27,  323,
 /*   240 */   174,   25,   24,   28,   27,   27,   27,   27,   26,   23,
 /*   250 */    23,   22,   22,   22,  323,  113,   21,   54,   20,  114,
 /*   260 */   115,   54,   54,   25,   24,   45,   27,   42,   27,   37,
 /*   270 */   110,    5,    9,   11,   27,   27,   27,   27,   26,   23,
 /*   280 */    23,   22,   22,   22,   97,  218,   21,   17,   33,   19,
 /*   290 */   236,   89,   29,   25,   24,  229,   29,   29,  173,  202,
 /*   300 */   202,  202,  202,  202,  202,   20,  172,   18,  252,  251,
 /*   310 */     2,  241,   20,  251,   42,  227,   48,  110,   32,   34,
 /*   320 */     1,   41,    2,   50,  216,  251,  251,  251,  251,  185,
 /*   330 */     8,   49,  251,  251,  251,  251,  251,  251,  251,   29,
 /*   340 */   251,  251,  251,   29,   29,  251,  202,  202,  202,  202,
 /*   350 */   202,  202,   20,  251,   18,  251,  251,    2,  243,  251,
 /*   360 */   251,   42,  251,  251,  216,   26,   23,   23,   22,   22,
 /*   370 */    22,  251,  251,   21,  251,  251,  251,  251,  251,  251,
 /*   380 */    25,   24,  251,  251,  251,  251,   29,  251,  251,  251,
 /*   390 */    29,   29,  251,  202,  202,  202,  202,  202,  202,   20,
 /*   400 */   251,  251,  251,  224,    2,  251,   68,  251,   42,  251,
 /*   410 */   251,  216,  251,  251,  251,  308,  251,  251,  251,  251,
 /*   420 */   251,  291,   36,  293,  307,  251,  251,  251,   78,   78,
 /*   430 */   251,  251,  251,   29,  251,  251,  251,   29,   29,  251,
 /*   440 */   202,  202,  202,  202,  202,  202,  251,  251,  251,  251,
 /*   450 */   251,    2,   53,  251,  251,   52,   51,  106,   68,  251,
 /*   460 */   251,   85,  251,   75,  251,  251,  251,  308,  251,   65,
 /*   470 */    35,  251,   82,  291,   36,  293,  307,   88,  308,  251,
 /*   480 */   251,  333,  251,  251,  291,   36,  293,  307,  251,  251,
 /*   490 */   251,   65,  251,  251,  251,  251,   65,  251,  251,   91,
 /*   500 */   308,  251,  251,  251,   95,  308,  291,   36,  293,  307,
 /*   510 */   251,  291,   36,  293,  307,   65,  251,  251,  251,   65,
 /*   520 */   251,  251,  251,   96,  308,  251,  251,   98,  308,  251,
 /*   530 */   291,   36,  293,  307,  291,   36,  293,  307,   56,  251,
 /*   540 */   251,  251,  251,  251,  251,  251,  251,  308,  320,   65,
 /*   550 */   251,  251,  251,  291,   36,  293,  307,  100,  308,   57,
 /*   560 */   251,  251,  251,  251,  291,   36,  293,  307,  308,   62,
 /*   570 */   251,  251,  271,  251,  291,   36,  293,  307,  308,  251,
 /*   580 */   251,  251,   55,  251,  291,   36,  293,  307,  251,  251,
 /*   590 */   251,  308,  251,   63,  251,  251,  251,  291,   36,  293,
 /*   600 */   307,  251,  308,  251,  251,  251,  251,  251,  291,   36,
 /*   610 */   293,  307,   64,  251,  251,  251,   66,  251,  251,  251,
 /*   620 */   251,  308,  251,  251,  251,  308,  251,  291,   36,  293,
 /*   630 */   307,  291,   36,  293,  307,   59,  251,  251,  251,   67,
 /*   640 */   251,  251,  251,  251,  308,  251,  251,  251,  308,   60,
 /*   650 */   291,   36,  293,  307,  291,   36,  293,  307,  308,  251,
 /*   660 */    79,  251,  251,  251,  291,   36,  293,  307,  251,  308,
 /*   670 */   251,   80,  251,  251,  308,  291,   36,  293,  307,  251,
 /*   680 */   308,  251,  251,  251,   76,  251,  291,   36,  293,  307,
 /*   690 */   251,  251,  251,  308,   69,  251,  308,  308,   70,  291,
 /*   700 */    36,  293,  307,  308,  251,  251,  251,  308,  251,  291,
 /*   710 */    36,  293,  307,  291,   36,  293,  307,   73,  308,  251,
 /*   720 */   251,   74,  308,  251,  251,  308,  308,  251,  251,  251,
 /*   730 */   308,  251,  291,   36,  293,  307,  291,   36,  293,  307,
 /*   740 */    72,  251,  251,  251,  251,  251,  251,  251,  251,  308,
 /*   750 */    81,  251,  251,  251,  251,  291,   36,  293,  307,  308,
 /*   760 */   251,   58,  251,  251,  251,  291,   36,  293,  307,  251,
 /*   770 */   308,  251,   71,  251,  251,  251,  291,   36,  293,  307,
 /*   780 */   251,  308,  251,  251,   61,  251,  251,  291,   36,  293,
 /*   790 */   307,  251,  251,  308,  251,  251,  251,  251,  251,  291,
 /*   800 */    36,  293,  307,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */    62,   71,   55,   56,   66,   58,    2,    1,   78,   71,
 /*    10 */    80,   10,   74,   75,   13,   77,   78,   79,   80,   13,
 /*    20 */     2,   21,   79,   23,   81,   68,   26,   27,   28,   29,
 /*    30 */    30,   31,   32,   33,   34,   35,   36,   37,   38,   82,
 /*    40 */    13,   41,   24,   25,   13,   21,    3,   23,   48,   49,
 /*    50 */    26,   27,   28,   29,   30,   31,   32,   33,   34,   35,
 /*    60 */    36,   37,   38,   73,   46,   41,   24,   25,   50,   26,
 /*    70 */    17,   53,   48,   49,   21,   79,   23,   81,   47,   13,
 /*    80 */    27,   28,   29,   30,   31,   32,   33,   34,   35,   36,
 /*    90 */    37,   38,   73,   73,   41,    3,   26,   54,   21,   59,
 /*   100 */    23,   48,   49,   83,   27,   28,   29,   30,   31,   32,
 /*   110 */    33,   34,   35,   36,   37,   38,   68,   57,   41,   36,
 /*   120 */    37,   38,   52,   26,   41,   48,   49,   21,   51,   23,
 /*   130 */    82,   48,   49,   27,   28,   29,   30,   31,   32,   33,
 /*   140 */    34,   35,   36,   37,   38,   57,   56,   41,   58,   15,
 /*   150 */    16,   61,   41,   57,   48,   49,   21,   51,   23,   48,
 /*   160 */    49,   13,   27,   28,   29,   30,   31,   32,   33,   34,
 /*   170 */    35,   36,   37,   38,    5,   68,   41,   67,   11,   11,
 /*   180 */    22,   11,   23,   48,   49,   13,   76,   52,   21,   82,
 /*   190 */    21,   13,   23,   26,   26,   47,   27,   28,   29,   30,
 /*   200 */    31,   32,   33,   34,   35,   36,   37,   38,   11,   56,
 /*   210 */    41,   58,    4,   56,   21,   58,   23,   48,   49,   13,
 /*   220 */    27,   28,   29,   30,   31,   32,   33,   34,   35,   36,
 /*   230 */    37,   38,   13,   56,   41,   58,   21,    6,   23,   11,
 /*   240 */     6,   48,   49,   28,   29,   30,   31,   32,   33,   34,
 /*   250 */    35,   36,   37,   38,   26,   56,   41,   58,    1,   56,
 /*   260 */    56,   58,   58,   48,   49,    5,   21,   10,   23,   13,
 /*   270 */    13,    2,   14,   11,   29,   30,   31,   32,   33,   34,
 /*   280 */    35,   36,   37,   38,   26,   13,   41,   11,   26,   11,
 /*   290 */    52,   26,   35,   48,   49,    6,   39,   40,    6,   42,
 /*   300 */    43,   44,   45,   46,   47,    1,    6,   50,    0,   86,
 /*   310 */    53,   54,    1,   86,   10,   46,    5,   13,    7,    8,
 /*   320 */     9,   10,   53,   12,   13,   86,   86,   86,   86,   18,
 /*   330 */    19,   20,   86,   86,   86,   86,   86,   86,   86,   35,
 /*   340 */    86,   86,   86,   39,   40,   86,   42,   43,   44,   45,
 /*   350 */    46,   47,    1,   86,   50,   86,   86,   53,   54,   86,
 /*   360 */    86,   10,   86,   86,   13,   33,   34,   35,   36,   37,
 /*   370 */    38,   86,   86,   41,   86,   86,   86,   86,   86,   86,
 /*   380 */    48,   49,   86,   86,   86,   86,   35,   86,   86,   86,
 /*   390 */    39,   40,   86,   42,   43,   44,   45,   46,   47,    1,
 /*   400 */    86,   86,   86,   52,   53,   86,   62,   86,   10,   86,
 /*   410 */    86,   13,   86,   86,   86,   71,   86,   86,   86,   86,
 /*   420 */    86,   77,   78,   79,   80,   86,   86,   86,   84,   85,
 /*   430 */    86,   86,   86,   35,   86,   86,   86,   39,   40,   86,
 /*   440 */    42,   43,   44,   45,   46,   47,   86,   86,   86,   86,
 /*   450 */    86,   53,   60,   86,   86,   63,   64,   65,   62,   86,
 /*   460 */    86,   69,   86,   71,   86,   86,   86,   71,   86,   62,
 /*   470 */    78,   86,   80,   77,   78,   79,   80,   70,   71,   86,
 /*   480 */    86,   85,   86,   86,   77,   78,   79,   80,   86,   86,
 /*   490 */    86,   62,   86,   86,   86,   86,   62,   86,   86,   70,
 /*   500 */    71,   86,   86,   86,   70,   71,   77,   78,   79,   80,
 /*   510 */    86,   77,   78,   79,   80,   62,   86,   86,   86,   62,
 /*   520 */    86,   86,   86,   70,   71,   86,   86,   70,   71,   86,
 /*   530 */    77,   78,   79,   80,   77,   78,   79,   80,   62,   86,
 /*   540 */    86,   86,   86,   86,   86,   86,   86,   71,   72,   62,
 /*   550 */    86,   86,   86,   77,   78,   79,   80,   70,   71,   62,
 /*   560 */    86,   86,   86,   86,   77,   78,   79,   80,   71,   62,
 /*   570 */    86,   86,   75,   86,   77,   78,   79,   80,   71,   86,
 /*   580 */    86,   86,   62,   86,   77,   78,   79,   80,   86,   86,
 /*   590 */    86,   71,   86,   62,   86,   86,   86,   77,   78,   79,
 /*   600 */    80,   86,   71,   86,   86,   86,   86,   86,   77,   78,
 /*   610 */    79,   80,   62,   86,   86,   86,   62,   86,   86,   86,
 /*   620 */    86,   71,   86,   86,   86,   71,   86,   77,   78,   79,
 /*   630 */    80,   77,   78,   79,   80,   62,   86,   86,   86,   62,
 /*   640 */    86,   86,   86,   86,   71,   86,   86,   86,   71,   62,
 /*   650 */    77,   78,   79,   80,   77,   78,   79,   80,   71,   86,
 /*   660 */    62,   86,   86,   86,   77,   78,   79,   80,   86,   71,
 /*   670 */    86,   62,   86,   86,    2,   77,   78,   79,   80,   86,
 /*   680 */    71,   86,   86,   86,   62,   86,   77,   78,   79,   80,
 /*   690 */    86,   86,   86,   71,   62,   86,   24,   25,   62,   77,
 /*   700 */    78,   79,   80,   71,   86,   86,   86,   71,   86,   77,
 /*   710 */    78,   79,   80,   77,   78,   79,   80,   62,   46,   86,
 /*   720 */    86,   62,   50,   86,   86,   53,   71,   86,   86,   86,
 /*   730 */    71,   86,   77,   78,   79,   80,   77,   78,   79,   80,
 /*   740 */    62,   86,   86,   86,   86,   86,   86,   86,   86,   71,
 /*   750 */    62,   86,   86,   86,   86,   77,   78,   79,   80,   71,
 /*   760 */    86,   62,   86,   86,   86,   77,   78,   79,   80,   86,
 /*   770 */    71,   86,   62,   86,   86,   86,   77,   78,   79,   80,
 /*   780 */    86,   71,   86,   86,   62,   86,   86,   77,   78,   79,
 /*   790 */    80,   86,   86,   71,   86,   86,   86,   86,   86,   77,
 /*   800 */    78,   79,   80,
};
#define YY_SHIFT_COUNT    (116)
#define YY_SHIFT_MIN      (0)
#define YY_SHIFT_MAX      (672)
static const unsigned short int yy_shift_ofst[] = {
 /*     0 */   803,  398,  257,  311,  304,  351,  398,  398,  398,  398,
 /*    10 */   398,  398,  398,  398,  398,  398,  398,  398,  398,  398,
 /*    20 */   398,  398,  398,  398,  398,  398,  398,  398,  398,  398,
 /*    30 */   398,  398,  398,    6,  803,   18,   18,  269,   31,    4,
 /*    40 */     4,   27,    4,  803,  803,  803,  803,  803,  803,    1,
 /*    50 */    66,   92,   92,   92,  803,    0,   24,   53,   77,  106,
 /*    60 */   135,  169,  193,  193,  193,  193,  193,  193,  193,  193,
 /*    70 */   193,  215,  245,  332,  332,  672,   83,  167,   43,  111,
 /*    80 */   111,  111,  168,  258,  228,  262,   42,  134,   70,  148,
 /*    90 */   172,   97,  170,  159,  158,   97,   97,  178,   97,  197,
 /*   100 */    97,  206,  219,  231,  208,  234,  260,  256,  272,  276,
 /*   110 */   278,  238,  265,  289,  292,  300,  308,
};
#define YY_REDUCE_COUNT (54)
#define YY_REDUCE_MIN   (-70)
#define YY_REDUCE_MAX   (722)
static const short yy_reduce_ofst[] = {
 /*     0 */   -53,  -62,  344,  392,  396,  407,  429,  434,  453,  457,
 /*    10 */   476,  487,  497,  507,  520,  531,  550,  554,  573,  577,
 /*    20 */   587,  598,  609,  622,  632,  636,  655,  659,  678,  688,
 /*    30 */   699,  710,  722,  -70,   90,  -57,  -57,   -4,   20,  -43,
 /*    40 */    48,  110,  107,  153,  157,  177,  199,  203,  204,  -10,
 /*    50 */    19,   60,   88,   96,   40,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   256,  249,  249,  253,  249,  249,  249,  249,  274,  249,
 /*    10 */   249,  249,  249,  249,  249,  249,  249,  249,  249,  249,
 /*    20 */   249,  249,  249,  249,  249,  249,  249,  249,  249,  249,
 /*    30 */   249,  249,  249,  249,  256,  249,  292,  249,  325,  249,
 /*    40 */   249,  249,  249,  256,  256,  256,  256,  256,  256,  249,
 /*    50 */   249,  319,  319,  319,  258,  288,  249,  249,  249,  249,
 /*    60 */   249,  249,  289,  255,  287,  286,  336,  335,  334,  300,
 /*    70 */   299,  295,  296,  298,  297,  267,  301,  276,  249,  303,
 /*    80 */   302,  294,  307,  249,  307,  249,  280,  268,  249,  249,
 /*    90 */   249,  278,  249,  249,  249,  277,  275,  249,  321,  284,
 /*   100 */   266,  249,  249,  249,  249,  249,  249,  249,  249,  249,
 /*   110 */   304,  249,  326,  249,  249,  249,  249,
};
/********** End of lemon-generated parsing tables *****************************/

/* The next table maps tokens (terminal symbols) into fallback tokens.  
** If a construct like the following:
** 
**      %fallback ID X Y Z.
**
** appears in the grammar, then ID becomes a fallback token for X, Y,
** and Z.  Whenever one of the tokens X, Y, or Z is input to the parser
** but it does not parse, the type of the token is changed to ID and
** the parse is retried before an error is thrown.
**
** This feature can be used, for example, to cause some keywords in a language
** to revert to identifiers if they keyword does not apply in the context where
** it appears.
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
**
** After the "shift" half of a SHIFTREDUCE action, the stateno field
** actually contains the reduce action for the second half of the
** SHIFTREDUCE.
*/
struct yyStackEntry {
  YYACTIONTYPE stateno;  /* The state-number, or reduce action in SHIFTREDUCE */
  YYCODETYPE major;      /* The major token value.  This is the code
                         ** number for the token at this stack level */
  YYMINORTYPE minor;     /* The user-supplied minor token value.  This
                         ** is the value of the token  */
};
typedef struct yyStackEntry yyStackEntry;

/* The state of the parser is completely contained in an instance of
** the following structure */
struct yyParser {
  yyStackEntry *yytos;          /* Pointer to top element of the stack */
#ifdef YYTRACKMAXSTACKDEPTH
  int yyhwm;                    /* High-water mark of the stack */
#endif
#ifndef YYNOERRORRECOVERY
  int yyerrcnt;                 /* Shifts left before out of the error */
#endif
  ParseARG_SDECL                /* A place to hold %extra_argument */
  ParseCTX_SDECL                /* A place to hold %extra_context */
#if YYSTACKDEPTH<=0
  int yystksz;                  /* Current side of the stack */
  yyStackEntry *yystack;        /* The parser's stack */
  yyStackEntry yystk0;          /* First stack entry */
#else
  yyStackEntry yystack[YYSTACKDEPTH];  /* The parser's stack */
  yyStackEntry *yystackEnd;            /* Last entry in the stack */
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

#if defined(YYCOVERAGE) || !defined(NDEBUG)
/* For tracing shifts, the names of all terminals and nonterminals
** are required.  The following table supplies these names */
static const char *const yyTokenName[] = { 
  /*    0 */ "$",
  /*    1 */ "OPEN",
  /*    2 */ "LEFT_PARANTHESIS",
  /*    3 */ "SEMICOLON",
  /*    4 */ "UNTIL",
  /*    5 */ "DO",
  /*    6 */ "END",
  /*    7 */ "WHILE",
  /*    8 */ "REPEAT",
  /*    9 */ "IF",
  /*   10 */ "FUNCTION",
  /*   11 */ "EQUAL",
  /*   12 */ "FOR",
  /*   13 */ "NAME",
  /*   14 */ "IN",
  /*   15 */ "ELSE",
  /*   16 */ "ELSEIF",
  /*   17 */ "THEN",
  /*   18 */ "BREAK",
  /*   19 */ "RETURN",
  /*   20 */ "LOCAL",
  /*   21 */ "LESS",
  /*   22 */ "CONST",
  /*   23 */ "GREATER",
  /*   24 */ "COLON",
  /*   25 */ "DOT",
  /*   26 */ "COMMA",
  /*   27 */ "OR",
  /*   28 */ "AND",
  /*   29 */ "LESS_EQUAL",
  /*   30 */ "GREATER_EQUAL",
  /*   31 */ "EQUAL_EQUAL",
  /*   32 */ "NOT_EQUAL",
  /*   33 */ "DOT_DOT",
  /*   34 */ "PLUS",
  /*   35 */ "MINUS",
  /*   36 */ "MULTIPLY",
  /*   37 */ "DIVIDE",
  /*   38 */ "MODULO",
  /*   39 */ "NOT",
  /*   40 */ "HASH",
  /*   41 */ "RAISE",
  /*   42 */ "NIL",
  /*   43 */ "TRUE",
  /*   44 */ "FALSE",
  /*   45 */ "NUMBER",
  /*   46 */ "STRING",
  /*   47 */ "DOT_DOT_DOT",
  /*   48 */ "BITWISE_OR",
  /*   49 */ "BITWISE_AND",
  /*   50 */ "LEFT_BRACKET",
  /*   51 */ "RIGHT_BRACKET",
  /*   52 */ "RIGHT_PARANTHESIS",
  /*   53 */ "LEFT_BRACE",
  /*   54 */ "RIGHT_BRACE",
  /*   55 */ "chunk",
  /*   56 */ "block",
  /*   57 */ "semi",
  /*   58 */ "scope",
  /*   59 */ "statlist",
  /*   60 */ "laststat",
  /*   61 */ "ublock",
  /*   62 */ "exp",
  /*   63 */ "binding",
  /*   64 */ "stat",
  /*   65 */ "repetition",
  /*   66 */ "conds",
  /*   67 */ "funcname",
  /*   68 */ "funcbody",
  /*   69 */ "setlist",
  /*   70 */ "explist1",
  /*   71 */ "functioncall",
  /*   72 */ "explist23",
  /*   73 */ "namelist",
  /*   74 */ "condlist",
  /*   75 */ "cond",
  /*   76 */ "dottedname",
  /*   77 */ "function",
  /*   78 */ "prefixexp",
  /*   79 */ "tableconstructor",
  /*   80 */ "var",
  /*   81 */ "args",
  /*   82 */ "params",
  /*   83 */ "parlist",
  /*   84 */ "fieldlist",
  /*   85 */ "field",
};
#endif /* defined(YYCOVERAGE) || !defined(NDEBUG) */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *const yyRuleName[] = {
 /*   0 */ "chunk ::= block",
 /*   1 */ "block ::= scope statlist",
 /*   2 */ "block ::= scope statlist laststat semi",
 /*   3 */ "ublock ::= block UNTIL exp",
 /*   4 */ "scope ::=",
 /*   5 */ "scope ::= scope statlist binding semi",
 /*   6 */ "statlist ::=",
 /*   7 */ "statlist ::= statlist stat semi",
 /*   8 */ "stat ::= DO block END",
 /*   9 */ "stat ::= WHILE exp DO block END",
 /*  10 */ "stat ::= repetition DO block END",
 /*  11 */ "stat ::= REPEAT ublock",
 /*  12 */ "stat ::= IF conds END",
 /*  13 */ "stat ::= FUNCTION funcname funcbody",
 /*  14 */ "stat ::= setlist EQUAL explist1",
 /*  15 */ "stat ::= functioncall",
 /*  16 */ "conds ::= condlist",
 /*  17 */ "conds ::= condlist ELSE block",
 /*  18 */ "condlist ::= cond",
 /*  19 */ "condlist ::= condlist ELSEIF cond",
 /*  20 */ "cond ::= exp THEN block",
 /*  21 */ "laststat ::= BREAK",
 /*  22 */ "laststat ::= RETURN",
 /*  23 */ "laststat ::= RETURN explist1",
 /*  24 */ "binding ::= LOCAL namelist",
 /*  25 */ "binding ::= LOCAL namelist EQUAL explist1",
 /*  26 */ "binding ::= LOCAL namelist LESS CONST GREATER EQUAL explist1",
 /*  27 */ "binding ::= LOCAL FUNCTION NAME funcbody",
 /*  28 */ "funcname ::= dottedname",
 /*  29 */ "funcname ::= dottedname COLON NAME",
 /*  30 */ "dottedname ::= NAME",
 /*  31 */ "dottedname ::= dottedname DOT NAME",
 /*  32 */ "namelist ::= NAME",
 /*  33 */ "namelist ::= namelist COMMA NAME",
 /*  34 */ "explist1 ::= exp",
 /*  35 */ "explist1 ::= explist1 COMMA exp",
 /*  36 */ "explist23 ::= exp COMMA exp",
 /*  37 */ "explist23 ::= exp COMMA exp COMMA exp",
 /*  38 */ "exp ::= NIL|TRUE|FALSE|NUMBER|STRING|DOT_DOT_DOT",
 /*  39 */ "exp ::= function",
 /*  40 */ "exp ::= prefixexp",
 /*  41 */ "exp ::= tableconstructor",
 /*  42 */ "exp ::= NOT|HASH|MINUS exp",
 /*  43 */ "exp ::= exp OR exp",
 /*  44 */ "exp ::= exp AND exp",
 /*  45 */ "exp ::= exp LESS|LESS_EQUAL|GREATER|GREATER_EQUAL|EQUAL_EQUAL|NOT_EQUAL exp",
 /*  46 */ "exp ::= exp DOT_DOT exp",
 /*  47 */ "exp ::= exp BITWISE_OR exp",
 /*  48 */ "exp ::= exp BITWISE_AND exp",
 /*  49 */ "exp ::= exp PLUS|MINUS exp",
 /*  50 */ "exp ::= exp MULTIPLY|DIVIDE|MODULO exp",
 /*  51 */ "exp ::= exp RAISE exp",
 /*  52 */ "var ::= NAME",
 /*  53 */ "var ::= prefixexp LEFT_BRACKET exp RIGHT_BRACKET",
 /*  54 */ "var ::= prefixexp DOT NAME",
 /*  55 */ "prefixexp ::= var",
 /*  56 */ "prefixexp ::= functioncall",
 /*  57 */ "prefixexp ::= OPEN exp RIGHT_PARANTHESIS",
 /*  58 */ "functioncall ::= prefixexp args",
 /*  59 */ "functioncall ::= prefixexp COLON NAME args",
 /*  60 */ "args ::= LEFT_PARANTHESIS RIGHT_PARANTHESIS",
 /*  61 */ "args ::= LEFT_PARANTHESIS explist1 RIGHT_PARANTHESIS",
 /*  62 */ "args ::= tableconstructor",
 /*  63 */ "args ::= STRING",
 /*  64 */ "function ::= FUNCTION funcbody",
 /*  65 */ "funcbody ::= params block END",
 /*  66 */ "semi ::= SEMICOLON",
 /*  67 */ "semi ::=",
 /*  68 */ "repetition ::= FOR NAME EQUAL explist23",
 /*  69 */ "repetition ::= FOR namelist IN explist1",
 /*  70 */ "setlist ::= var",
 /*  71 */ "setlist ::= setlist COMMA var",
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
** Try to increase the size of the parser stack.  Return the number
** of errors.  Return 0 on success.
*/
static int yyGrowStack(yyParser *p){
  int newSize;
  int idx;
  yyStackEntry *pNew;

  newSize = p->yystksz*2 + 100;
  idx = p->yytos ? (int)(p->yytos - p->yystack) : 0;
  if( p->yystack==&p->yystk0 ){
    pNew = malloc(newSize*sizeof(pNew[0]));
    if( pNew ) pNew[0] = p->yystk0;
  }else{
    pNew = realloc(p->yystack, newSize*sizeof(pNew[0]));
  }
  if( pNew ){
    p->yystack = pNew;
    p->yytos = &p->yystack[idx];
#ifndef NDEBUG
    if( yyTraceFILE ){
      fprintf(yyTraceFILE,"%sStack grows from %d to %d entries.\n",
              yyTracePrompt, p->yystksz, newSize);
    }
#endif
    p->yystksz = newSize;
  }
  return pNew==0; 
}
#endif

/* Datatype of the argument to the memory allocated passed as the
** second argument to ParseAlloc() below.  This can be changed by
** putting an appropriate #define in the %include section of the input
** grammar.
*/
#ifndef YYMALLOCARGTYPE
# define YYMALLOCARGTYPE size_t
#endif

/* Initialize a new parser that has already been allocated.
*/
void ParseInit(void *yypRawParser ParseCTX_PDECL){
  yyParser *yypParser = (yyParser*)yypRawParser;
  ParseCTX_STORE
#ifdef YYTRACKMAXSTACKDEPTH
  yypParser->yyhwm = 0;
#endif
#if YYSTACKDEPTH<=0
  yypParser->yytos = NULL;
  yypParser->yystack = NULL;
  yypParser->yystksz = 0;
  if( yyGrowStack(yypParser) ){
    yypParser->yystack = &yypParser->yystk0;
    yypParser->yystksz = 1;
  }
#endif
#ifndef YYNOERRORRECOVERY
  yypParser->yyerrcnt = -1;
#endif
  yypParser->yytos = yypParser->yystack;
  yypParser->yystack[0].stateno = 0;
  yypParser->yystack[0].major = 0;
#if YYSTACKDEPTH>0
  yypParser->yystackEnd = &yypParser->yystack[YYSTACKDEPTH-1];
#endif
}

#ifndef Parse_ENGINEALWAYSONSTACK
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
void *ParseAlloc(void *(*mallocProc)(YYMALLOCARGTYPE) ParseCTX_PDECL){
  yyParser *yypParser;
  yypParser = (yyParser*)(*mallocProc)( (YYMALLOCARGTYPE)sizeof(yyParser) );
  if( yypParser ){
    ParseCTX_STORE
    ParseInit(yypParser ParseCTX_PARAM);
  }
  return (void*)yypParser;
}
#endif /* Parse_ENGINEALWAYSONSTACK */


/* The following function deletes the "minor type" or semantic value
** associated with a symbol.  The symbol can be either a terminal
** or nonterminal. "yymajor" is the symbol code, and "yypminor" is
** a pointer to the value to be deleted.  The code used to do the 
** deletions is derived from the %destructor and/or %token_destructor
** directives of the input grammar.
*/
static void yy_destructor(
  yyParser *yypParser,    /* The parser */
  YYCODETYPE yymajor,     /* Type code for object to destroy */
  YYMINORTYPE *yypminor   /* The object to be destroyed */
){
  ParseARG_FETCH
  ParseCTX_FETCH
  switch( yymajor ){
    /* Here is inserted the actions which take place when a
    ** terminal or non-terminal is destroyed.  This can happen
    ** when the symbol is popped from the stack during a
    ** reduce or during error processing or when a parser is 
    ** being destroyed before it is finished parsing.
    **
    ** Note: during a reduce, the only symbols destroyed are those
    ** which appear on the RHS of the rule, but which are *not* used
    ** inside the C code.
    */
/********* Begin destructor definitions ***************************************/
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
 delete (yypminor->yy0); 
}
      break;
/********* End destructor definitions *****************************************/
    default:  break;   /* If no destructor action specified: do nothing */
  }
}

/*
** Pop the parser's stack once.
**
** If there is a destructor routine associated with the token which
** is popped from the stack, then call it.
*/
static void yy_pop_parser_stack(yyParser *pParser){
  yyStackEntry *yytos;
  assert( pParser->yytos!=0 );
  assert( pParser->yytos > pParser->yystack );
  yytos = pParser->yytos--;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sPopping %s\n",
      yyTracePrompt,
      yyTokenName[yytos->major]);
  }
#endif
  yy_destructor(pParser, yytos->major, &yytos->minor);
}

/*
** Clear all secondary memory allocations from the parser
*/
void ParseFinalize(void *p){
  yyParser *pParser = (yyParser*)p;
  while( pParser->yytos>pParser->yystack ) yy_pop_parser_stack(pParser);
#if YYSTACKDEPTH<=0
  if( pParser->yystack!=&pParser->yystk0 ) free(pParser->yystack);
#endif
}

#ifndef Parse_ENGINEALWAYSONSTACK
/* 
** Deallocate and destroy a parser.  Destructors are called for
** all stack elements before shutting the parser down.
**
** If the YYPARSEFREENEVERNULL macro exists (for example because it
** is defined in a %include section of the input grammar) then it is
** assumed that the input pointer is never NULL.
*/
void ParseFree(
  void *p,                    /* The parser to be deleted */
  void (*freeProc)(void*)     /* Function used to reclaim memory */
){
#ifndef YYPARSEFREENEVERNULL
  if( p==0 ) return;
#endif
  ParseFinalize(p);
  (*freeProc)(p);
}
#endif /* Parse_ENGINEALWAYSONSTACK */

/*
** Return the peak depth of the stack for a parser.
*/
#ifdef YYTRACKMAXSTACKDEPTH
int ParseStackPeak(void *p){
  yyParser *pParser = (yyParser*)p;
  return pParser->yyhwm;
}
#endif

/* This array of booleans keeps track of the parser statement
** coverage.  The element yycoverage[X][Y] is set when the parser
** is in state X and has a lookahead token Y.  In a well-tested
** systems, every element of this matrix should end up being set.
*/
#if defined(YYCOVERAGE)
static unsigned char yycoverage[YYNSTATE][YYNTOKEN];
#endif

/*
** Write into out a description of every state/lookahead combination that
**
**   (1)  has not been used by the parser, and
**   (2)  is not a syntax error.
**
** Return the number of missed state/lookahead combinations.
*/
#if defined(YYCOVERAGE)
int ParseCoverage(FILE *out){
  int stateno, iLookAhead, i;
  int nMissed = 0;
  for(stateno=0; stateno<YYNSTATE; stateno++){
    i = yy_shift_ofst[stateno];
    for(iLookAhead=0; iLookAhead<YYNTOKEN; iLookAhead++){
      if( yy_lookahead[i+iLookAhead]!=iLookAhead ) continue;
      if( yycoverage[stateno][iLookAhead]==0 ) nMissed++;
      if( out ){
        fprintf(out,"State %d lookahead %s %s\n", stateno,
                yyTokenName[iLookAhead],
                yycoverage[stateno][iLookAhead] ? "ok" : "missed");
      }
    }
  }
  return nMissed;
}
#endif

/*
** Find the appropriate action for a parser given the terminal
** look-ahead token iLookAhead.
*/
static YYACTIONTYPE yy_find_shift_action(
  YYCODETYPE iLookAhead,    /* The look-ahead token */
  YYACTIONTYPE stateno      /* Current state number */
){
  int i;

  if( stateno>YY_MAX_SHIFT ) return stateno;
  assert( stateno <= YY_SHIFT_COUNT );
#if defined(YYCOVERAGE)
  yycoverage[stateno][iLookAhead] = 1;
#endif
  do{
    i = yy_shift_ofst[stateno];
    assert( i>=0 );
    assert( i+YYNTOKEN<=(int)sizeof(yy_lookahead)/sizeof(yy_lookahead[0]) );
    assert( iLookAhead!=YYNOCODE );
    assert( iLookAhead < YYNTOKEN );
    i += iLookAhead;
    if( yy_lookahead[i]!=iLookAhead ){
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
        assert( yyFallback[iFallback]==0 ); /* Fallback loop must terminate */
        iLookAhead = iFallback;
        continue;
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
          yy_lookahead[j]==YYWILDCARD && iLookAhead>0
        ){
#ifndef NDEBUG
          if( yyTraceFILE ){
            fprintf(yyTraceFILE, "%sWILDCARD %s => %s\n",
               yyTracePrompt, yyTokenName[iLookAhead],
               yyTokenName[YYWILDCARD]);
          }
#endif /* NDEBUG */
          return yy_action[j];
        }
      }
#endif /* YYWILDCARD */
      return yy_default[stateno];
    }else{
      return yy_action[i];
    }
  }while(1);
}

/*
** Find the appropriate action for a parser given the non-terminal
** look-ahead token iLookAhead.
*/
static int yy_find_reduce_action(
  YYACTIONTYPE stateno,     /* Current state number */
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
static void yyStackOverflow(yyParser *yypParser){
   ParseARG_FETCH
   ParseCTX_FETCH
#ifndef NDEBUG
   if( yyTraceFILE ){
     fprintf(yyTraceFILE,"%sStack Overflow!\n",yyTracePrompt);
   }
#endif
   while( yypParser->yytos>yypParser->yystack ) yy_pop_parser_stack(yypParser);
   /* Here code is inserted which will execute if the parser
   ** stack every overflows */
/******** Begin %stack_overflow code ******************************************/
/******** End %stack_overflow code ********************************************/
   ParseARG_STORE /* Suppress warning about unused %extra_argument var */
   ParseCTX_STORE
}

/*
** Print tracing information for a SHIFT action
*/
#ifndef NDEBUG
static void yyTraceShift(yyParser *yypParser, int yyNewState, const char *zTag){
  if( yyTraceFILE ){
    if( yyNewState<YYNSTATE ){
      fprintf(yyTraceFILE,"%s%s '%s', go to state %d\n",
         yyTracePrompt, zTag, yyTokenName[yypParser->yytos->major],
         yyNewState);
    }else{
      fprintf(yyTraceFILE,"%s%s '%s', pending reduce %d\n",
         yyTracePrompt, zTag, yyTokenName[yypParser->yytos->major],
         yyNewState - YY_MIN_REDUCE);
    }
  }
}
#else
# define yyTraceShift(X,Y,Z)
#endif

/*
** Perform a shift action.
*/
static void yy_shift(
  yyParser *yypParser,          /* The parser to be shifted */
  YYACTIONTYPE yyNewState,      /* The new state to shift in */
  YYCODETYPE yyMajor,           /* The major token to shift in */
  ParseTOKENTYPE yyMinor        /* The minor token to shift in */
){
  yyStackEntry *yytos;
  yypParser->yytos++;
#ifdef YYTRACKMAXSTACKDEPTH
  if( (int)(yypParser->yytos - yypParser->yystack)>yypParser->yyhwm ){
    yypParser->yyhwm++;
    assert( yypParser->yyhwm == (int)(yypParser->yytos - yypParser->yystack) );
  }
#endif
#if YYSTACKDEPTH>0 
  if( yypParser->yytos>yypParser->yystackEnd ){
    yypParser->yytos--;
    yyStackOverflow(yypParser);
    return;
  }
#else
  if( yypParser->yytos>=&yypParser->yystack[yypParser->yystksz] ){
    if( yyGrowStack(yypParser) ){
      yypParser->yytos--;
      yyStackOverflow(yypParser);
      return;
    }
  }
#endif
  if( yyNewState > YY_MAX_SHIFT ){
    yyNewState += YY_MIN_REDUCE - YY_MIN_SHIFTREDUCE;
  }
  yytos = yypParser->yytos;
  yytos->stateno = yyNewState;
  yytos->major = yyMajor;
  yytos->minor.yy0 = yyMinor;
  yyTraceShift(yypParser, yyNewState, "Shift");
}

/* The following table contains information about every rule that
** is used during the reduce.
*/
static const struct {
  YYCODETYPE lhs;       /* Symbol on the left-hand side of the rule */
  signed char nrhs;     /* Negative of the number of RHS symbols in the rule */
} yyRuleInfo[] = {
  {   55,   -1 }, /* (0) chunk ::= block */
  {   56,   -2 }, /* (1) block ::= scope statlist */
  {   56,   -4 }, /* (2) block ::= scope statlist laststat semi */
  {   61,   -3 }, /* (3) ublock ::= block UNTIL exp */
  {   58,    0 }, /* (4) scope ::= */
  {   58,   -4 }, /* (5) scope ::= scope statlist binding semi */
  {   59,    0 }, /* (6) statlist ::= */
  {   59,   -3 }, /* (7) statlist ::= statlist stat semi */
  {   64,   -3 }, /* (8) stat ::= DO block END */
  {   64,   -5 }, /* (9) stat ::= WHILE exp DO block END */
  {   64,   -4 }, /* (10) stat ::= repetition DO block END */
  {   64,   -2 }, /* (11) stat ::= REPEAT ublock */
  {   64,   -3 }, /* (12) stat ::= IF conds END */
  {   64,   -3 }, /* (13) stat ::= FUNCTION funcname funcbody */
  {   64,   -3 }, /* (14) stat ::= setlist EQUAL explist1 */
  {   64,   -1 }, /* (15) stat ::= functioncall */
  {   66,   -1 }, /* (16) conds ::= condlist */
  {   66,   -3 }, /* (17) conds ::= condlist ELSE block */
  {   74,   -1 }, /* (18) condlist ::= cond */
  {   74,   -3 }, /* (19) condlist ::= condlist ELSEIF cond */
  {   75,   -3 }, /* (20) cond ::= exp THEN block */
  {   60,   -1 }, /* (21) laststat ::= BREAK */
  {   60,   -1 }, /* (22) laststat ::= RETURN */
  {   60,   -2 }, /* (23) laststat ::= RETURN explist1 */
  {   63,   -2 }, /* (24) binding ::= LOCAL namelist */
  {   63,   -4 }, /* (25) binding ::= LOCAL namelist EQUAL explist1 */
  {   63,   -7 }, /* (26) binding ::= LOCAL namelist LESS CONST GREATER EQUAL explist1 */
  {   63,   -4 }, /* (27) binding ::= LOCAL FUNCTION NAME funcbody */
  {   67,   -1 }, /* (28) funcname ::= dottedname */
  {   67,   -3 }, /* (29) funcname ::= dottedname COLON NAME */
  {   76,   -1 }, /* (30) dottedname ::= NAME */
  {   76,   -3 }, /* (31) dottedname ::= dottedname DOT NAME */
  {   73,   -1 }, /* (32) namelist ::= NAME */
  {   73,   -3 }, /* (33) namelist ::= namelist COMMA NAME */
  {   70,   -1 }, /* (34) explist1 ::= exp */
  {   70,   -3 }, /* (35) explist1 ::= explist1 COMMA exp */
  {   72,   -3 }, /* (36) explist23 ::= exp COMMA exp */
  {   72,   -5 }, /* (37) explist23 ::= exp COMMA exp COMMA exp */
  {   62,   -1 }, /* (38) exp ::= NIL|TRUE|FALSE|NUMBER|STRING|DOT_DOT_DOT */
  {   62,   -1 }, /* (39) exp ::= function */
  {   62,   -1 }, /* (40) exp ::= prefixexp */
  {   62,   -1 }, /* (41) exp ::= tableconstructor */
  {   62,   -2 }, /* (42) exp ::= NOT|HASH|MINUS exp */
  {   62,   -3 }, /* (43) exp ::= exp OR exp */
  {   62,   -3 }, /* (44) exp ::= exp AND exp */
  {   62,   -3 }, /* (45) exp ::= exp LESS|LESS_EQUAL|GREATER|GREATER_EQUAL|EQUAL_EQUAL|NOT_EQUAL exp */
  {   62,   -3 }, /* (46) exp ::= exp DOT_DOT exp */
  {   62,   -3 }, /* (47) exp ::= exp BITWISE_OR exp */
  {   62,   -3 }, /* (48) exp ::= exp BITWISE_AND exp */
  {   62,   -3 }, /* (49) exp ::= exp PLUS|MINUS exp */
  {   62,   -3 }, /* (50) exp ::= exp MULTIPLY|DIVIDE|MODULO exp */
  {   62,   -3 }, /* (51) exp ::= exp RAISE exp */
  {   80,   -1 }, /* (52) var ::= NAME */
  {   80,   -4 }, /* (53) var ::= prefixexp LEFT_BRACKET exp RIGHT_BRACKET */
  {   80,   -3 }, /* (54) var ::= prefixexp DOT NAME */
  {   78,   -1 }, /* (55) prefixexp ::= var */
  {   78,   -1 }, /* (56) prefixexp ::= functioncall */
  {   78,   -3 }, /* (57) prefixexp ::= OPEN exp RIGHT_PARANTHESIS */
  {   71,   -2 }, /* (58) functioncall ::= prefixexp args */
  {   71,   -4 }, /* (59) functioncall ::= prefixexp COLON NAME args */
  {   81,   -2 }, /* (60) args ::= LEFT_PARANTHESIS RIGHT_PARANTHESIS */
  {   81,   -3 }, /* (61) args ::= LEFT_PARANTHESIS explist1 RIGHT_PARANTHESIS */
  {   81,   -1 }, /* (62) args ::= tableconstructor */
  {   81,   -1 }, /* (63) args ::= STRING */
  {   77,   -2 }, /* (64) function ::= FUNCTION funcbody */
  {   68,   -3 }, /* (65) funcbody ::= params block END */
  {   57,   -1 }, /* (66) semi ::= SEMICOLON */
  {   57,    0 }, /* (67) semi ::= */
  {   65,   -4 }, /* (68) repetition ::= FOR NAME EQUAL explist23 */
  {   65,   -4 }, /* (69) repetition ::= FOR namelist IN explist1 */
  {   69,   -1 }, /* (70) setlist ::= var */
  {   69,   -3 }, /* (71) setlist ::= setlist COMMA var */
  {   82,   -3 }, /* (72) params ::= LEFT_PARANTHESIS parlist RIGHT_PARANTHESIS */
  {   83,    0 }, /* (73) parlist ::= */
  {   83,   -1 }, /* (74) parlist ::= namelist */
  {   83,   -1 }, /* (75) parlist ::= DOT_DOT_DOT */
  {   83,   -3 }, /* (76) parlist ::= namelist COMMA DOT_DOT_DOT */
  {   79,   -2 }, /* (77) tableconstructor ::= LEFT_BRACE RIGHT_BRACE */
  {   79,   -3 }, /* (78) tableconstructor ::= LEFT_BRACE fieldlist RIGHT_BRACE */
  {   79,   -4 }, /* (79) tableconstructor ::= LEFT_BRACE fieldlist COMMA|SEMICOLON RIGHT_BRACE */
  {   84,   -1 }, /* (80) fieldlist ::= field */
  {   84,   -3 }, /* (81) fieldlist ::= fieldlist COMMA|SEMICOLON field */
  {   85,   -1 }, /* (82) field ::= exp */
  {   85,   -3 }, /* (83) field ::= NAME EQUAL exp */
  {   85,   -5 }, /* (84) field ::= LEFT_BRACKET exp RIGHT_BRACKET EQUAL exp */
};

static void yy_accept(yyParser*);  /* Forward Declaration */

/*
** Perform a reduce action and the shift that must immediately
** follow the reduce.
**
** The yyLookahead and yyLookaheadToken parameters provide reduce actions
** access to the lookahead token (if any).  The yyLookahead will be YYNOCODE
** if the lookahead token has already been consumed.  As this procedure is
** only called from one place, optimizing compilers will in-line it, which
** means that the extra parameters have no performance impact.
*/
static YYACTIONTYPE yy_reduce(
  yyParser *yypParser,         /* The parser */
  unsigned int yyruleno,       /* Number of the rule by which to reduce */
  int yyLookahead,             /* Lookahead token, or YYNOCODE if none */
  ParseTOKENTYPE yyLookaheadToken  /* Value of the lookahead token */
  ParseCTX_PDECL                   /* %extra_context */
){
  int yygoto;                     /* The next state */
  int yyact;                      /* The next action */
  yyStackEntry *yymsp;            /* The top of the parser's stack */
  int yysize;                     /* Amount to pop the stack */
  ParseARG_FETCH
  (void)yyLookahead;
  (void)yyLookaheadToken;
  yymsp = yypParser->yytos;
#ifndef NDEBUG
  if( yyTraceFILE && yyruleno<(int)(sizeof(yyRuleName)/sizeof(yyRuleName[0])) ){
    yysize = yyRuleInfo[yyruleno].nrhs;
    if( yysize ){
      fprintf(yyTraceFILE, "%sReduce %d [%s], go to state %d.\n",
        yyTracePrompt,
        yyruleno, yyRuleName[yyruleno], yymsp[yysize].stateno);
    }else{
      fprintf(yyTraceFILE, "%sReduce %d [%s].\n",
        yyTracePrompt, yyruleno, yyRuleName[yyruleno]);
    }
  }
#endif /* NDEBUG */

  /* Check that the stack is large enough to grow by a single entry
  ** if the RHS of the rule is empty.  This ensures that there is room
  ** enough on the stack to push the LHS value */
  if( yyRuleInfo[yyruleno].nrhs==0 ){
#ifdef YYTRACKMAXSTACKDEPTH
    if( (int)(yypParser->yytos - yypParser->yystack)>yypParser->yyhwm ){
      yypParser->yyhwm++;
      assert( yypParser->yyhwm == (int)(yypParser->yytos - yypParser->yystack));
    }
#endif
#if YYSTACKDEPTH>0 
    if( yypParser->yytos>=yypParser->yystackEnd ){
      yyStackOverflow(yypParser);
      /* The call to yyStackOverflow() above pops the stack until it is
      ** empty, causing the main parser loop to exit.  So the return value
      ** is never used and does not matter. */
      return 0;
    }
#else
    if( yypParser->yytos>=&yypParser->yystack[yypParser->yystksz-1] ){
      if( yyGrowStack(yypParser) ){
        yyStackOverflow(yypParser);
        /* The call to yyStackOverflow() above pops the stack until it is
        ** empty, causing the main parser loop to exit.  So the return value
        ** is never used and does not matter. */
        return 0;
      }
      yymsp = yypParser->yytos;
    }
#endif
  }

  switch( yyruleno ){
  /* Beginning here are the reduction cases.  A typical example
  ** follows:
  **   case 0:
  **  #line <lineno> <grammarfile>
  **     { ... }           // User supplied code
  **  #line <lineno> <thisfile>
  **     break;
  */
/********** Begin reduce actions **********************************************/
        YYMINORTYPE yylhsminor;
      case 0: /* chunk ::= block */
{ outline->parseResult(yymsp[0].minor.yy0->node); }
        break;
      case 1: /* block ::= scope statlist */
{ yylhsminor.yy0 = merge(yymsp[-1].minor.yy0, yymsp[0].minor.yy0); }
  yymsp[-1].minor.yy0 = yylhsminor.yy0;
        break;
      case 2: /* block ::= scope statlist laststat semi */
      case 5: /* scope ::= scope statlist binding semi */ yytestcase(yyruleno==5);
{ yylhsminor.yy0 = merge(merge(yymsp[-3].minor.yy0, yymsp[-2].minor.yy0), yymsp[-1].minor.yy0); }
  yymsp[-3].minor.yy0 = yylhsminor.yy0;
        break;
      case 3: /* ublock ::= block UNTIL exp */
{ yylhsminor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[0].minor.yy0); }
  yy_destructor(yypParser,4,&yymsp[-1].minor);
  yymsp[-2].minor.yy0 = yylhsminor.yy0;
        break;
      case 4: /* scope ::= */
      case 6: /* statlist ::= */ yytestcase(yyruleno==6);
{ yymsp[1].minor.yy0 = null(); }
        break;
      case 7: /* statlist ::= statlist stat semi */
{ yylhsminor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[-1].minor.yy0); }
  yymsp[-2].minor.yy0 = yylhsminor.yy0;
        break;
      case 8: /* stat ::= DO block END */
{  yy_destructor(yypParser,5,&yymsp[-2].minor);
{ yymsp[-2].minor.yy0 = copy(yymsp[-1].minor.yy0); }
  yy_destructor(yypParser,6,&yymsp[0].minor);
}
        break;
      case 9: /* stat ::= WHILE exp DO block END */
{  yy_destructor(yypParser,7,&yymsp[-4].minor);
{ yymsp[-4].minor.yy0 = copy(yymsp[-1].minor.yy0); }
  yy_destructor(yypParser,5,&yymsp[-2].minor);
  yy_destructor(yypParser,6,&yymsp[0].minor);
}
        break;
      case 10: /* stat ::= repetition DO block END */
{ yymsp[-3].minor.yy0 = copy(yymsp[-1].minor.yy0); }
  yy_destructor(yypParser,5,&yymsp[-2].minor);
  yy_destructor(yypParser,6,&yymsp[0].minor);
        break;
      case 11: /* stat ::= REPEAT ublock */
{  yy_destructor(yypParser,8,&yymsp[-1].minor);
{ yymsp[-1].minor.yy0 = copy(yymsp[0].minor.yy0); }
}
        break;
      case 12: /* stat ::= IF conds END */
{  yy_destructor(yypParser,9,&yymsp[-2].minor);
{ yymsp[-2].minor.yy0 = copy(yymsp[-1].minor.yy0); }
  yy_destructor(yypParser,6,&yymsp[0].minor);
}
        break;
      case 13: /* stat ::= FUNCTION funcname funcbody */
{  yy_destructor(yypParser,10,&yymsp[-2].minor);
{ yymsp[-2].minor.yy0 = new LuaGrammarToken(new IScriptOutline::FunctionNode(yymsp[-1].minor.yy0->line, yymsp[-1].minor.yy0->text, false, yymsp[0].minor.yy0->node), yymsp[-1].minor.yy0->line); }
}
        break;
      case 14: /* stat ::= setlist EQUAL explist1 */
{ yymsp[-2].minor.yy0 = null(); }
  yy_destructor(yypParser,11,&yymsp[-1].minor);
        break;
      case 15: /* stat ::= functioncall */
      case 16: /* conds ::= condlist */ yytestcase(yyruleno==16);
      case 18: /* condlist ::= cond */ yytestcase(yyruleno==18);
      case 28: /* funcname ::= dottedname */ yytestcase(yyruleno==28);
      case 30: /* dottedname ::= NAME */ yytestcase(yyruleno==30);
      case 32: /* namelist ::= NAME */ yytestcase(yyruleno==32);
      case 34: /* explist1 ::= exp */ yytestcase(yyruleno==34);
      case 39: /* exp ::= function */ yytestcase(yyruleno==39);
      case 40: /* exp ::= prefixexp */ yytestcase(yyruleno==40);
      case 52: /* var ::= NAME */ yytestcase(yyruleno==52);
      case 55: /* prefixexp ::= var */ yytestcase(yyruleno==55);
      case 56: /* prefixexp ::= functioncall */ yytestcase(yyruleno==56);
{ yylhsminor.yy0 = copy(yymsp[0].minor.yy0); }
  yymsp[0].minor.yy0 = yylhsminor.yy0;
        break;
      case 17: /* conds ::= condlist ELSE block */
{ yylhsminor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[0].minor.yy0); }
  yy_destructor(yypParser,15,&yymsp[-1].minor);
  yymsp[-2].minor.yy0 = yylhsminor.yy0;
        break;
      case 19: /* condlist ::= condlist ELSEIF cond */
{ yylhsminor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[0].minor.yy0); }
  yy_destructor(yypParser,16,&yymsp[-1].minor);
  yymsp[-2].minor.yy0 = yylhsminor.yy0;
        break;
      case 20: /* cond ::= exp THEN block */
{ yylhsminor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[0].minor.yy0); }
  yy_destructor(yypParser,17,&yymsp[-1].minor);
  yymsp[-2].minor.yy0 = yylhsminor.yy0;
        break;
      case 21: /* laststat ::= BREAK */
{  yy_destructor(yypParser,18,&yymsp[0].minor);
{ yymsp[0].minor.yy0 = null(); }
}
        break;
      case 22: /* laststat ::= RETURN */
{  yy_destructor(yypParser,19,&yymsp[0].minor);
{ yymsp[0].minor.yy0 = null(); }
}
        break;
      case 23: /* laststat ::= RETURN explist1 */
{  yy_destructor(yypParser,19,&yymsp[-1].minor);
{ yymsp[-1].minor.yy0 = copy(yymsp[0].minor.yy0); }
}
        break;
      case 24: /* binding ::= LOCAL namelist */
{  yy_destructor(yypParser,20,&yymsp[-1].minor);
{ yymsp[-1].minor.yy0 = null(); }
}
        break;
      case 25: /* binding ::= LOCAL namelist EQUAL explist1 */
{  yy_destructor(yypParser,20,&yymsp[-3].minor);
{ yymsp[-3].minor.yy0 = copy(yymsp[0].minor.yy0); }
  yy_destructor(yypParser,11,&yymsp[-1].minor);
}
        break;
      case 26: /* binding ::= LOCAL namelist LESS CONST GREATER EQUAL explist1 */
{  yy_destructor(yypParser,20,&yymsp[-6].minor);
{ yymsp[-6].minor.yy0 = copy(yymsp[0].minor.yy0); }
  yy_destructor(yypParser,21,&yymsp[-4].minor);
  yy_destructor(yypParser,22,&yymsp[-3].minor);
  yy_destructor(yypParser,23,&yymsp[-2].minor);
  yy_destructor(yypParser,11,&yymsp[-1].minor);
}
        break;
      case 27: /* binding ::= LOCAL FUNCTION NAME funcbody */
{  yy_destructor(yypParser,20,&yymsp[-3].minor);
{ yymsp[-3].minor.yy0 = new LuaGrammarToken(new IScriptOutline::FunctionNode(yymsp[-1].minor.yy0->line, yymsp[-1].minor.yy0->text, true, yymsp[0].minor.yy0->node), yymsp[-1].minor.yy0->line); }
  yy_destructor(yypParser,10,&yymsp[-2].minor);
}
        break;
      case 29: /* funcname ::= dottedname COLON NAME */
{ yylhsminor.yy0 = new LuaGrammarToken(yymsp[-2].minor.yy0->text + L":" + yymsp[0].minor.yy0->text, yymsp[0].minor.yy0->line); }
  yy_destructor(yypParser,24,&yymsp[-1].minor);
  yymsp[-2].minor.yy0 = yylhsminor.yy0;
        break;
      case 31: /* dottedname ::= dottedname DOT NAME */
{ yylhsminor.yy0 = new LuaGrammarToken(yymsp[-2].minor.yy0->text + L"." + yymsp[0].minor.yy0->text, yymsp[0].minor.yy0->line); }
  yy_destructor(yypParser,25,&yymsp[-1].minor);
  yymsp[-2].minor.yy0 = yylhsminor.yy0;
        break;
      case 33: /* namelist ::= namelist COMMA NAME */
{ yylhsminor.yy0 = new LuaGrammarToken(yymsp[-2].minor.yy0->text + L"," + yymsp[0].minor.yy0->text, yymsp[0].minor.yy0->line); }
  yy_destructor(yypParser,26,&yymsp[-1].minor);
  yymsp[-2].minor.yy0 = yylhsminor.yy0;
        break;
      case 35: /* explist1 ::= explist1 COMMA exp */
      case 36: /* explist23 ::= exp COMMA exp */ yytestcase(yyruleno==36);
{ yylhsminor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[0].minor.yy0); }
  yy_destructor(yypParser,26,&yymsp[-1].minor);
  yymsp[-2].minor.yy0 = yylhsminor.yy0;
        break;
      case 37: /* explist23 ::= exp COMMA exp COMMA exp */
{ yylhsminor.yy0 = merge(merge(yymsp[-4].minor.yy0, yymsp[-2].minor.yy0), yymsp[0].minor.yy0); }
  yy_destructor(yypParser,26,&yymsp[-3].minor);
  yy_destructor(yypParser,26,&yymsp[-1].minor);
  yymsp[-4].minor.yy0 = yylhsminor.yy0;
        break;
      case 38: /* exp ::= NIL|TRUE|FALSE|NUMBER|STRING|DOT_DOT_DOT */
      case 41: /* exp ::= tableconstructor */ yytestcase(yyruleno==41);
      case 62: /* args ::= tableconstructor */ yytestcase(yyruleno==62);
{ yymsp[0].minor.yy0 = null(); }
        break;
      case 42: /* exp ::= NOT|HASH|MINUS exp */
{ yymsp[-1].minor.yy0 = copy(yymsp[0].minor.yy0); }
        break;
      case 43: /* exp ::= exp OR exp */
{ yylhsminor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[0].minor.yy0); }
  yy_destructor(yypParser,27,&yymsp[-1].minor);
  yymsp[-2].minor.yy0 = yylhsminor.yy0;
        break;
      case 44: /* exp ::= exp AND exp */
{ yylhsminor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[0].minor.yy0); }
  yy_destructor(yypParser,28,&yymsp[-1].minor);
  yymsp[-2].minor.yy0 = yylhsminor.yy0;
        break;
      case 45: /* exp ::= exp LESS|LESS_EQUAL|GREATER|GREATER_EQUAL|EQUAL_EQUAL|NOT_EQUAL exp */
      case 49: /* exp ::= exp PLUS|MINUS exp */ yytestcase(yyruleno==49);
      case 50: /* exp ::= exp MULTIPLY|DIVIDE|MODULO exp */ yytestcase(yyruleno==50);
{ yylhsminor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[0].minor.yy0); }
  yymsp[-2].minor.yy0 = yylhsminor.yy0;
        break;
      case 46: /* exp ::= exp DOT_DOT exp */
{ yylhsminor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[0].minor.yy0); }
  yy_destructor(yypParser,33,&yymsp[-1].minor);
  yymsp[-2].minor.yy0 = yylhsminor.yy0;
        break;
      case 47: /* exp ::= exp BITWISE_OR exp */
{ yylhsminor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[0].minor.yy0); }
  yy_destructor(yypParser,48,&yymsp[-1].minor);
  yymsp[-2].minor.yy0 = yylhsminor.yy0;
        break;
      case 48: /* exp ::= exp BITWISE_AND exp */
{ yylhsminor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[0].minor.yy0); }
  yy_destructor(yypParser,49,&yymsp[-1].minor);
  yymsp[-2].minor.yy0 = yylhsminor.yy0;
        break;
      case 51: /* exp ::= exp RAISE exp */
{ yylhsminor.yy0 = merge(yymsp[-2].minor.yy0, yymsp[0].minor.yy0); }
  yy_destructor(yypParser,41,&yymsp[-1].minor);
  yymsp[-2].minor.yy0 = yylhsminor.yy0;
        break;
      case 53: /* var ::= prefixexp LEFT_BRACKET exp RIGHT_BRACKET */
{ yylhsminor.yy0 = merge(yymsp[-3].minor.yy0, yymsp[-1].minor.yy0); }
  yy_destructor(yypParser,50,&yymsp[-2].minor);
  yy_destructor(yypParser,51,&yymsp[0].minor);
  yymsp[-3].minor.yy0 = yylhsminor.yy0;
        break;
      case 54: /* var ::= prefixexp DOT NAME */
{ yylhsminor.yy0 = new LuaGrammarToken(as_string(yymsp[-2].minor.yy0) + L"." + yymsp[0].minor.yy0->text, yymsp[0].minor.yy0->line); }
  yy_destructor(yypParser,25,&yymsp[-1].minor);
  yymsp[-2].minor.yy0 = yylhsminor.yy0;
        break;
      case 57: /* prefixexp ::= OPEN exp RIGHT_PARANTHESIS */
{  yy_destructor(yypParser,1,&yymsp[-2].minor);
{ yymsp[-2].minor.yy0 = copy(yymsp[-1].minor.yy0); }
  yy_destructor(yypParser,52,&yymsp[0].minor);
}
        break;
      case 58: /* functioncall ::= prefixexp args */
{ yylhsminor.yy0 = merge(new LuaGrammarToken(new IScriptOutline::FunctionReferenceNode(yymsp[-1].minor.yy0->line, as_string(yymsp[-1].minor.yy0)), yymsp[-1].minor.yy0->line), yymsp[0].minor.yy0); }
  yymsp[-1].minor.yy0 = yylhsminor.yy0;
        break;
      case 59: /* functioncall ::= prefixexp COLON NAME args */
{ yylhsminor.yy0 = merge(new LuaGrammarToken(new IScriptOutline::FunctionReferenceNode(yymsp[-1].minor.yy0->line, as_string(yymsp[-3].minor.yy0) + L":" + yymsp[-1].minor.yy0->text), yymsp[-1].minor.yy0->line), yymsp[0].minor.yy0); }
  yy_destructor(yypParser,24,&yymsp[-2].minor);
  yymsp[-3].minor.yy0 = yylhsminor.yy0;
        break;
      case 60: /* args ::= LEFT_PARANTHESIS RIGHT_PARANTHESIS */
{  yy_destructor(yypParser,2,&yymsp[-1].minor);
{ yymsp[-1].minor.yy0 = null(); }
  yy_destructor(yypParser,52,&yymsp[0].minor);
}
        break;
      case 61: /* args ::= LEFT_PARANTHESIS explist1 RIGHT_PARANTHESIS */
{  yy_destructor(yypParser,2,&yymsp[-2].minor);
{ yymsp[-2].minor.yy0 = copy(yymsp[-1].minor.yy0); }
  yy_destructor(yypParser,52,&yymsp[0].minor);
}
        break;
      case 63: /* args ::= STRING */
{  yy_destructor(yypParser,46,&yymsp[0].minor);
{ yymsp[0].minor.yy0 = null(); }
}
        break;
      case 64: /* function ::= FUNCTION funcbody */
{  yy_destructor(yypParser,10,&yymsp[-1].minor);
{ yymsp[-1].minor.yy0 = copy(yymsp[0].minor.yy0); }
}
        break;
      case 65: /* funcbody ::= params block END */
{ yymsp[-2].minor.yy0 = copy(yymsp[-1].minor.yy0); }
  yy_destructor(yypParser,6,&yymsp[0].minor);
        break;
      case 66: /* semi ::= SEMICOLON */
{  yy_destructor(yypParser,3,&yymsp[0].minor);
{
}
}
        break;
      case 68: /* repetition ::= FOR NAME EQUAL explist23 */
{  yy_destructor(yypParser,12,&yymsp[-3].minor);
{
}
  yy_destructor(yypParser,13,&yymsp[-2].minor);
  yy_destructor(yypParser,11,&yymsp[-1].minor);
}
        break;
      case 69: /* repetition ::= FOR namelist IN explist1 */
{  yy_destructor(yypParser,12,&yymsp[-3].minor);
{
}
  yy_destructor(yypParser,14,&yymsp[-1].minor);
}
        break;
      case 71: /* setlist ::= setlist COMMA var */
{
}
  yy_destructor(yypParser,26,&yymsp[-1].minor);
        break;
      case 72: /* params ::= LEFT_PARANTHESIS parlist RIGHT_PARANTHESIS */
{  yy_destructor(yypParser,2,&yymsp[-2].minor);
{
}
  yy_destructor(yypParser,52,&yymsp[0].minor);
}
        break;
      case 75: /* parlist ::= DOT_DOT_DOT */
{  yy_destructor(yypParser,47,&yymsp[0].minor);
{
}
}
        break;
      case 76: /* parlist ::= namelist COMMA DOT_DOT_DOT */
{
}
  yy_destructor(yypParser,26,&yymsp[-1].minor);
  yy_destructor(yypParser,47,&yymsp[0].minor);
        break;
      case 77: /* tableconstructor ::= LEFT_BRACE RIGHT_BRACE */
{  yy_destructor(yypParser,53,&yymsp[-1].minor);
{
}
  yy_destructor(yypParser,54,&yymsp[0].minor);
}
        break;
      case 78: /* tableconstructor ::= LEFT_BRACE fieldlist RIGHT_BRACE */
{  yy_destructor(yypParser,53,&yymsp[-2].minor);
{
}
  yy_destructor(yypParser,54,&yymsp[0].minor);
}
        break;
      case 79: /* tableconstructor ::= LEFT_BRACE fieldlist COMMA|SEMICOLON RIGHT_BRACE */
{  yy_destructor(yypParser,53,&yymsp[-3].minor);
{
}
  yy_destructor(yypParser,54,&yymsp[0].minor);
}
        break;
      case 83: /* field ::= NAME EQUAL exp */
{  yy_destructor(yypParser,13,&yymsp[-2].minor);
{
}
  yy_destructor(yypParser,11,&yymsp[-1].minor);
}
        break;
      case 84: /* field ::= LEFT_BRACKET exp RIGHT_BRACKET EQUAL exp */
{  yy_destructor(yypParser,50,&yymsp[-4].minor);
{
}
  yy_destructor(yypParser,51,&yymsp[-2].minor);
  yy_destructor(yypParser,11,&yymsp[-1].minor);
}
        break;
      default:
      /* (67) semi ::= */ yytestcase(yyruleno==67);
      /* (70) setlist ::= var */ yytestcase(yyruleno==70);
      /* (73) parlist ::= */ yytestcase(yyruleno==73);
      /* (74) parlist ::= namelist */ yytestcase(yyruleno==74);
      /* (80) fieldlist ::= field (OPTIMIZED OUT) */ assert(yyruleno!=80);
      /* (81) fieldlist ::= fieldlist COMMA|SEMICOLON field */ yytestcase(yyruleno==81);
      /* (82) field ::= exp */ yytestcase(yyruleno==82);
        break;
/********** End reduce actions ************************************************/
  };
  assert( yyruleno<sizeof(yyRuleInfo)/sizeof(yyRuleInfo[0]) );
  yygoto = yyRuleInfo[yyruleno].lhs;
  yysize = yyRuleInfo[yyruleno].nrhs;
  yyact = yy_find_reduce_action(yymsp[yysize].stateno,(YYCODETYPE)yygoto);

  /* There are no SHIFTREDUCE actions on nonterminals because the table
  ** generator has simplified them to pure REDUCE actions. */
  assert( !(yyact>YY_MAX_SHIFT && yyact<=YY_MAX_SHIFTREDUCE) );

  /* It is not possible for a REDUCE to be followed by an error */
  assert( yyact!=YY_ERROR_ACTION );

  yymsp += yysize+1;
  yypParser->yytos = yymsp;
  yymsp->stateno = (YYACTIONTYPE)yyact;
  yymsp->major = (YYCODETYPE)yygoto;
  yyTraceShift(yypParser, yyact, "... then shift");
  return yyact;
}

/*
** The following code executes when the parse fails
*/
#ifndef YYNOERRORRECOVERY
static void yy_parse_failed(
  yyParser *yypParser           /* The parser */
){
  ParseARG_FETCH
  ParseCTX_FETCH
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sFail!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yytos>yypParser->yystack ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser fails */
/************ Begin %parse_failure code ***************************************/
/************ End %parse_failure code *****************************************/
  ParseARG_STORE /* Suppress warning about unused %extra_argument variable */
  ParseCTX_STORE
}
#endif /* YYNOERRORRECOVERY */

/*
** The following code executes when a syntax error first occurs.
*/
static void yy_syntax_error(
  yyParser *yypParser,           /* The parser */
  int yymajor,                   /* The major type of the error token */
  ParseTOKENTYPE yyminor         /* The minor type of the error token */
){
  ParseARG_FETCH
  ParseCTX_FETCH
#define TOKEN yyminor
/************ Begin %syntax_error code ****************************************/


outline->syntaxError();

/************ End %syntax_error code ******************************************/
  ParseARG_STORE /* Suppress warning about unused %extra_argument variable */
  ParseCTX_STORE
}

/*
** The following is executed when the parser accepts
*/
static void yy_accept(
  yyParser *yypParser           /* The parser */
){
  ParseARG_FETCH
  ParseCTX_FETCH
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sAccept!\n",yyTracePrompt);
  }
#endif
#ifndef YYNOERRORRECOVERY
  yypParser->yyerrcnt = -1;
#endif
  assert( yypParser->yytos==yypParser->yystack );
  /* Here code is inserted which will be executed whenever the
  ** parser accepts */
/*********** Begin %parse_accept code *****************************************/
/*********** End %parse_accept code *******************************************/
  ParseARG_STORE /* Suppress warning about unused %extra_argument variable */
  ParseCTX_STORE
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
  YYACTIONTYPE yyact;   /* The parser action. */
#if !defined(YYERRORSYMBOL) && !defined(YYNOERRORRECOVERY)
  int yyendofinput;     /* True if we are at the end of input */
#endif
#ifdef YYERRORSYMBOL
  int yyerrorhit = 0;   /* True if yymajor has invoked an error */
#endif
  yyParser *yypParser = (yyParser*)yyp;  /* The parser */
  ParseCTX_FETCH
  ParseARG_STORE

  assert( yypParser->yytos!=0 );
#if !defined(YYERRORSYMBOL) && !defined(YYNOERRORRECOVERY)
  yyendofinput = (yymajor==0);
#endif

  yyact = yypParser->yytos->stateno;
#ifndef NDEBUG
  if( yyTraceFILE ){
    if( yyact < YY_MIN_REDUCE ){
      fprintf(yyTraceFILE,"%sInput '%s' in state %d\n",
              yyTracePrompt,yyTokenName[yymajor],yyact);
    }else{
      fprintf(yyTraceFILE,"%sInput '%s' with pending reduce %d\n",
              yyTracePrompt,yyTokenName[yymajor],yyact-YY_MIN_REDUCE);
    }
  }
#endif

  do{
    assert( yyact==yypParser->yytos->stateno );
    yyact = yy_find_shift_action(yymajor,yyact);
    if( yyact >= YY_MIN_REDUCE ){
      yyact = yy_reduce(yypParser,yyact-YY_MIN_REDUCE,yymajor,
                        yyminor ParseCTX_PARAM);
    }else if( yyact <= YY_MAX_SHIFTREDUCE ){
      yy_shift(yypParser,yyact,yymajor,yyminor);
#ifndef YYNOERRORRECOVERY
      yypParser->yyerrcnt--;
#endif
      break;
    }else if( yyact==YY_ACCEPT_ACTION ){
      yypParser->yytos--;
      yy_accept(yypParser);
      return;
    }else{
      assert( yyact == YY_ERROR_ACTION );
      yyminorunion.yy0 = yyminor;
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
        yy_syntax_error(yypParser,yymajor,yyminor);
      }
      yymx = yypParser->yytos->major;
      if( yymx==YYERRORSYMBOL || yyerrorhit ){
#ifndef NDEBUG
        if( yyTraceFILE ){
          fprintf(yyTraceFILE,"%sDiscard input token %s\n",
             yyTracePrompt,yyTokenName[yymajor]);
        }
#endif
        yy_destructor(yypParser, (YYCODETYPE)yymajor, &yyminorunion);
        yymajor = YYNOCODE;
      }else{
        while( yypParser->yytos >= yypParser->yystack
            && yymx != YYERRORSYMBOL
            && (yyact = yy_find_reduce_action(
                        yypParser->yytos->stateno,
                        YYERRORSYMBOL)) >= YY_MIN_REDUCE
        ){
          yy_pop_parser_stack(yypParser);
        }
        if( yypParser->yytos < yypParser->yystack || yymajor==0 ){
          yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
          yy_parse_failed(yypParser);
#ifndef YYNOERRORRECOVERY
          yypParser->yyerrcnt = -1;
#endif
          yymajor = YYNOCODE;
        }else if( yymx!=YYERRORSYMBOL ){
          yy_shift(yypParser,yyact,YYERRORSYMBOL,yyminor);
        }
      }
      yypParser->yyerrcnt = 3;
      yyerrorhit = 1;
      if( yymajor==YYNOCODE ) break;
      yyact = yypParser->yytos->stateno;
#elif defined(YYNOERRORRECOVERY)
      /* If the YYNOERRORRECOVERY macro is defined, then do not attempt to
      ** do any kind of error recovery.  Instead, simply invoke the syntax
      ** error routine and continue going as if nothing had happened.
      **
      ** Applications can set this macro (for example inside %include) if
      ** they intend to abandon the parse upon the first syntax error seen.
      */
      yy_syntax_error(yypParser,yymajor, yyminor);
      yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
      break;
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
        yy_syntax_error(yypParser,yymajor, yyminor);
      }
      yypParser->yyerrcnt = 3;
      yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
      if( yyendofinput ){
        yy_parse_failed(yypParser);
#ifndef YYNOERRORRECOVERY
        yypParser->yyerrcnt = -1;
#endif
      }
      break;
#endif
    }
  }while( yypParser->yytos>yypParser->yystack );
#ifndef NDEBUG
  if( yyTraceFILE ){
    yyStackEntry *i;
    char cDiv = '[';
    fprintf(yyTraceFILE,"%sReturn. Stack=",yyTracePrompt);
    for(i=&yypParser->yystack[1]; i<=yypParser->yytos; i++){
      fprintf(yyTraceFILE,"%c%s", cDiv, yyTokenName[i->major]);
      cDiv = ' ';
    }
    fprintf(yyTraceFILE,"]\n");
  }
#endif
  return;
}
