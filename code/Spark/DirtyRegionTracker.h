#pragma once

#include "Core/Object.h"
#include "Core/Math/Aabb2.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Matrix33;

	namespace spark
	{

class CharacterInstance;

/*! \brief
 * \ingroup Spark
 */
class T_DLLCLASS DirtyRegionTracker : public Object
{
	T_RTTI_CLASS;

public:
	DirtyRegionTracker();

	bool update(CharacterInstance* movieInstance, Aabb2& outDirtyRegion);

private:
	struct State : public RefCountImpl< IRefCount >
	{
		Aabb2 bounds;
		bool visible;

		State();

		virtual ~State();
	};

	int32_t m_stateCount;

	void calculate(CharacterInstance* characterInstance, const Matrix33& transform, bool visible, Aabb2& outDirtyRegion);
};

	}
}
