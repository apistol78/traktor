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

// Test case for Mouse ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf


rcsid="$Id: Mouse.as,v 1.19 2008/04/16 11:16:34 bwy Exp $";
#include "check.as"

check_equals ( typeof(Mouse), 'object' );

// Mouse object can't be instanciated !
var mouseObj = new Mouse;
check_equals(mouseObj, undefined);

check_equals (typeof(Mouse.__proto__), 'object');
check_equals (Mouse.__proto__, Object.prototype);

// test the Mouse::hide method
check_equals ( typeof(Mouse.hide), 'function' );
check_equals ( typeof(Mouse.show), 'function' );
#if OUTPUT_VERSION > 5

 // Mouse was implicitly initialized by ASBroadcaster.initialize !
 // See http://www.senocular.com/flash/tutorials/listenersasbroadcaster/?page=2
 check_equals ( typeof(Mouse.removeListener), 'function' );
 check_equals ( typeof(Mouse.addListener), 'function' );
 check_equals( typeof(Mouse.broadcastMessage), 'function' );
 check(Mouse.hasOwnProperty("_listeners"));
 check_equals(typeof(Mouse._listeners), 'object');
 check(Mouse._listeners instanceof Array);

 check(Mouse.hasOwnProperty("hide"));
 check(Mouse.hasOwnProperty("show"));

#endif // OUTPUT_VERSION > 5


// test the Mouse::show method
check_equals ( typeof(Mouse.show), 'function' );
#if OUTPUT_VERSION > 5
check_totals(15);
#else
check_totals(7);
#endif
