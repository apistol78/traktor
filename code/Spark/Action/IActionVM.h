#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spark
	{

class IActionVMImage;
class SwfReader;

/*! ActionScript virtual machine interface.
 * \ingroup Spark
 */
class T_DLLCLASS IActionVM : public Object
{
	T_RTTI_CLASS;

public:
	/*! Load executable image from binary representation of bytecode. */
	virtual Ref< const IActionVMImage > load(SwfReader& swf) const = 0;
};

	}
}

