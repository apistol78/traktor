/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_input_InputUtilities_H
#define traktor_input_InputUtilities_H

namespace traktor
{
	namespace input
	{

bool isInputAllowed();

bool getMouseRange(NSSize& outRange);
        
bool getMousePosition(NSPoint& outMousePositionGlobal, NSPoint& outMousePositionLocal);

bool getMouseCenterPosition(NSPoint& outCenterPosition);

	}
}

#endif	// traktor_input_InputUtilities_H
