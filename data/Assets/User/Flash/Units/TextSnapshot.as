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

// Test case for TextSnapshot ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf


rcsid="$Id: TextSnapshot.as,v 1.12 2008/03/11 19:31:48 strk Exp $";
#include "check.as"

#if OUTPUT_VERSION > 5

check_equals ( typeof(TextSnapshot), 'function' );

var textsnapshotObj = new TextSnapshot;

// test the TextSnapshot constuctor
check_equals ( typeof(textsnapshotObj), 'object' );

// test the TextSnapshot::findtext method
check_equals (typeof(textsnapshotObj.findText), 'function');

// test the TextSnapshot::getcount method
check_equals (typeof(textsnapshotObj.getCount), 'function');

// test the TextSnapshot::getselected method
check_equals (typeof(textsnapshotObj.getSelected), 'function');

// test the TextSnapshot::getselectedtext method
check_equals (typeof(textsnapshotObj.getSelectedText), 'function');

// test the TextSnapshot::gettext method
check_equals (typeof(textsnapshotObj.getText), 'function');

// test the TextSnapshot::hittesttextnearpos method
check_equals (typeof(textsnapshotObj.hitTestTextNearPos), 'function');

// test the TextSnapshot::setselectcolor method
check_equals (typeof(textsnapshotObj.setSelectColor), 'function');

// test the TextSnapshot::setselected method
check_equals (typeof(textsnapshotObj.setSelected), 'function');

#endif // OUTPUT_VERSION > 5
totals();
