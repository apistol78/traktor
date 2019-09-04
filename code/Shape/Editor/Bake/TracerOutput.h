#pragma once

#include "Core/Guid.h"
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Math/Transform.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
    namespace model
    {

class Model;

    }

    namespace shape
    {

class T_DLLCLASS TracerOutput : public Object
{
    T_RTTI_CLASS;

public:
    TracerOutput(
		const std::wstring& name,
		int32_t priority,
        const model::Model* model,
		const Transform& transform,
        const Guid& lightmapId,
		int32_t lightmapSize
    );

	const std::wstring& getName() const { return m_name; }

	int32_t getPriority() const { return m_priority; }

    const model::Model* getModel() const { return m_model; }

	const Transform& getTransform() const { return m_transform; }

    const Guid& getLightmapId() const { return m_lightmapId; }

	int32_t getLightmapSize() const { return m_lightmapSize; }

private:
	std::wstring m_name;
	int32_t m_priority;
	Ref< const model::Model > m_model;
	Transform m_transform;
	Guid m_lightmapId;
	int32_t m_lightmapSize;
};

    }
}