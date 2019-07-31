#pragma once

#include "Spark/Action/IActionVM.h"

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

/*! \brief ActionScript virtual machine 2.
 * \ingroup Spark
 *
 * Virtual machine to execute blocks of ActionScript
 * byte code.
 *
 * \sa http://www.adobe.com/content/dam/Adobe/en/devnet/actionscript/articles/avm2overview.pdf
 */
class T_DLLCLASS ActionVM2 : public IActionVM
{
	T_RTTI_CLASS;

public:
	virtual Ref< const IActionVMImage > load(SwfReader& swf) const override final;
};

	}
}

