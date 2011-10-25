// 
//   Copyright (C) 2008 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
//
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
// Test case for TextFormat ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="$Id: TextFormat.as,v 1.4 2008/05/06 11:21:02 bwy Exp $";

#include "check.as"

Object.prototype.hasOwnProperty = ASnative(101, 5);

check_equals(typeof(TextFormat), 'function');
check_equals(typeof(TextFormat.prototype), 'object');
tfObj = new TextFormat();
check_equals(typeof(tfObj), 'object');
check(tfObj instanceof TextFormat);

// The members below would not exist before
// the construction of first TextFormat object
check(TextFormat.prototype.hasOwnProperty('display'));
check(TextFormat.prototype.hasOwnProperty('bullet'));
check(TextFormat.prototype.hasOwnProperty('tabStops'));
check(TextFormat.prototype.hasOwnProperty('blockIndent'));
check(TextFormat.prototype.hasOwnProperty('leading'));
check(TextFormat.prototype.hasOwnProperty('indent'));
check(TextFormat.prototype.hasOwnProperty('rightMargin'));
check(TextFormat.prototype.hasOwnProperty('leftMargin'));
check(TextFormat.prototype.hasOwnProperty('align'));
check(TextFormat.prototype.hasOwnProperty('underline'));
check(TextFormat.prototype.hasOwnProperty('italic'));
check(TextFormat.prototype.hasOwnProperty('bold'));
check(TextFormat.prototype.hasOwnProperty('target'));
check(TextFormat.prototype.hasOwnProperty('url'));
check(TextFormat.prototype.hasOwnProperty('color'));
check(TextFormat.prototype.hasOwnProperty('size'));
check(TextFormat.prototype.hasOwnProperty('font'));
check(!TextFormat.prototype.hasOwnProperty('getTextExtent'));
check(tfObj.hasOwnProperty('getTextExtent'));


// When you construct a TextFormat w/out args all members
// are of the 'null' type. In general, uninitialized members
// are all of the 'null' type.
xcheck_equals(typeof(tfObj.display), 'string');
xcheck_equals(tfObj.display, 'block');
check_equals(typeof(tfObj.bullet), 'null');
xcheck_equals(typeof(tfObj.tabStops), 'null');
check_equals(typeof(tfObj.blockIndent), 'null');
check_equals(typeof(tfObj.leading), 'null');
check_equals(typeof(tfObj.indent), 'null');
check_equals(typeof(tfObj.rightMargin), 'null');
check_equals(typeof(tfObj.leftMargin), 'null');
check_equals(typeof(tfObj.align), 'null');
check_equals(typeof(tfObj.underline), 'null');
check_equals(typeof(tfObj.italic), 'null');
check_equals(typeof(tfObj.bold), 'null');
xcheck_equals(typeof(tfObj.target), 'null');
xcheck_equals(typeof(tfObj.url), 'null');
check_equals(typeof(tfObj.color), 'null');
check_equals(typeof(tfObj.size), 'null');
check_equals(typeof(tfObj.font), 'null');
check_equals(typeof(tfObj.getTextExtent), 'function');

// new TextFormat([font, [size, [color, [bold, [italic, [underline, [url, [target, [align,[leftMargin, [rightMargin, [indent, [leading]]]]]]]]]]]]])
tfObj = new TextFormat("fname", 2, 30, true, false, true, 'http', 'tgt', 'cEnter', '23', '32', 12, 4);
xcheck_equals(typeof(tfObj.display), 'string');
xcheck_equals(tfObj.display, 'block');
check_equals(typeof(tfObj.bullet), 'null');
xcheck_equals(typeof(tfObj.tabStops), 'null');
check_equals(typeof(tfObj.blockIndent), 'null');
check_equals(tfObj.leading, 4);
check_equals(tfObj.indent, 12);
check_equals(typeof(tfObj.rightMargin), 'number'); // even if we passed a string to it
check_equals(tfObj.rightMargin, 32);
check_equals(typeof(tfObj.leftMargin), 'number'); // even if we passed a string to it
check_equals(tfObj.leftMargin, 23);
check_equals(tfObj.align, 'center');
xcheck_equals(tfObj.target, 'tgt');
xcheck_equals(tfObj.url, 'http');
check_equals(tfObj.underline, true);
check_equals(typeof(tfObj.italic), 'boolean');
check_equals(tfObj.italic, false);
check_equals(tfObj.bold, true);
check_equals(tfObj.color, 30);
check_equals(tfObj.size, 2);
check_equals(tfObj.font, 'fname');



check_totals(63);
