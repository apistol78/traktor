
rcsid="$Id: delete.as,v 1.14 2008/03/11 19:31:48 strk Exp $";
#include "check.as"

anObject = new Object();
check(anObject != undefined);

anObjectBackup = anObject;
check(delete anObject);
check_equals(typeof(anObject), 'undefined'); 
check_equals(typeof(anObjectBackup), 'object'); 
check(!delete noObject);

//
// Scoped delete (see bug #18482)
//

var anotherObject = new Object();
check(anotherObject);
anotherObject.a = "anotherObject.a";
a = "a";
b = "b";
_global.a = "_global.a";
anotherObject.b = "anotherObject.b (protected)";
ASSetPropFlags(anotherObject, "b", 2); // protect b
with(anotherObject)
{
	check_equals(a, "anotherObject.a");
	check_equals(b, "anotherObject.b (protected)");
	check(!delete b); // protected from deletion !
	check_equals(b, "anotherObject.b (protected)");
	check(delete a);
	check_equals(a, "a");
	check(delete a);

#if OUTPUT_VERSION > 5
	check_equals(a, "_global.a");
	check(delete a);
#else
	check_equals(a, undefined);
	check(! delete a);
#endif

	check_equals(a, undefined);
	check(!delete a);
}

//
// Deleting a user function's prototype
//

function func() {};
func.prototype.appended_value = 4;
check_equals(typeof(func.prototype), 'object');
check_equals(func.prototype.appended_value, 4);
//protoback = MovieClip.prototype;
check(!delete func.prototype);
check_equals(typeof(func.prototype), 'object');
check_equals(func.prototype.appended_value, 4);

//
// Deleting an object's member
//

obj = new Object;
obj.a = 1;
check_equals(obj.a, 1);
check(delete obj.a);
check_equals(typeof(obj.a), 'undefined');
check(!delete obj.a);
check(!delete unexistent.a);

// TODO: try other malformed ActionDelete calls

// TODO: test deletion of variables referenced by path (slash-based or dot-based)
//       make sure to test use of 'this' here too !
totals();
