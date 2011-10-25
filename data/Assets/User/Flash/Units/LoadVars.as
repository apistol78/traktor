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


// Test case for LoadVars ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf


rcsid="$Id: LoadVars.as,v 1.29 2008/04/16 23:07:15 strk Exp $";
#include "check.as"

#if OUTPUT_VERSION < 6

xcheck_equals(typeof(LoadVars), 'function');

var loadvarsObj = new LoadVars;

// test the LoadVars constuctor
xcheck_equals (typeof(loadvarsObj), 'object');

check_totals(2);

#else // OUTPUT_VERSION >= 6

check_equals(typeof(LoadVars), 'function');

var loadvarsObj = new LoadVars;

// test the LoadVars constuctor
check_equals (typeof(loadvarsObj), 'object');

// test the LoadVars::addrequestheader method
check (LoadVars.prototype.hasOwnProperty('addRequestHeader'));
check_equals (typeof(loadvarsObj.addRequestHeader), 'function');

// test the LoadVars::decode method
check (LoadVars.prototype.hasOwnProperty('decode'));
check_equals (typeof(loadvarsObj.decode), 'function');
ret = loadvarsObj.decode("ud3=3&ud2=2");
check_equals(typeof(ret), 'undefined');
check_equals (loadvarsObj.ud3, 3);
check_equals (loadvarsObj.ud2, 2);
loadvarsObj.decode("ud4=4&ud2=20");
check_equals (loadvarsObj.ud4, 4);
check_equals (loadvarsObj.ud3, 3);
check_equals (loadvarsObj.ud2, 20);
ret = loadvarsObj.decode();
check_equals( typeof(ret), 'boolean');
check_equals( ret, false );
ret = loadvarsObj.decode("");
check_equals( typeof(ret), 'undefined');
o = {};
ret = loadvarsObj.decode(o);
check_equals( typeof(ret), 'undefined');
o.toString = function() { return "ud5=5"; };
ret = loadvarsObj.decode(o);
check_equals( typeof(ret), 'undefined');
check_equals (loadvarsObj.ud5, 5);
bomstarting="﻿ud6=6&ud7=7";
ret = loadvarsObj.decode(bomstarting);
check_equals( typeof(ret), 'undefined');
check_equals (loadvarsObj.ud6, undefined);
check_equals (loadvarsObj['﻿ud6'], 6);
check_equals (loadvarsObj.ud7, 7);

// test the LoadVars::getbytesloaded method
check (LoadVars.prototype.hasOwnProperty('getBytesLoaded'));
check_equals (typeof(loadvarsObj.getBytesLoaded), 'function');

// test the LoadVars::getbytestotal method
check (LoadVars.prototype.hasOwnProperty('getBytesTotal'));
check_equals (typeof(loadvarsObj.getBytesTotal), 'function');

// test the LoadVars::load method
check (LoadVars.prototype.hasOwnProperty('load'));
check_equals (typeof(loadvarsObj.load), 'function');

// test the LoadVars::send method
check (LoadVars.prototype.hasOwnProperty('send'));
check_equals (typeof(loadvarsObj.send), 'function');

// test the LoadVars::sendandload method
check (LoadVars.prototype.hasOwnProperty('sendAndLoad'));
check_equals (typeof(loadvarsObj.sendAndLoad), 'function');

// test the LoadVars::tostring method
check (LoadVars.prototype.hasOwnProperty('toString'));
check_equals (typeof(loadvarsObj.toString), 'function');

// test the LoadVars::tostring method
check (!LoadVars.prototype.hasOwnProperty('valueOf'));
check_equals (loadvarsObj.valueOf, Object.prototype.valueOf);
check_equals (typeof(loadvarsObj.valueOf), 'function');

// test the LoadVars::onData method
check (LoadVars.prototype.hasOwnProperty('onData'));
check_equals (typeof(loadvarsObj.onData), 'function');

// test the LoadVars::onLoad method
check (LoadVars.prototype.hasOwnProperty('onLoad'));
check_equals (typeof(loadvarsObj.onLoad), 'function');

