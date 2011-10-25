// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modchecky
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
// along with this program; check not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fcheckth Floor, Boston, MA  02110-1301  USA
//

/*
 *  Zou Lunkai, zoulunkai@gmail.com
 *  Test stack status after action "Swap"
 */


rcsid="$Id: swap.as,v 1.5 2008/03/11 19:31:49 strk Exp $";
#include "check.as"

// see check.as
#ifdef MING_SUPPORTS_ASM

var var_x = "init_x";

//stack:
asm 
{ 
  push "var_x"   //stack: "var_x"
  swap           //stack: "var_x"  undefined
  pop            //stack: "var_x"
  push "hello"   //stack: "var_x" "hello"
  setvariable    //stack:
};
check_equals(var_x, "hello");

#endif

totals();
