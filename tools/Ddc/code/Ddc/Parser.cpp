/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
/* Driver template for the LEMON parser generator.
** The author disclaims copyright to this source code.
*/
/* First off, code is included that follows the "include" declaration
** in the input grammar file. */
#include <stdio.h>
#line 9 ".\\code\\Ddc\\Parser.y"


#include <cassert>
#include <Core/Log/Log.h>
#include "Ddc/DfnAlias.h"
#include "Ddc/DfnClass.h"
#include "Ddc/DfnImport.h"
#include "Ddc/DfnMember.h"
#include "Ddc/DfnNamespace.h"
#include "Ddc/DfnType.h"
#include "Ddc/DfnTypeSubst.h"
#include "Ddc/ParseState.h"
#include "Ddc/Token.h"

using namespace traktor;
using namespace ddc;

#line 26 ".\\code\\Ddc\\Parser.c"
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
#define YYNOCODE 32
#define YYACTIONTYPE unsigned char
#define ParseTOKENTYPE  Token* 
typedef union {
  int yyinit;
  ParseTOKENTYPE yy0;
  Token* yy30;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 100
#endif
#define ParseARG_SDECL  ddc::ParseState* state ;
#define ParseARG_PDECL , ddc::ParseState* state 
#define ParseARG_FETCH  ddc::ParseState* state  = yypParser->state 
#define ParseARG_STORE yypParser->state  = state 
#define YYNSTATE 92
#define YYNRULE 33
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
static const YYACTIONTYPE yy_action[] = {
 /*     0 */    14,   26,   45,   26,   62,    7,   47,   44,   70,   25,
 /*    10 */    51,    2,   35,   40,   60,   61,   26,   89,    9,   90,
 /*    20 */     7,   47,   26,   17,   73,   57,    7,   47,   58,   26,
 /*    30 */    56,   16,   49,    7,   47,   54,    8,   26,   30,    5,
 /*    40 */    46,    7,   47,   66,   26,   38,    9,   43,    7,   47,
 /*    50 */    26,   15,   20,   32,    7,   47,  126,   65,    9,   50,
 /*    60 */    64,   12,    6,   84,   79,   53,   81,   22,   11,   20,
 /*    70 */    29,   26,   68,   82,   19,    1,   71,   55,   63,   92,
 /*    80 */    87,   78,   39,   67,   69,   88,   31,   59,   21,   86,
 /*    90 */    72,    3,   27,   18,   80,   41,   24,   33,   85,   23,
 /*   100 */    36,   10,   83,   34,   74,   75,   77,   91,    4,   48,
 /*   110 */    52,   13,   28,   37,  127,   42,  127,  127,   76,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */     1,   24,    3,   24,   27,   28,   29,   25,   29,   30,
 /*    10 */     8,    5,   13,   14,   15,   16,   24,   22,   23,   27,
 /*    20 */    28,   29,   24,   17,   12,   27,   28,   29,   24,   24,
 /*    30 */    18,    6,   27,   28,   29,   10,    9,   24,   11,    5,
 /*    40 */    27,   28,   29,   24,   24,   22,   23,   27,   28,   29,
 /*    50 */    24,   17,    8,   27,   28,   29,   21,   22,   23,    8,
 /*    60 */     8,    6,    5,   12,   12,   10,   25,   26,    9,    8,
 /*    70 */    11,   24,   12,   25,   17,    5,   29,   24,   18,    0,
 /*    80 */    24,   12,   24,   12,    7,    2,   14,   14,    8,    7,
 /*    90 */    12,    5,    8,   19,    7,    6,   11,    4,    7,   11,
 /*   100 */     8,    5,   12,    6,    2,    7,    7,    7,    5,    8,
 /*   110 */     4,    4,    8,    4,   31,    5,   31,   31,    7,
};
#define YY_SHIFT_USE_DFLT (-2)
#define YY_SHIFT_MAX 66
static const signed char yy_shift_ofst[] = {
 /*     0 */    -1,    2,    2,    2,    2,    2,    2,    2,    2,   -1,
 /*    10 */    -1,   44,    2,   61,    2,    2,   44,    2,    2,    2,
 /*    20 */    59,   57,   25,   12,   51,   55,   27,    6,   34,   52,
 /*    30 */    60,   84,   87,   89,   93,   92,   96,   97,   99,  103,
 /*    40 */   104,  106,  109,  111,  110,  107,  100,  101,  102,   98,
 /*    50 */    90,   74,   91,   88,   85,   86,   78,   82,   83,   80,
 /*    60 */    73,   72,   77,   71,   69,   79,   70,
};
#define YY_REDUCE_USE_DFLT (-24)
#define YY_REDUCE_MAX 19
static const signed char yy_reduce_ofst[] = {
 /*     0 */    35,  -23,   -2,   13,   26,   20,    5,   -8,  -21,   -5,
 /*    10 */    23,   41,   47,  -18,    4,   19,   48,   53,   56,   58,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   125,  125,  125,  125,  125,  125,  125,  113,  125,   94,
 /*    10 */   125,  125,  125,  125,  125,  125,  125,  125,  125,  125,
 /*    20 */    97,  125,  125,  125,  125,  125,  115,  125,  125,  125,
 /*    30 */   125,  125,  125,  125,  125,  125,  125,  125,  125,  125,
 /*    40 */   125,  125,  125,  125,  125,  125,  125,  125,  125,  125,
 /*    50 */   125,  124,  125,  116,   98,  125,  125,  125,  125,  125,
 /*    60 */   125,  125,  125,  125,  125,  125,  125,  118,  117,  109,
 /*    70 */   122,  121,  120,  119,  114,  107,  106,  105,  100,   99,
 /*    80 */   110,  104,  103,  102,  101,   96,  108,  123,   95,   93,
 /*    90 */   112,  111,
};
#define YY_SZ_ACTTAB (int)(sizeof(yy_action)/sizeof(yy_action[0]))

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
  "$",             "IMPORT",        "SEMI_COLON",    "ALIAS",       
  "STRING",        "OPEN_BRACE",    "COMMA",         "CLOSE_BRACE", 
  "LITERAL",       "LESS",          "GREATER",       "OPEN_BRACKET",
  "CLOSE_BRACKET",  "NAMESPACE",     "CLASS",         "PUBLIC",      
  "PRIVATE",       "COLON",         "NUMBER",        "DOT",         
  "error",         "program",       "statements",    "statement",   
  "qualified_literal",  "alias_type",    "alias_type_subst",  "members",     
  "member",        "member_type",   "member_type_subst",
};
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *const yyRuleName[] = {
 /*   0 */ "program ::= statements",
 /*   1 */ "statements ::= statement statements",
 /*   2 */ "statements ::= statement",
 /*   3 */ "statement ::= IMPORT qualified_literal SEMI_COLON",
 /*   4 */ "statement ::= ALIAS STRING alias_type OPEN_BRACE STRING COMMA STRING COMMA STRING CLOSE_BRACE",
 /*   5 */ "alias_type ::= LITERAL",
 /*   6 */ "alias_type ::= LITERAL LESS alias_type_subst GREATER",
 /*   7 */ "alias_type ::= LITERAL OPEN_BRACKET CLOSE_BRACKET",
 /*   8 */ "alias_type ::= LITERAL OPEN_BRACKET LITERAL CLOSE_BRACKET",
 /*   9 */ "alias_type ::= LITERAL LESS alias_type_subst GREATER OPEN_BRACKET CLOSE_BRACKET",
 /*  10 */ "alias_type ::= LITERAL LESS alias_type_subst GREATER OPEN_BRACKET LITERAL CLOSE_BRACKET",
 /*  11 */ "alias_type_subst ::= alias_type_subst COMMA alias_type",
 /*  12 */ "alias_type_subst ::= alias_type",
 /*  13 */ "statement ::= NAMESPACE LITERAL OPEN_BRACE statements CLOSE_BRACE",
 /*  14 */ "statement ::= CLASS LITERAL OPEN_BRACE members CLOSE_BRACE",
 /*  15 */ "statement ::= PUBLIC CLASS LITERAL OPEN_BRACE members CLOSE_BRACE",
 /*  16 */ "statement ::= PRIVATE CLASS LITERAL OPEN_BRACE members CLOSE_BRACE",
 /*  17 */ "statement ::= CLASS LITERAL COLON qualified_literal OPEN_BRACE members CLOSE_BRACE",
 /*  18 */ "statement ::= PUBLIC CLASS LITERAL COLON qualified_literal OPEN_BRACE members CLOSE_BRACE",
 /*  19 */ "statement ::= PRIVATE CLASS LITERAL COLON qualified_literal OPEN_BRACE members CLOSE_BRACE",
 /*  20 */ "members ::= member members",
 /*  21 */ "members ::= member",
 /*  22 */ "member ::= member_type LITERAL SEMI_COLON",
 /*  23 */ "member_type ::= qualified_literal",
 /*  24 */ "member_type ::= qualified_literal LESS member_type_subst GREATER",
 /*  25 */ "member_type ::= qualified_literal OPEN_BRACKET CLOSE_BRACKET",
 /*  26 */ "member_type ::= qualified_literal OPEN_BRACKET NUMBER CLOSE_BRACKET",
 /*  27 */ "member_type ::= qualified_literal LESS member_type_subst GREATER OPEN_BRACKET CLOSE_BRACKET",
 /*  28 */ "member_type ::= qualified_literal LESS member_type_subst GREATER OPEN_BRACKET NUMBER CLOSE_BRACKET",
 /*  29 */ "member_type_subst ::= member_type_subst COMMA member_type",
 /*  30 */ "member_type_subst ::= member_type",
 /*  31 */ "qualified_literal ::= LITERAL DOT qualified_literal",
 /*  32 */ "qualified_literal ::= LITERAL",
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
    case 1: /* IMPORT */
    case 2: /* SEMI_COLON */
    case 3: /* ALIAS */
    case 4: /* STRING */
    case 5: /* OPEN_BRACE */
    case 6: /* COMMA */
    case 7: /* CLOSE_BRACE */
    case 8: /* LITERAL */
    case 9: /* LESS */
    case 10: /* GREATER */
    case 11: /* OPEN_BRACKET */
    case 12: /* CLOSE_BRACKET */
    case 13: /* NAMESPACE */
    case 14: /* CLASS */
    case 15: /* PUBLIC */
    case 16: /* PRIVATE */
    case 17: /* COLON */
    case 18: /* NUMBER */
    case 19: /* DOT */
{
#line 5 ".\\code\\Ddc\\Parser.y"
 delete (yypminor->yy0); 
#line 460 ".\\code\\Ddc\\Parser.c"
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
 
  if( stateno>YY_SHIFT_MAX || (i = yy_shift_ofst[stateno])==YY_SHIFT_USE_DFLT ){
    return yy_default[stateno];
  }
  assert( iLookAhead!=YYNOCODE );
  i += iLookAhead;
  if( i<0 || i>=YY_SZ_ACTTAB || yy_lookahead[i]!=iLookAhead ){
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
        if( j>=0 && j<YY_SZ_ACTTAB && yy_lookahead[j]==YYWILDCARD ){
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
  if( stateno>YY_REDUCE_MAX ){
    return yy_default[stateno];
  }
#else
  assert( stateno<=YY_REDUCE_MAX );
#endif
  i = yy_reduce_ofst[stateno];
  assert( i!=YY_REDUCE_USE_DFLT );
  assert( iLookAhead!=YYNOCODE );
  i += iLookAhead;
#ifdef YYERRORSYMBOL
  if( i<0 || i>=YY_SZ_ACTTAB || yy_lookahead[i]!=iLookAhead ){
    return yy_default[stateno];
  }
#else
  assert( i>=0 && i<YY_SZ_ACTTAB );
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
  { 21, 1 },
  { 22, 2 },
  { 22, 1 },
  { 23, 3 },
  { 23, 10 },
  { 25, 1 },
  { 25, 4 },
  { 25, 3 },
  { 25, 4 },
  { 25, 6 },
  { 25, 7 },
  { 26, 3 },
  { 26, 1 },
  { 23, 5 },
  { 23, 5 },
  { 23, 6 },
  { 23, 6 },
  { 23, 7 },
  { 23, 8 },
  { 23, 8 },
  { 27, 2 },
  { 27, 1 },
  { 28, 3 },
  { 29, 1 },
  { 29, 4 },
  { 29, 3 },
  { 29, 4 },
  { 29, 6 },
  { 29, 7 },
  { 30, 3 },
  { 30, 1 },
  { 24, 3 },
  { 24, 1 },
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
      case 0: /* program ::= statements */
#line 36 ".\\code\\Ddc\\Parser.y"
{
	state->root = yymsp[0].minor.yy30->node;
}
#line 781 ".\\code\\Ddc\\Parser.c"
        break;
      case 1: /* statements ::= statement statements */
      case 20: /* members ::= member members */ yytestcase(yyruleno==20);
#line 41 ".\\code\\Ddc\\Parser.y"
{
	yymsp[-1].minor.yy30->node->setNext(yymsp[0].minor.yy30->node);
	yygotominor.yy30 = new Token(yymsp[-1].minor.yy30->node);
}
#line 790 ".\\code\\Ddc\\Parser.c"
        break;
      case 2: /* statements ::= statement */
      case 12: /* alias_type_subst ::= alias_type */ yytestcase(yyruleno==12);
      case 30: /* member_type_subst ::= member_type */ yytestcase(yyruleno==30);
#line 47 ".\\code\\Ddc\\Parser.y"
{
	yygotominor.yy30 = new Token(yymsp[0].minor.yy30->node);
}
#line 799 ".\\code\\Ddc\\Parser.c"
        break;
      case 3: /* statement ::= IMPORT qualified_literal SEMI_COLON */
#line 52 ".\\code\\Ddc\\Parser.y"
{
	yygotominor.yy30 = new Token(
		new DfnImport(
			yymsp[-1].minor.yy30->literal
		)
	);
  yy_destructor(yypParser,1,&yymsp[-2].minor);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 812 ".\\code\\Ddc\\Parser.c"
        break;
      case 4: /* statement ::= ALIAS STRING alias_type OPEN_BRACE STRING COMMA STRING COMMA STRING CLOSE_BRACE */
#line 61 ".\\code\\Ddc\\Parser.y"
{
	yygotominor.yy30 = new Token(
		new DfnAlias(
			yymsp[-8].minor.yy0->literal,	// language
			yymsp[-7].minor.yy30->node,	// type
			yymsp[-5].minor.yy0->literal,	// language declare type
			yymsp[-3].minor.yy0->literal,	// language in/out type
			yymsp[-1].minor.yy0->literal	// language member
		)
	);
  yy_destructor(yypParser,3,&yymsp[-9].minor);
  yy_destructor(yypParser,5,&yymsp[-6].minor);
  yy_destructor(yypParser,6,&yymsp[-4].minor);
  yy_destructor(yypParser,6,&yymsp[-2].minor);
  yy_destructor(yypParser,7,&yymsp[0].minor);
}
#line 832 ".\\code\\Ddc\\Parser.c"
        break;
      case 5: /* alias_type ::= LITERAL */
#line 74 ".\\code\\Ddc\\Parser.y"
{
	yygotominor.yy30 = new Token(new DfnType(yymsp[0].minor.yy0->literal, 0, false, 0));
}
#line 839 ".\\code\\Ddc\\Parser.c"
        break;
      case 6: /* alias_type ::= LITERAL LESS alias_type_subst GREATER */
#line 79 ".\\code\\Ddc\\Parser.y"
{
	yygotominor.yy30 = new Token(new DfnType(yymsp[-3].minor.yy0->literal, yymsp[-1].minor.yy30->node, false, 0));
  yy_destructor(yypParser,9,&yymsp[-2].minor);
  yy_destructor(yypParser,10,&yymsp[0].minor);
}
#line 848 ".\\code\\Ddc\\Parser.c"
        break;
      case 7: /* alias_type ::= LITERAL OPEN_BRACKET CLOSE_BRACKET */
#line 84 ".\\code\\Ddc\\Parser.y"
{
	yygotominor.yy30 = new Token(new DfnType(yymsp[-2].minor.yy0->literal, 0, true, 0));
  yy_destructor(yypParser,11,&yymsp[-1].minor);
  yy_destructor(yypParser,12,&yymsp[0].minor);
}
#line 857 ".\\code\\Ddc\\Parser.c"
        break;
      case 8: /* alias_type ::= LITERAL OPEN_BRACKET LITERAL CLOSE_BRACKET */
#line 89 ".\\code\\Ddc\\Parser.y"
{
	yygotominor.yy30 = new Token(new DfnType(yymsp[-3].minor.yy0->literal, 0, true, 0));
  yy_destructor(yypParser,11,&yymsp[-2].minor);
  yy_destructor(yypParser,12,&yymsp[0].minor);
}
#line 866 ".\\code\\Ddc\\Parser.c"
        break;
      case 9: /* alias_type ::= LITERAL LESS alias_type_subst GREATER OPEN_BRACKET CLOSE_BRACKET */
#line 94 ".\\code\\Ddc\\Parser.y"
{
	yygotominor.yy30 = new Token(new DfnType(yymsp[-5].minor.yy0->literal, yymsp[-3].minor.yy30->node, true, 0));
  yy_destructor(yypParser,9,&yymsp[-4].minor);
  yy_destructor(yypParser,10,&yymsp[-2].minor);
  yy_destructor(yypParser,11,&yymsp[-1].minor);
  yy_destructor(yypParser,12,&yymsp[0].minor);
}
#line 877 ".\\code\\Ddc\\Parser.c"
        break;
      case 10: /* alias_type ::= LITERAL LESS alias_type_subst GREATER OPEN_BRACKET LITERAL CLOSE_BRACKET */
#line 99 ".\\code\\Ddc\\Parser.y"
{
	yygotominor.yy30 = new Token(new DfnType(yymsp[-6].minor.yy0->literal, yymsp[-4].minor.yy30->node, true, 0));
  yy_destructor(yypParser,9,&yymsp[-5].minor);
  yy_destructor(yypParser,10,&yymsp[-3].minor);
  yy_destructor(yypParser,11,&yymsp[-2].minor);
  yy_destructor(yypParser,12,&yymsp[0].minor);
}
#line 888 ".\\code\\Ddc\\Parser.c"
        break;
      case 11: /* alias_type_subst ::= alias_type_subst COMMA alias_type */
      case 29: /* member_type_subst ::= member_type_subst COMMA member_type */ yytestcase(yyruleno==29);
#line 104 ".\\code\\Ddc\\Parser.y"
{
	yymsp[-2].minor.yy30->node->setNext(yymsp[0].minor.yy30->node);
	yygotominor.yy30 = new Token(yymsp[-2].minor.yy30->node);
  yy_destructor(yypParser,6,&yymsp[-1].minor);
}
#line 898 ".\\code\\Ddc\\Parser.c"
        break;
      case 13: /* statement ::= NAMESPACE LITERAL OPEN_BRACE statements CLOSE_BRACE */
#line 115 ".\\code\\Ddc\\Parser.y"
{
	yygotominor.yy30 = new Token(
		new DfnNamespace(
			yymsp[-3].minor.yy0->literal,
			yymsp[-1].minor.yy30->node
		)
	);
  yy_destructor(yypParser,13,&yymsp[-4].minor);
  yy_destructor(yypParser,5,&yymsp[-2].minor);
  yy_destructor(yypParser,7,&yymsp[0].minor);
}
#line 913 ".\\code\\Ddc\\Parser.c"
        break;
      case 14: /* statement ::= CLASS LITERAL OPEN_BRACE members CLOSE_BRACE */
#line 125 ".\\code\\Ddc\\Parser.y"
{
	yygotominor.yy30 = new Token(
		new DfnClass(
			DfnClass::AccPrivate,
			yymsp[-3].minor.yy0->literal,
			yymsp[-1].minor.yy30->node
		)
	);
  yy_destructor(yypParser,14,&yymsp[-4].minor);
  yy_destructor(yypParser,5,&yymsp[-2].minor);
  yy_destructor(yypParser,7,&yymsp[0].minor);
}
#line 929 ".\\code\\Ddc\\Parser.c"
        break;
      case 15: /* statement ::= PUBLIC CLASS LITERAL OPEN_BRACE members CLOSE_BRACE */
#line 136 ".\\code\\Ddc\\Parser.y"
{
	yygotominor.yy30 = new Token(
		new DfnClass(
			DfnClass::AccPublic,
			yymsp[-3].minor.yy0->literal,
			yymsp[-1].minor.yy30->node
		)
	);
  yy_destructor(yypParser,15,&yymsp[-5].minor);
  yy_destructor(yypParser,14,&yymsp[-4].minor);
  yy_destructor(yypParser,5,&yymsp[-2].minor);
  yy_destructor(yypParser,7,&yymsp[0].minor);
}
#line 946 ".\\code\\Ddc\\Parser.c"
        break;
      case 16: /* statement ::= PRIVATE CLASS LITERAL OPEN_BRACE members CLOSE_BRACE */
#line 147 ".\\code\\Ddc\\Parser.y"
{
	yygotominor.yy30 = new Token(
		new DfnClass(
			DfnClass::AccPrivate,
			yymsp[-3].minor.yy0->literal,
			yymsp[-1].minor.yy30->node
		)
	);
  yy_destructor(yypParser,16,&yymsp[-5].minor);
  yy_destructor(yypParser,14,&yymsp[-4].minor);
  yy_destructor(yypParser,5,&yymsp[-2].minor);
  yy_destructor(yypParser,7,&yymsp[0].minor);
}
#line 963 ".\\code\\Ddc\\Parser.c"
        break;
      case 17: /* statement ::= CLASS LITERAL COLON qualified_literal OPEN_BRACE members CLOSE_BRACE */
#line 158 ".\\code\\Ddc\\Parser.y"
{
	yygotominor.yy30 = new Token(
		new DfnClass(
			DfnClass::AccPrivate,
			yymsp[-5].minor.yy0->literal,
			yymsp[-3].minor.yy30->literal,
			yymsp[-1].minor.yy30->node
		)
	);
  yy_destructor(yypParser,14,&yymsp[-6].minor);
  yy_destructor(yypParser,17,&yymsp[-4].minor);
  yy_destructor(yypParser,5,&yymsp[-2].minor);
  yy_destructor(yypParser,7,&yymsp[0].minor);
}
#line 981 ".\\code\\Ddc\\Parser.c"
        break;
      case 18: /* statement ::= PUBLIC CLASS LITERAL COLON qualified_literal OPEN_BRACE members CLOSE_BRACE */
#line 170 ".\\code\\Ddc\\Parser.y"
{
	yygotominor.yy30 = new Token(
		new DfnClass(
			DfnClass::AccPublic,
			yymsp[-5].minor.yy0->literal,
			yymsp[-3].minor.yy30->literal,
			yymsp[-1].minor.yy30->node
		)
	);
  yy_destructor(yypParser,15,&yymsp[-7].minor);
  yy_destructor(yypParser,14,&yymsp[-6].minor);
  yy_destructor(yypParser,17,&yymsp[-4].minor);
  yy_destructor(yypParser,5,&yymsp[-2].minor);
  yy_destructor(yypParser,7,&yymsp[0].minor);
}
#line 1000 ".\\code\\Ddc\\Parser.c"
        break;
      case 19: /* statement ::= PRIVATE CLASS LITERAL COLON qualified_literal OPEN_BRACE members CLOSE_BRACE */
#line 182 ".\\code\\Ddc\\Parser.y"
{
	yygotominor.yy30 = new Token(
		new DfnClass(
			DfnClass::AccPrivate,
			yymsp[-5].minor.yy0->literal,
			yymsp[-3].minor.yy30->literal,
			yymsp[-1].minor.yy30->node
		)
	);
  yy_destructor(yypParser,16,&yymsp[-7].minor);
  yy_destructor(yypParser,14,&yymsp[-6].minor);
  yy_destructor(yypParser,17,&yymsp[-4].minor);
  yy_destructor(yypParser,5,&yymsp[-2].minor);
  yy_destructor(yypParser,7,&yymsp[0].minor);
}
#line 1019 ".\\code\\Ddc\\Parser.c"
        break;
      case 21: /* members ::= member */
#line 200 ".\\code\\Ddc\\Parser.y"
{
	yygotominor.yy30 = new Token(
		yymsp[0].minor.yy30->node
	);
}
#line 1028 ".\\code\\Ddc\\Parser.c"
        break;
      case 22: /* member ::= member_type LITERAL SEMI_COLON */
#line 207 ".\\code\\Ddc\\Parser.y"
{
	yygotominor.yy30 = new Token(
		new DfnMember(yymsp[-2].minor.yy30->node, yymsp[-1].minor.yy0->literal)
	);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 1038 ".\\code\\Ddc\\Parser.c"
        break;
      case 23: /* member_type ::= qualified_literal */
#line 214 ".\\code\\Ddc\\Parser.y"
{
	yygotominor.yy30 = new Token(new DfnType(yymsp[0].minor.yy30->literal, 0, false, 0));
}
#line 1045 ".\\code\\Ddc\\Parser.c"
        break;
      case 24: /* member_type ::= qualified_literal LESS member_type_subst GREATER */
#line 219 ".\\code\\Ddc\\Parser.y"
{
	yygotominor.yy30 = new Token(new DfnType(yymsp[-3].minor.yy30->literal, yymsp[-1].minor.yy30->node, false, 0));
  yy_destructor(yypParser,9,&yymsp[-2].minor);
  yy_destructor(yypParser,10,&yymsp[0].minor);
}
#line 1054 ".\\code\\Ddc\\Parser.c"
        break;
      case 25: /* member_type ::= qualified_literal OPEN_BRACKET CLOSE_BRACKET */
#line 224 ".\\code\\Ddc\\Parser.y"
{
	yygotominor.yy30 = new Token(new DfnType(yymsp[-2].minor.yy30->literal, 0, true, 0));
  yy_destructor(yypParser,11,&yymsp[-1].minor);
  yy_destructor(yypParser,12,&yymsp[0].minor);
}
#line 1063 ".\\code\\Ddc\\Parser.c"
        break;
      case 26: /* member_type ::= qualified_literal OPEN_BRACKET NUMBER CLOSE_BRACKET */
#line 229 ".\\code\\Ddc\\Parser.y"
{
	yygotominor.yy30 = new Token(new DfnType(yymsp[-3].minor.yy30->literal, 0, true, int32_t(yymsp[-1].minor.yy0->number)));
  yy_destructor(yypParser,11,&yymsp[-2].minor);
  yy_destructor(yypParser,12,&yymsp[0].minor);
}
#line 1072 ".\\code\\Ddc\\Parser.c"
        break;
      case 27: /* member_type ::= qualified_literal LESS member_type_subst GREATER OPEN_BRACKET CLOSE_BRACKET */
#line 234 ".\\code\\Ddc\\Parser.y"
{
	yygotominor.yy30 = new Token(new DfnType(yymsp[-5].minor.yy30->literal, yymsp[-3].minor.yy30->node, true, 0));
  yy_destructor(yypParser,9,&yymsp[-4].minor);
  yy_destructor(yypParser,10,&yymsp[-2].minor);
  yy_destructor(yypParser,11,&yymsp[-1].minor);
  yy_destructor(yypParser,12,&yymsp[0].minor);
}
#line 1083 ".\\code\\Ddc\\Parser.c"
        break;
      case 28: /* member_type ::= qualified_literal LESS member_type_subst GREATER OPEN_BRACKET NUMBER CLOSE_BRACKET */
#line 239 ".\\code\\Ddc\\Parser.y"
{
	yygotominor.yy30 = new Token(new DfnType(yymsp[-6].minor.yy30->literal, yymsp[-4].minor.yy30->node, true, int32_t(yymsp[-1].minor.yy0->number)));
  yy_destructor(yypParser,9,&yymsp[-5].minor);
  yy_destructor(yypParser,10,&yymsp[-3].minor);
  yy_destructor(yypParser,11,&yymsp[-2].minor);
  yy_destructor(yypParser,12,&yymsp[0].minor);
}
#line 1094 ".\\code\\Ddc\\Parser.c"
        break;
      case 31: /* qualified_literal ::= LITERAL DOT qualified_literal */
#line 255 ".\\code\\Ddc\\Parser.y"
{
	yygotominor.yy30 = new Token(yymsp[-2].minor.yy0->literal + L"." + yymsp[0].minor.yy30->literal);
  yy_destructor(yypParser,19,&yymsp[-1].minor);
}
#line 1102 ".\\code\\Ddc\\Parser.c"
        break;
      case 32: /* qualified_literal ::= LITERAL */
#line 260 ".\\code\\Ddc\\Parser.y"
{
	yygotominor.yy30 = yymsp[0].minor.yy0;
}
#line 1109 ".\\code\\Ddc\\Parser.c"
        break;
      default:
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
#line 29 ".\\code\\Ddc\\Parser.y"


log::error << L"Syntax error; unable to generate data definition class" << Endl;

#line 1176 ".\\code\\Ddc\\Parser.c"
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
