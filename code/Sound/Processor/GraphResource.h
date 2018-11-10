#pragma once

#include "Core/Ref.h"
#include "Sound/ISoundResource.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class Graph;

/*! \brief
 */
class T_DLLCLASS GraphResource : public ISoundResource
{
	T_RTTI_CLASS;

public:
	GraphResource();

	GraphResource(
		const std::wstring& category,
		float gain,
		float presence,
		float presenceRate,
		float range,
		const Graph* graph
	);

	virtual Ref< Sound > createSound(resource::IResourceManager* resourceManager, const db::Instance* resourceInstance) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	const Graph* getGraph() const { return m_graph; }

private:
	std::wstring m_category;
	float m_gain;
	float m_presence;
	float m_presenceRate;
	float m_range;
	Ref< const Graph > m_graph;
};

	}
}