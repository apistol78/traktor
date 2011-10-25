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


// Test case for Error ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf


rcsid="$Id: Error.as,v 1.14 2008/06/17 08:45:36 bwy Exp $";
#include "check.as"

#if OUTPUT_VERSION > 5
check(Error.prototype.hasOwnProperty("name"));
check(Error.prototype.hasOwnProperty("toString"));
check(Error.prototype.hasOwnProperty("message"));
#endif

xcheck_equals(typeof(Error.prototype.message), "string");
xcheck_equals(typeof(Error.prototype.name), "string");

var errorObj = new Error;

// test the Error constuctor
check_equals (typeof(errorObj), 'object');

// test the Error::tostring method
check_equals (typeof(errorObj.toString), 'function');


e = new Error;
check_equals(e.toString(), "Error");
check_equals(e.message, "Error");
check_equals(e.name, "Error");

e = new Error("NameOfError");
check_equals(e.toString(), "NameOfError");
check_equals(e.name, "Error");
check_equals(e.message, "NameOfError");


e = new Error(7.8898);
check_equals(e.toString(), "7.8898");
check_equals(e.name, "Error");
check_equals(e.message, "7.8898");


// Is there any sense in this?
e = new Error(new Color);
xcheck_equals(typeof(e.toString()), "object");
check_equals(e.toString().toString(), "[object Object]");
check_equals(e.name, "Error");
xcheck_equals(typeof(e.message), "object");

e.name = "ANewName";
check_equals(e.name, "ANewName");
e.message = "New message";
check_equals(e.message, "New message");


e = Error("NameOfSecondError");
xcheck_equals(typeof(e), "undefined");

totals();
