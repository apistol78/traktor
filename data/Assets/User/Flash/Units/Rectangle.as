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

rcsid="$Id: Rectangle.as,v 1.2 2008/05/07 08:03:40 strk Exp $";

#include "check.as"

#if OUTPUT_VERSION < 8

check_equals(typeof(flash), 'undefined');

check_totals(1);

#else

Rectangle = flash.geom.Rectangle;
check_equals(typeof(Rectangle), 'function');
check_equals(typeof(Rectangle.prototype), 'object');
check(Rectangle.prototype.hasOwnProperty('bottom'));
check(Rectangle.prototype.hasOwnProperty('bottomRight'));
check(Rectangle.prototype.hasOwnProperty('left'));
check(Rectangle.prototype.hasOwnProperty('right'));
check(Rectangle.prototype.hasOwnProperty('size'));
check(Rectangle.prototype.hasOwnProperty('top'));
check(Rectangle.prototype.hasOwnProperty('topLeft'));
check(Rectangle.prototype.hasOwnProperty('clone'));
check(Rectangle.prototype.hasOwnProperty('contains'));
check(Rectangle.prototype.hasOwnProperty('containsPoint'));
check(Rectangle.prototype.hasOwnProperty('containsRectangle'));
check(Rectangle.prototype.hasOwnProperty('equals'));
check(Rectangle.prototype.hasOwnProperty('inflate'));
check(Rectangle.prototype.hasOwnProperty('inflatePoint'));
check(Rectangle.prototype.hasOwnProperty('intersection'));
check(Rectangle.prototype.hasOwnProperty('intersects'));
check(Rectangle.prototype.hasOwnProperty('isEmpty'));
check(Rectangle.prototype.hasOwnProperty('offset'));
check(Rectangle.prototype.hasOwnProperty('offsetPoint'));
check(Rectangle.prototype.hasOwnProperty('setEmpty'));
check(Rectangle.prototype.hasOwnProperty('toString'));
check(Rectangle.prototype.hasOwnProperty('union'));
check(!Rectangle.prototype.hasOwnProperty('height'));
check(!Rectangle.prototype.hasOwnProperty('width'));
check(!Rectangle.prototype.hasOwnProperty('x'));
check(!Rectangle.prototype.hasOwnProperty('y'));

//-------------------------------------------------------------
// Test constructor (and width, height, x, y)
//-------------------------------------------------------------

r0 = new Rectangle();
check_equals(typeof(r0), 'object');
check(r0 instanceof Rectangle);
check(r0.hasOwnProperty('height'));
check(r0.hasOwnProperty('width'));
check(r0.hasOwnProperty('x'));
check(r0.hasOwnProperty('y'));
check_equals(''+r0, '(x=0, y=0, w=0, h=0)');
check(r0.isEmpty());
check_equals(typeof(r0.x), 'number');
check_equals(typeof(r0.y), 'number');
check_equals(typeof(r0.width), 'number');
check_equals(typeof(r0.height), 'number');

a = []; for (var i in r0) a.push(i);
check_equals(a.length, 26); // most of them...

r0 = new Rectangle(1);
check(r0.hasOwnProperty('height'));
check(r0.hasOwnProperty('width'));
check(r0.hasOwnProperty('x'));
check(r0.hasOwnProperty('y'));
check_equals(''+r0, '(x=1, y=undefined, w=undefined, h=undefined)');
check(r0.isEmpty());

r0 = new Rectangle(1, 1, 1, 'string');
check_equals(''+r0, '(x=1, y=1, w=1, h=string)');
check(r0.isEmpty());

r0 = new Rectangle(['a',3], 1, -30, 'string');
check_equals(''+r0, '(x=a,3, y=1, w=-30, h=string)');
check(r0.isEmpty());
check_equals(typeof(r0.width), 'number');
check_equals(typeof(r0.height), 'string');

o1 = {}; o1.toString = function() { return '2'; };
r0 = new Rectangle(0, 0, o1, null);
check_equals(''+r0, '(x=0, y=0, w=2, h=null)');
check(r0.isEmpty());
check_equals(typeof(r0.width), 'object');
check_equals(typeof(r0.height), 'null');

r0 = new Rectangle(0, 0, o1, o1);
check_equals(''+r0, '(x=0, y=0, w=2, h=2)');
check(r0.isEmpty()); // but it's still considered empty!
check_equals(typeof(r0.width), 'object');
check_equals(typeof(r0.height), 'object');

