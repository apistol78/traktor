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

// Test case for Stage ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf


rcsid="$Id: Key.as,v 1.7 2008/03/11 19:31:47 strk Exp $";
#include "check.as"

check_equals (typeof(Key), 'object');

var keyObj = new Key;
check_equals (typeof(keyObj), 'undefined');

check_equals(Key.__proto__, Object.prototype);

// Methods
#if OUTPUT_VERSION > 5

 // Key was implicitly initialized by ASBroadcaster.initialize !
 // See http://www.senocular.com/flash/tutorials/listenersasbroadcaster/?page=2
 check_equals(typeof(Key.addListener), 'function');
 check_equals(typeof(Key.removeListener), 'function');
 check_equals(typeof(Key.broadcastMessage), 'function');
 check(Key.hasOwnProperty("_listeners"));
 check_equals(typeof(Key._listeners), 'object');
 check(Key._listeners instanceof Array);

#else // OUTPUT_VERSION <= 5

 check_equals(typeof(Key.addListener), 'undefined');
 check_equals(typeof(Key.removeListener), 'undefined');

#endif // OUTPUT_VERSION <= 5

check_equals(typeof(Key.getAscii), 'function');
check_equals(typeof(Key.getCode), 'function');
check_equals(typeof(Key.isDown), 'function');
check_equals(typeof(Key.isToggled), 'function');

// Properties
check_equals(typeof(Key.CAPSLOCK), 'number');
check_equals(Key.CAPSLOCK, 20);
check_equals(typeof(Key.TAB), 'number');
check_equals(Key.TAB, 9);
check_equals(typeof(Key.CONTROL), 'number');
check_equals(Key.CONTROL, 17);
check_equals(typeof(Key.SHIFT), 'number');
check_equals(Key.SHIFT, 16);
check_equals(typeof(Key.DELETEKEY), 'number');
check_equals(Key.DELETEKEY, 46);
check_equals(typeof(Key.INSERT), 'number');
check_equals(Key.INSERT, 45);
check_equals(typeof(Key.BACKSPACE), 'number');
check_equals(Key.BACKSPACE, 8);
check_equals(typeof(Key.LEFT), 'number');
check_equals(Key.LEFT, 37);
check_equals(typeof(Key.UP), 'number');
check_equals(Key.UP, 38);
check_equals(typeof(Key.RIGHT), 'number');
check_equals(Key.RIGHT, 39);
check_equals(typeof(Key.DOWN), 'number');
check_equals(Key.DOWN, 40);
check_equals(typeof(Key.HOME), 'number');
check_equals(Key.HOME, 36);
check_equals(typeof(Key.END), 'number');
check_equals(Key.END, 35);
check_equals(typeof(Key.ENTER), 'number');
check_equals(Key.ENTER, 13);
check_equals(typeof(Key.ESCAPE), 'number');
check_equals(Key.ESCAPE, 27);
check_equals(typeof(Key.PGDN), 'number');
check_equals(Key.PGDN, 34);
check_equals(typeof(Key.PGUP), 'number');
check_equals(Key.PGUP, 33);
check_equals(typeof(Key.SPACE), 'number');
check_equals(Key.SPACE, 32);

// Event handlers are:
// onKeyUp
// onKeyDown
totals();
