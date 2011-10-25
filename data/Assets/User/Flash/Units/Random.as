// 
//   Copyright (C) 2007 Free Software Foundation, Inc.
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
//
// Tests for random behaviour. This tests only the randomness of the output
// and the range of results. Tests for Math.random() actionscript correctness
// are in Math.as.
//
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

// For obvious reasons, tests of randomness can fail even if there is
// nothing wrong (though with carefully-designed pseudo-randomness this
// is very unlikely). The chance of a failure is very small if Gnash
// is working correctly.

rcsid="$Id: Random.as,v 1.5 2008/03/11 19:31:48 strk Exp $";
#include "check.as"

// Number of random numbers to generate.
var max = 1000;

/*
Tests for random(n).

This AS function should return an integer from 0 up to but not
including n.
*/

// With n = integer (how it should be used...)
var tally = new Array();

for (var i = 0; i < max; i++)
{
	rnd = random(4);
	if (typeof(tally[rnd]) == "undefined") { tally[rnd] = 0; }
	tally[rnd] += 1;
}

// Check proportion of each number exceeds 10%; should be about
// 25%.
check (tally[0] > (max / 10));
check (tally[1] > (max / 10));
check (tally[2] > (max / 10));
check (tally[3] > (max / 10));

check_equals(typeof(tally[4]), "undefined"); // Should not exist

// With n = non-integer

var tally = new Array();
for (var i = 0; i < max; i++)
{
	rnd = random(4.5);
	if (typeof(tally[rnd]) == "undefined") { tally[rnd] = 0; }
	tally[rnd] += 1;
}

// Check proportion of each number exceeds 10%; should be about
// 25%.
check (tally[0] > (max / 10));
check (tally[1] > (max / 10));
check (tally[2] > (max / 10));
check (tally[3] > (max / 10));

check_equals(typeof(tally[4]), "undefined"); // Should not exist

// With n = negative number

var tally = new Array();
for (var i = 0; i < max / 5; i++)
{
	rnd = random(-1);
	if (typeof(tally[rnd]) == "undefined") { tally[rnd] = 0; }
	tally[rnd] += 1;
}

check_equals (tally[0], max / 5 );
check_equals(typeof(tally[1]), "undefined"); // Should not exist

/*

Tests for Math.random()

Returns double n where 0 <= n < 1.

*/


// Note: test also relies on Math.round()!
var tally = new Array();

for (i = 0; i < max; i++)
{
	rnd = Math.round(Math.random() * 10 + 0.5);
	if (typeof(tally[rnd]) == "undefined") { tally[rnd] = 0; }
	tally[rnd] += 1;
}

check_equals(typeof(tally[0]), "undefined"); // Should not exist

check (tally[1] > (max / 20));

check (tally[2] > (max / 20));

check (tally[3] > (max / 20));

check (tally[4] > (max / 20));

check (tally[5] > (max / 20));

check (tally[7] > (max / 20));

check (tally[8] > (max / 20));

check (tally[9] > (max / 20));

check (tally[10] > (max / 20));

check_equals(typeof(tally[11]), "undefined"); // Should not exist

/* End of tests */
check_totals(24);