r0 = new Rectangle('string', 0, 2, 2);
check_equals(''+r0, '(x=string, y=0, w=2, h=2)');
check(!r0.isEmpty()); // just checks width and height
check_equals(r0.x, 'string');
check_equals(r0.y, 0);
check_equals(r0.width, 2);
check_equals(r0.height, 2);

r0.width = 40;
r0.height = 'string2';
r0.x = 32;
r0.y = -30;
check_equals(''+r0, '(x=32, y=-30, w=40, h=string2)');
check(delete r0.x);
check_equals(''+r0, '(x=undefined, y=-30, w=40, h=string2)');

//-------------------------------------------------------------
// Test bottom, right, left top
//-------------------------------------------------------------

r0 = new Rectangle('x', 'y', 'w', 'h');
check_equals(r0.left, 'x');
check_equals(r0.top, 'y');
check_equals(r0.right, 'xw');
check_equals(r0.bottom, 'yh');

r0.left = 10;
check_equals(r0.x, 10);
check_equals(r0.left, 10);
check_equals(r0.width, 'wNaN'); // w + old_x-10 ?
check_equals(r0.right, '10wNaN'); 

r0.right = 20;
check_equals(r0.x, 10);
check_equals(typeof(r0.width), 'number');
check_equals(r0.width, 10); // right-left

r0.top = 5;
check_equals(r0.y, 5);
check_equals(r0.bottom, '5hNaN');
r0.bottom = 10;
check_equals(r0.height, '5'); // 10-5

r0 = new Rectangle(10, 10, 20, 20);
r0.left = 15;
check_equals(r0.width, 15); // old width (20) + ( old left (10) - new left (15) )

//-------------------------------------------------------------
// Test bottomRight, topLeft
//-------------------------------------------------------------

r0 = new Rectangle('x', 'y', 'w', 'h');
check(r0.bottomRight instanceof flash.geom.Point);
check(r0.topLeft instanceof flash.geom.Point);
check_equals(''+r0.bottomRight, '(x=xw, y=yh)');
check_equals(''+r0.topLeft, '(x=x, y=y)');

ASSetPropFlags(r0, "bottomRight", 0, 4); // clear read-only (if any)
r0.bottomRight = 4;
check_equals(typeof(r0.bottomRight), 'object');

ASSetPropFlags(r0, "topLeft", 0, 4); // clear read-only (if any)
r0.topLeft = 4;
check_equals(typeof(r0.topLeft), 'object');

//-------------------------------------------------------------
// Test size
//-------------------------------------------------------------

r0 = new Rectangle('x', 'y', 'w', 'h');
check(r0.size instanceof flash.geom.Point);
check_equals(''+r0.size, '(x=w, y=h)');
ASSetPropFlags(r0, "size", 0, 4); // clear read-only (if any)
r0.size = 4;
check_equals(typeof(r0.topLeft), 'object');

//-------------------------------------------------------------
// Test clone
//-------------------------------------------------------------

r0 = new Rectangle('x', 'y', 'w', 'h');
r0.custom = 4;
r2 = r0.clone();
check_equals(r2.toString(), '(x=x, y=y, w=w, h=h)');
check_equals(r2.custom, undefined);

//-------------------------------------------------------------
// Test contains
//-------------------------------------------------------------

// TODO

//-------------------------------------------------------------
// Test containsPoint
//-------------------------------------------------------------

// TODO

//-------------------------------------------------------------
// Test containsRectangle
//-------------------------------------------------------------

// TODO

//-------------------------------------------------------------
// Test equals
//-------------------------------------------------------------

// TODO

//-------------------------------------------------------------
// Test inflate
//-------------------------------------------------------------

// TODO

//-------------------------------------------------------------
// Test inflatePoint
//-------------------------------------------------------------

// TODO

//-------------------------------------------------------------
// Test intersection
//-------------------------------------------------------------

// TODO

//-------------------------------------------------------------
// Test intersects
//-------------------------------------------------------------

// TODO

//-------------------------------------------------------------
// Test offset
//-------------------------------------------------------------

// TODO

//-------------------------------------------------------------
// Test offsetPoint
//-------------------------------------------------------------

// TODO

//-------------------------------------------------------------
// Test setEmpty
//-------------------------------------------------------------

// TODO

//-------------------------------------------------------------
// Test union
//-------------------------------------------------------------

// TODO

//-------------------------------------------------------------
// END OF TEST
//-------------------------------------------------------------

check_totals(96);

#endif // OUTPUT_VERSION >= 8
