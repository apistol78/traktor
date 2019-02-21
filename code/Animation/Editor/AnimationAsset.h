#pragma once

#include "Editor/Asset.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace animation
	{

/*! \brief
 * \ingroup Animation
 */
class T_DLLCLASS AnimationAsset : public editor::Asset
{
	T_RTTI_CLASS;

public:
	AnimationAsset();

	virtual void serialize(ISerializer& s) override final;

	const std::wstring& getTake() const { return m_take; }

	float getScale() const { return m_scale; }

private:
	std::wstring m_take;
	float m_scale;
};

	}
}
