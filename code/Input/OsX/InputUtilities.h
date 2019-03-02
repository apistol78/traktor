#pragma once

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

