#ifndef traktor_input_InputValue_H
#define traktor_input_InputValue_H

#include "Core/Config.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

/*! \brief Input value from sources.
 * \ingroup Input
 *
 * A value read from a value source.
 *
 * \fixme
 * This class should be extended
 * to contain necessary meta information
 * about the value such as which device, category
 * etc.
 */
class T_DLLCLASS InputValue
{
public:
	InputValue(float value = 0.0f);
	
	void set(float value);
	
	float get() const;
	
private:
	float m_value;
};
	
	}
}

#endif	// traktor_input_InputValue_H