// test the LoadVars::loaded member
check (!LoadVars.prototype.hasOwnProperty('loaded'));
check_equals (typeof(loadvarsObj.loaded), 'undefined');

//--------------------------------------------------------------------------
// Test LoadVars::load()
//--------------------------------------------------------------------------

varsloaded = 0;
datareceived = 0;
//var1 = undefined;
//var2 = undefined;
loadvarsObj.onLoad = function(success) {
	varsloaded++;
	note("LoadVars.onLoad called "+varsloaded+". "
		+"Bytes loaded: "+loadvarsObj.getBytesLoaded()
		+"/"+loadvarsObj.getBytesTotal());

	//delete loadvarsObj; // this to test robustness

	check_equals (loadvarsObj.getBytesTotal(), loadvarsObj.getBytesLoaded());
	check (loadvarsObj.getBytesLoaded() > 10);
	check_equals (this, loadvarsObj);
	check_equals(arguments.length, 1);
	check_equals(typeof(success), 'boolean');
	check_equals(success, true);
	check_equals(this.loaded, success);
	check_equals(typeof(this.loaded), 'boolean');
	this.loaded = 5;
	check_equals(typeof(this.loaded), 'number');

	check(varsloaded < 3);

	// onLoad is called after all vars have been called
	check_equals( loadvarsObj.getBytesLoaded(), loadvarsObj.getBytesTotal() );

	//for (var i in _root) { note("_root["+i+"] = "+_root[i]); }

	if ( varsloaded == 1 )
	{
		check_equals(loadvarsObj['var1'], 'val1');
		//check_equals(loadvarsObj['var1_check'], 'previous val1');
		check_equals(loadvarsObj['var2'], 'val2');
		//check_equals(loadvarsObj['v2_var1'], 'val1');
		//check_equals(loadvarsObj['v2_var2'], 'val2');

		// Gnash insists in looking for an ending & char !!		
		check_equals(loadvarsObj['var3'], 'val3\n');

		check_totals(72);

		play();
	}
};

// onData is called once with full parsed content.
loadvarsObj.onDataReal = loadvarsObj.onData;
loadvarsObj.onData = function(src) {
	check_equals (this, loadvarsObj);
	check_equals(typeof(this.loaded), 'boolean');
	check_equals(this.loaded, false);
	check_equals(arguments.length, 1);
	check_equals(typeof(src), 'string');
	check_equals(src.substr(0, 10), 'var1=val1&');
	check_equals(src.substr(loadvarsObj.getBytesTotal()-13), 'var3=val3\n');
	check_equals(datareceived, 0);
	datareceived++; // we expecte it to be called only once ?
	note("LoadVars.onData called ("+datareceived+"), byte loaded: "
		+loadvarsObj.getBytesLoaded()
		+"/"+loadvarsObj.getBytesTotal());
	this.onDataReal(src);
	//check_equals(loadvarsObj['var1'], 'val1');
	//check_equals(loadvarsObj['var2'], 'val2');
	//for (var i in _root) { note("_root["+i+"] = "+_root[i]); }
	//play();
};

loadvarsObj.var1 = "previous val1";

// We expect the loaded file to return this string:
//
// 	"var1=val1&var2=val2&"
//
// The final '&' char is important, and it must
// not start with a '?' char.
// 
check( loadvarsObj instanceOf LoadVars );
//check( loadvarsObj.sendAndLoad( 'http://localhost/vars.php', loadvarsObj ) );
check( loadvarsObj.load( MEDIA(vars.txt) ) );
check_equals(typeof(this.loaded), 'undefined');
//loadvarsObj.load( 'vars.cgi' );

check_equals( loadvarsObj.loaded, false );
//loadvars.Obj.loaded = true;
//check_equals( loadvarsObj.loaded, false );
check_equals(varsloaded, 0);
check_equals(loadvarsObj['var1'], 'previous val1'); // will be overridden
check_equals(loadvarsObj['var2'], undefined);
//delete loadvarsObj; // this to test robustness

stop();

#endif //  OUTPUT_VERSION >= 6

