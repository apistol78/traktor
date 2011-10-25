// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
//   Copyright (C) 2008 Pekka Lampila
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#ifndef _CHECK_AS_
#define _CHECK_AS_

#define OUTPUT_VERSION __SWF_VERSION__

#define MING_VERSION_CODE 00047777

#define MING_SUPPORTS_ASM
#define MING_SUPPORTS_ASM_EXTENDS
#define MING_SUPPORTS_ASM_GETPROPERTY
#define MING_SUPPORTS_ASM_SETPROPERTY
#define MING_SUPPORTS_ASM_TONUMBER
#define MING_SUPPORTS_ASM_TOSTRING
#define MING_SUPPORTS_ASM_TARGETPATH
#define MING_SUPPORTS_ASM_IMPLEMENTS

#define note(text) trace ("NOTE: " + text)

#define pass_check(text) trace ("PASSED: " + text)
#define xpass_check(text) trace ("XPASSED: " + text)
#define fail_check(text) trace ("FAILED: " + text)
#define xfail_check(text) trace ("XFAILED: " + text)
#define info(x) trace (x)

#define MEDIA(x) "../media/" + #x

#define INFO " [" + __FILE__ + ":" + __LINE__ + "]" 

#define totals(x) trace ("totals"); getURL ('fscommand:quit', '');
#define xtotals(x) trace ("xtotals"); getURL ('fscommand:quit', '');
#define check_totals(x) trace ("check_totals: " + #x); \
  getURL ('fscommand:quit', '');
#define xcheck_totals(x) trace ("xcheck_totals: " + #x); \
  getURL ('fscommand:quit', '');

//
// Use check (<expression>)
//
#define check(expr)  \
	if (expr) pass_check (#expr + \
		" [" + __FILE__ + ":" + __LINE__ + "]" ); \
	else fail_check (#expr + \
		" [" + __FILE__ + ":" + __LINE__ + "]" ); \

#define xcheck(expr)  \
        if (expr) xpass_check (#expr + \
		" [" + __FILE__ + ":" + __LINE__ + "]" ); \
        else xfail_check (#expr + \
		" [" + __FILE__ + ":" + __LINE__ + "]" ); \

//
// Use check_equals (<obtained>, <expected>)
//
#define check_equals(obt, exp)  \
	if (obt == exp) pass_check ( \
		#obt + " == " + #exp + \
		" [" + __FILE__ + ":" + __LINE__ + "]" ); \
	else fail_check ("expected: " + #exp + \
		" obtained: " + obt + \
		" [" + __FILE__ + ":" + __LINE__ + "]" ); \

#define xcheck_equals(obt, exp)  \
        if (obt == exp) xpass_check ( \
                #obt + " == " + #exp + \
		" [" + __FILE__ + ":" + __LINE__ + "]" ); \
        else xfail_check ("expected: " + #exp + \
                " obtained: " + obt + \
		" [" + __FILE__ + ":" + __LINE__ + "]" ); \

#endif // _CHECK_AS_
