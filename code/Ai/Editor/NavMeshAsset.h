#ifndef traktor_ai_NavMeshAsset_H
#define traktor_ai_NavMeshAsset_H

#include "Core/Guid.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AI_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ai
	{

/*! \brief Navigation mesh asset.
 * \ingroup AI
 */
class T_DLLCLASS NavMeshAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	NavMeshAsset();

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	friend class NavMeshPipeline;

	Guid m_source;
	float m_cellSize;
	float m_cellHeight;
	float m_agentSlope;
	float m_agentHeight;
	float m_agentClimb;
	float m_agentRadius;
	float m_maxEdgeLength;
	float m_maxSimplificationError;
	float m_minRegionSize;
	float m_mergeRegionSize;
	float m_detailSampleDistance;
	float m_detailSampleMaxError;
};

	}
}

#endif	// traktor_ai_NavMeshAsset_H
