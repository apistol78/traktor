// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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


// Test case for ContextMenu ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf


rcsid="$Id: ContextMenu.as,v 1.14 2008/03/11 19:31:47 strk Exp $";
#include "check.as"

#if OUTPUT_VERSION < 7

note("You can't rely on availability of ContextMenu class with an SWF < 7");

#else // OUTPUT_VERSION >= 7

// there was no ContextMenu before SWF7
check_equals(typeof(ContextMenu), 'function');

var contextmenuObj = new ContextMenu;

// test the ContextMenu constuctor
check_equals (typeof(contextmenuObj), 'object');

// test the ContextMenu::copy method
check_equals (typeof(contextmenuObj.copy), 'function');

// test the ContextMenu::hideBuiltinItems method
check_equals (typeof(contextmenuObj.hideBuiltInItems), 'function');

// test existance of ContextMenu::builtInItems object
xcheck_equals (typeof(contextmenuObj.builtInItems), 'object');
check ( ! contextmenuObj.builtInItems instanceOf Array );
check_equals (typeof(contextmenuObj.builtInItems.length), 'undefined');

// test existance of ContextMenu::customItems object
xcheck_equals (typeof(contextmenuObj.customItems), 'object');
check ( ! contextmenuObj.builtInItems instanceOf Array );
xcheck_equals (typeof(contextmenuObj.customItems.length), 'number');

// test existance of ContextMenu::onSelect object
check_equals (typeof(contextmenuObj.onSelect), 'undefined');

//----------------------------------------------
// Test onSelect
//----------------------------------------------

function callback() { }
var contextMenuObj2 = new ContextMenu(callback);
check_equals(typeof(contextMenuObj2.onSelect), 'function');
check_equals(contextMenuObj2.onSelect, callback);
function callback2() { }
contextMenuObj2.onSelect = callback2;
check_equals(typeof(contextMenuObj2.onSelect), 'function');
check_equals(contextMenuObj2.onSelect, callback2);
contextMenuObj2.onSelect = null;
check_equals(typeof(contextMenuObj2.onSelect), 'null');
contextMenuObj2.onSelect = undefined;
check_equals(typeof(contextMenuObj2.onSelect), 'undefined');
contextMenuObj2.onSelect = 4;
check_equals(typeof(contextMenuObj2.onSelect), 'number');

#endif
totals();
