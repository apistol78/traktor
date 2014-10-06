#ifndef traktor_illuminate_IlluminateEntityData_H
#define traktor_illuminate_IlluminateEntityData_H

#include "Core/Guid.h"
#include "Core/RefArray.h"
#include "World/EntityData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ILLUMINATE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace illuminate
	{

class T_DLLCLASS IlluminateEntityData : public world::EntityData
{
	T_RTTI_CLASS;

public:
	IlluminateEntityData();

	void addEntityData(world::EntityData* entityData);

	void removeEntityData(world::EntityData* entityData);

	void removeAllEntityData();

	virtual void serialize(ISerializer& s);
	
	virtual void setTransform(const Transform& transform);

	const Guid& getSeedGuid() const { return m_seedGuid; }

	float getLumelsPerUnit() const { return m_lumelsPerUnit; }

	bool traceDirectLighting() const { return m_directLighting; }

	bool traceIndirectLighting() const { return m_indirectLighting; }

	int32_t getIndirectTraceSamples() const { return m_indirectTraceSamples; }

	int32_t getIndirectTraceIterations() const { return m_indirectTraceIterations; }

	int32_t getIndirectConvolveIterations() const { return m_indirectConvolveIterations; }

	bool highDynamicRange() const { return m_highDynamicRange; }

	bool compressLightMap() const { return m_compressLightMap; }

	const RefArray< world::EntityData >& getEntityData() const { return m_entityData; }

private:
	Guid m_seedGuid;
	float m_lumelsPerUnit;
	bool m_directLighting;
	bool m_indirectLighting;
	int32_t m_indirectTraceSamples;
	int32_t m_indirectTraceIterations;
	int32_t m_indirectConvolveIterations;
	bool m_highDynamicRange;
	bool m_compressLightMap;
	RefArray< world::EntityData > m_entityData;
};

	}
}

#endif	// game_IlluminateEntityData_H
