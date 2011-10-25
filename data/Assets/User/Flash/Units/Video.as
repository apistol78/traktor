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

// Test case for Video ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf


rcsid="$Id: Video.as,v 1.16 2008/03/29 22:50:54 strk Exp $";
#include "check.as"

#if OUTPUT_VERSION < 6

check_equals(Video, undefined);
check_totals(1);

#else

// test Video class an interface availability
check_equals(typeof(Video), 'function');
check_equals(typeof(Video.prototype), 'object');
check_equals(typeof(Video.prototype.__proto__), 'object');
check_equals(Video.prototype.__proto__, Object.prototype);
check_equals(typeof(Video.prototype.attachVideo), 'function');
check_equals(typeof(Video.prototype.clear), 'function');

// test Video instance
var videoObj = new Video;
check_equals (typeof(videoObj), 'object');
check_equals (typeof(videoObj.attachVideo), 'function');
check_equals (typeof(videoObj.clear), 'function');
check_equals (typeof(videoObj._x), 'undefined');
check_equals (typeof(videoObj._y), 'undefined');
check_equals (typeof(videoObj._width), 'undefined');
check_equals (typeof(videoObj._height), 'undefined');
check_equals (typeof(videoObj._xscale), 'undefined');
check_equals (typeof(videoObj._yscale), 'undefined');
check_equals (typeof(videoObj._xmouse), 'undefined');
check_equals (typeof(videoObj._ymouse), 'undefined');
check_equals (typeof(videoObj._alpha), 'undefined');
check_equals (typeof(videoObj._rotation), 'undefined');
check_equals (typeof(videoObj._target), 'undefined');
check_equals (typeof(videoObj._parent), 'undefined');

// TODO: test other properties !

check_totals(21);

#endif
