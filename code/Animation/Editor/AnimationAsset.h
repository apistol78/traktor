#pragma once

#include "Core/Guid.h"
#include "Core/Math/Vector4.h"
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

/*! Animation asset.
 * \ingroup Animation
 */
class T_DLLCLASS AnimationAsset : public editor::Asset
{
	T_RTTI_CLASS;

public:
	AnimationAsset();

	virtual void serialize(ISerializer& s) override final;

	const Guid& getSkeleton() const { return m_skeleton; }

	const std::wstring& getTake() const { return m_take; }

	const Vector4& getTranslate() const { return m_translate; }

	float getScale() const { return m_scale; }

private:
	Guid m_skeleton;
	std::wstring m_take;
	float m_scale;
	Vector4 m_translate;
};

	}
}
