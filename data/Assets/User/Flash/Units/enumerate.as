 // 
//   Copyright (C) 2007 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modchecky
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
// along with this program; check not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fcheckth Floor, Boston, MA  02110-1301  USA
//

/*
 *  Zou Lunkai, zoulunkai@gmail.com
 *
 *  Test ActionEnumerate/2
 */


rcsid="$Id: enumerate.as,v 1.10 2008/03/11 19:31:48 strk Exp $";
#include "check.as"

#if OUTPUT_VERSION > 5

//
// enumerate characters of a sprite
//
{
  // Create a recorder and init it.
  recorder =  new Array(10);
  for(i=0; i<10; i++){
    recorder['mc'+i] = 0;
  }

  // Create six characters
  for(i=0; i<6; i++){
    _root.createEmptyMovieClip('mc'+i, i+1);
  }

  // Use the recorder to record the enumerated results in _root
  i = 0;
  for (var j in _root){
    recorder[j.toString()] = _root[j];
    i++;
  } 

  // Test what we got in the above step. Tests show that characters were also got enumerated!
  check_equals(typeof(recorder['mc0']), 'movieclip');
  check_equals(typeof(recorder['mc1']), 'movieclip');
  check_equals(typeof(recorder['mc2']), 'movieclip');
  check_equals(typeof(recorder['mc3']), 'movieclip');  
  check_equals(typeof(recorder['mc4']), 'movieclip');
  check_equals(typeof(recorder['mc5']), 'movieclip');
  check_equals(_root.hasOwnProperty('mc0'), false);
  check_equals(_root.hasOwnProperty('mc1'), false);
  check_equals(_root.hasOwnProperty('mc2'), false);
  check_equals(_root.hasOwnProperty('mc3'), false);
  check_equals(_root.hasOwnProperty('mc4'), false);
  check_equals(_root.hasOwnProperty('mc5'), false);
  delete recorder;
}

//
// enumerate properties of an AS object
//
{
  recorder =  new Array(10);
  
  // initialize the recorder
  for(i=0; i<10; i++){
    recorder['x'+i] = 0;
  }

  obj = new Object();
  obj.x1 = function () {};
  obj.x2 = 1;
  obj.x3 = Array();

  i = 0;
  for (var j in obj){
    // equivalent to recorder[j.toString()] = obj[j.toString()];
    recorder[j.toString()] = obj[j];
    i++;
  } 
  
  check_equals(typeof(recorder['x1']), 'function');
  check_equals(typeof(recorder['x2']), 'number');
  check_equals(typeof(recorder['x3']), 'object');
  check_equals(obj.hasOwnProperty('x1'), true);
  check_equals(obj.hasOwnProperty('x2'), true);
  check_equals(obj.hasOwnProperty('x3'), true);
  delete recorder;
}

#endif  // OUTPUT_VERSION > 5
totals();
