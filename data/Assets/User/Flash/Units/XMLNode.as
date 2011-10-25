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

// Test case for XML ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf


rcsid="$Id: XMLNode.as,v 1.17 2008/03/11 19:31:48 strk Exp $";
#include "check.as"
//#include "dejagnu.as"

var doc = new XML();

check(doc);
var textnode = doc.createTextNode("text content");
check_equals(typeOf(textnode), 'object');

// test the XMLNode constuctor
//dejagnu(node, "XMLNode::XMLNode()");

//note("Test the existance of all the methods");

//check_equals(typeOf(myXML.createElement), 'function');



check(textnode.appendChild);
check(textnode.cloneNode);
check(textnode.hasChildNodes);
check(textnode.insertBefore);
check(textnode.removeNode);
check(textnode.toString);

//note("Now test the functionality of the properties");

textnode.nodeName = "foo";
check_equals(textnode.nodeName, "foo");

check_equals (textnode.nodeValue, "text content");
textnode.nodeValue = "bar";
check_equals (textnode.nodeValue, "bar");

// The read only properties. These should all return NULL, because
// there are no children.
check_equals(textnode.hasChildNodes(), false);
check_equals(typeof(textnode.firstChild), 'null');
check_equals(typeof(textnode.lastChild), 'null');
check_equals(typeof(textnode.parentNode), 'null');
check_equals(typeof(textnode.nextSibling), 'null');
check_equals(typeof(textnode.previousSibling), 'null');

//note("Now test the functionality of the methods");


var node1 = doc.createElement("node1");
var node2 = doc.createElement("node2");
var textnode1 = doc.createTextNode("first text node");
var textnode2 = doc.createTextNode("second text node");
check_equals(textnode1.nodeType, 3);
node1.appendChild(textnode1);
node2.appendChild(textnode2);
node1.appendChild(node2);
check_equals(node1.hasChildNodes(), true);

check_equals(node1.firstChild.nodeValue, "first text node");
check_equals(typeof(node1.lastChild.nodeValue), 'null');
check_equals(node2.lastChild.toString(), "second text node"); 

var node3 = doc.createElement("node3");
var textnode3 = doc.createTextNode("third text node");
node3.appendChild(textnode3);
node1.appendChild(node3);

// trace(node1.toString());
trace("===========================================");

// trace(node1.firstChild.nodeValue);
// trace(node1.lastChild.nodeValue);
check_equals(node1.firstChild.nodeValue, "first text node");
check_equals(typeof(node1.lastChild.nodeValue), 'null');

trace(node1.lastChild.previousSibling);
trace(node1.firstChild.nextSibling);

check_equals(typeof(node1.firstChild.nodeName), "null");
check_equals(node1.lastChild.nodeName, "node3");

check_equals(node2.previousSibling.nodeValue, "first text node");

// TODO: test removeNode, insertNode

// for (var aNode = node1.firstChild; node1 != null; aNode = node1.nextSibling) {
//     trace(aNode);
// }


check_totals(27);
