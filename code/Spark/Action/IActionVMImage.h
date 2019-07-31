#pragma once

#include "Core/Serialization/ISerializable.h"

namespace traktor
{
	namespace spark
	{

class ActionFrame;

/*! \brief ActionScript virtual machine image interface.
 * \ingroup Spark
 */
class IActionVMImage : public ISerializable
{
	T_RTTI_CLASS;

public:
	/*! \brief Execute image. */
	virtual void execute(ActionFrame* frame) const = 0;
};

	}
}

