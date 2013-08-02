#ifndef traktor_input_InputUtilities_H
#define traktor_input_InputUtilities_H

namespace traktor
{
	namespace input
	{

bool isInputAllowed();

bool getMouseRange(float& outMaxX, float& outMaxY);
        
bool getMousePosition(float& outX, float& outY);

bool getMouseCenterPosition(float& outX, float& outY);

	}
}

#endif	// traktor_input_InputUtilities_H
