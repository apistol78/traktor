#ifndef traktor_spark_IComponentInstance_H
#define traktor_spark_IComponentInstance_H

#include "Core/Object.h"

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

/*! \brief Character component instance.
 * \ingroup Spark
 */
class T_DLLCLASS IComponentInstance : public Object
{
	T_RTTI_CLASS;

public:
	virtual void update() = 0;

	/*! \name Events */
	//@{

	virtual void eventKey(wchar_t unicode) = 0;

	virtual void eventKeyDown(int32_t keyCode) = 0;

	virtual void eventKeyUp(int32_t keyCode) = 0;

	virtual void eventMouseDown(int32_t x, int32_t y, int32_t button) = 0;

	virtual void eventMouseUp(int32_t x, int32_t y, int32_t button) = 0;

	virtual void eventMouseMove(int32_t x, int32_t y, int32_t button) = 0;

	virtual void eventMouseWheel(int32_t x, int32_t y, int32_t delta) = 0;

	virtual void eventViewResize(int32_t width, int32_t height) = 0;

	//@}
};

	}
}

#endif	// traktor_spark_IComponentInstance_H
