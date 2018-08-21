/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#pragma once

#include "Core/Object.h"
#include "Core/Math/Aabb2.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Matrix33;

	namespace flash
	{

class CharacterInstance;
	
/*! \brief
 * \ingroup Flash
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
