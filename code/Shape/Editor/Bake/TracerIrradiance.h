#pragma once

#include "Core/Guid.h"
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Math/Aabb3.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
    namespace shape
    {

class T_DLLCLASS TracerIrradiance : public Object
{
    T_RTTI_CLASS;

public:
    TracerIrradiance(
		const std::wstring& name,
        const Guid& irradianceGridId,
		const Aabb3& boundingBox
    );

	const std::wstring& getName() const { return m_name; }

    const Guid& getIrradianceGridId() const { return m_irradianceGridId; }

	const Aabb3& getBoundingBox() const { return m_boundingBox; }

private:
	std::wstring m_name;
	Guid m_irradianceGridId;
	Aabb3 m_boundingBox;
};

    }
}