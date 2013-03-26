#ifndef traktor_model_LwTags_H
#define traktor_model_LwTags_H

#include "Core/Config.h"

namespace traktor
{
	namespace model
	{

#define MAKE_FOURCC(a, b, c, d)	(((a) << 24) | ((b) << 16) | ((c) << 8) | ((d) << 0))

struct LwTags
{
	enum
	{

FORM = MAKE_FOURCC('F','O','R','M'), 
LWO2 = MAKE_FOURCC('L','W','O','2'),
LWOB = MAKE_FOURCC('L','W','O','B'),
	
/* top-level chunks */
LAYR = MAKE_FOURCC('L','A','Y','R'),
TAGS = MAKE_FOURCC('T','A','G','S'),
PNTS = MAKE_FOURCC('P','N','T','S'),
BBOX = MAKE_FOURCC('B','B','O','X'),
VMAP = MAKE_FOURCC('V','M','A','P'),
VMAD = MAKE_FOURCC('V','M','A','D'),
POLS = MAKE_FOURCC('P','O','L','S'),
PTAG = MAKE_FOURCC('P','T','A','G'),
ENVL = MAKE_FOURCC('E','N','V','L'),
CLIP = MAKE_FOURCC('C','L','I','P'),
SURF = MAKE_FOURCC('S','U','R','F'),
DESC = MAKE_FOURCC('D','E','S','C'),
TEXT = MAKE_FOURCC('T','E','X','T'),
ICON = MAKE_FOURCC('I','C','O','N'),
SRFS = MAKE_FOURCC('S','R','F','S'),

/* vmap types */
PICK = MAKE_FOURCC('P','I','C','K'),
WGHT = MAKE_FOURCC('W','G','H','T'),
MNVW = MAKE_FOURCC('M','N','V','W'),
TXUV = MAKE_FOURCC('T','X','U','V'),
RGB  = MAKE_FOURCC('R','G','B',' '),
RGBA = MAKE_FOURCC('R','G','B','A'),
MORF = MAKE_FOURCC('M','O','R','F'),
SPOT = MAKE_FOURCC('S','P','O','T'),
	
/* polygon types */
FACE = MAKE_FOURCC('F','A','C','E'),
CURV = MAKE_FOURCC('C','U','R','V'),
PTCH = MAKE_FOURCC('P','T','C','H'),
MBAL = MAKE_FOURCC('M','B','A','L'),
BONE = MAKE_FOURCC('B','O','N','E'),

/* polygon tags */
//	SURF = MAKE_FOURCC('S','U','R','F'),
PART = MAKE_FOURCC('P','A','R','T'),
SMGP = MAKE_FOURCC('S','M','G','P'),

/* envelopes */
PRE  = MAKE_FOURCC('P','R','E',' '),
POST = MAKE_FOURCC('P','O','S','T'),
KEY  = MAKE_FOURCC('K','E','Y',' '),
SPAN = MAKE_FOURCC('S','P','A','N'),
TCB  = MAKE_FOURCC('T','C','B',' '),
HERM = MAKE_FOURCC('H','E','R','M'),
BEZI = MAKE_FOURCC('B','E','Z','I'),
BEZ2 = MAKE_FOURCC('B','E','Z','2'),
LINE = MAKE_FOURCC('L','I','N','E'),
STEP = MAKE_FOURCC('S','T','E','P'),

/* clips */
STIL = MAKE_FOURCC('S','T','I','L'),
ISEQ = MAKE_FOURCC('I','S','E','Q'),
ANIM = MAKE_FOURCC('A','N','I','M'),
XREF = MAKE_FOURCC('X','R','E','F'),
STCC = MAKE_FOURCC('S','T','C','C'),
TIME = MAKE_FOURCC('T','I','M','E'),
CONT = MAKE_FOURCC('C','O','N','T'),
BRIT = MAKE_FOURCC('B','R','I','T'),
SATR = MAKE_FOURCC('S','A','T','R'),
HUE  = MAKE_FOURCC('H','U','E',' '),
GAMM = MAKE_FOURCC('G','A','M','M'),
NEGA = MAKE_FOURCC('N','E','G','A'),
IFLT = MAKE_FOURCC('I','F','L','T'),
PFLT = MAKE_FOURCC('P','F','L','T'),

/* surfaces */
//	COLR = MAKE_FOURCC('C','O','L','R'),
LUMI = MAKE_FOURCC('L','U','M','I'),
DIFF = MAKE_FOURCC('D','I','F','F'),
SPEC = MAKE_FOURCC('S','P','E','C'),
GLOS = MAKE_FOURCC('G','L','O','S'),
REFL = MAKE_FOURCC('R','E','F','L'),
RFOP = MAKE_FOURCC('R','F','O','P'),
RIMG = MAKE_FOURCC('R','I','M','G'),
RSAN = MAKE_FOURCC('R','S','A','N'),
TRAN = MAKE_FOURCC('T','R','A','N'),
TROP = MAKE_FOURCC('T','R','O','P'),
TIMG = MAKE_FOURCC('T','I','M','G'),
RIND = MAKE_FOURCC('R','I','N','D'),
TRNL = MAKE_FOURCC('T','R','N','L'),
BUMP = MAKE_FOURCC('B','U','M','P'),
SMAN = MAKE_FOURCC('S','M','A','N'),
SIDE = MAKE_FOURCC('S','I','D','E'),
CLRH = MAKE_FOURCC('C','L','R','H'),
CLRF = MAKE_FOURCC('C','L','R','F'),
ADTR = MAKE_FOURCC('A','D','T','R'),
SHRP = MAKE_FOURCC('S','H','R','P'),
//	LINE = MAKE_FOURCC('L','I','N','E'),
LSIZ = MAKE_FOURCC('L','S','I','Z'),
ALPH = MAKE_FOURCC('A','L','P','H'),
AVAL = MAKE_FOURCC('A','V','A','L'),
GVAL = MAKE_FOURCC('G','V','A','L'),
CMNT = MAKE_FOURCC('C','M','N','T'),
BLOK = MAKE_FOURCC('B','L','O','K'),

/* texture layer */
TYPE = MAKE_FOURCC('T','Y','P','E'),
CHAN = MAKE_FOURCC('C','H','A','N'),
NAME = MAKE_FOURCC('N','A','M','E'),
ENAB = MAKE_FOURCC('E','N','A','B'),
OPAC = MAKE_FOURCC('O','P','A','C'),
FLAG = MAKE_FOURCC('F','L','A','G'),
PROJ = MAKE_FOURCC('P','R','O','J'),
STCK = MAKE_FOURCC('S','T','C','K'),
TAMP = MAKE_FOURCC('T','A','M','P'),

/* texture coordinates */
TMAP = MAKE_FOURCC('T','M','A','P'),
AXIS = MAKE_FOURCC('A','X','I','S'),
CNTR = MAKE_FOURCC('C','N','T','R'),
SIZE = MAKE_FOURCC('S','I','Z','E'),
ROTA = MAKE_FOURCC('R','O','T','A'),
OREF = MAKE_FOURCC('O','R','E','F'),
FALL = MAKE_FOURCC('F','A','L','L'),
CSYS = MAKE_FOURCC('C','S','Y','S'),

/* image map */
IMAP = MAKE_FOURCC('I','M','A','P'),
IMAG = MAKE_FOURCC('I','M','A','G'),
WRAP = MAKE_FOURCC('W','R','A','P'),
WRPW = MAKE_FOURCC('W','R','P','W'),
WRPH = MAKE_FOURCC('W','R','P','H'),
//	VMAP = MAKE_FOURCC('V','M','A','P'),
AAST = MAKE_FOURCC('A','A','S','T'),
PIXB = MAKE_FOURCC('P','I','X','B'),

/* procedural */
PROC = MAKE_FOURCC('P','R','O','C'),
COLR = MAKE_FOURCC('C','O','L','R'),
VALU = MAKE_FOURCC('V','A','L','U'),
FUNC = MAKE_FOURCC('F','U','N','C'),
FTPS = MAKE_FOURCC('F','T','P','S'),
ITPS = MAKE_FOURCC('I','T','P','S'),
ETPS = MAKE_FOURCC('E','T','P','S'),

/* gradient */
GRAD = MAKE_FOURCC('G','R','A','D'),
GRST = MAKE_FOURCC('G','R','S','T'),
GREN = MAKE_FOURCC('G','R','E','N'),
PNAM = MAKE_FOURCC('P','N','A','M'),
INAM = MAKE_FOURCC('I','N','A','M'),
GRPT = MAKE_FOURCC('G','R','P','T'),
FKEY = MAKE_FOURCC('F','K','E','Y'),
IKEY = MAKE_FOURCC('I','K','E','Y'),

/* shader */
SHDR = MAKE_FOURCC('S','H','D','R'),
DATA = MAKE_FOURCC('D','A','T','A')

	};
};

	}
}

#endif	// traktor_model_LwTags_H
