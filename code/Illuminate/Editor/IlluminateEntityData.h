/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_illuminate_IlluminateEntityData_H
#define traktor_illuminate_IlluminateEntityData_H

#include "Core/Guid.h"
#include "Core/RefArray.h"
#include "Core/Io/Path.h"
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

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;
	
	virtual void setTransform(const Transform& transform) T_OVERRIDE T_FINAL;

	const Guid& getSeedGuid() const { return m_seedGuid; }

	bool traceOcclusion() const { return m_occlusion; }

	bool traceDirectLighting() const { return m_directLighting; }

	bool traceIndirectLighting() const { return m_indirectLighting; }

	float getLumelDensity() const { return m_lumelDensity; }

	float getPointLightRadius() const { return m_pointLightRadius; }

	int32_t getShadowSamples() const { return m_shadowSamples; }

	int32_t getProbeSamples() const { return m_probeSamples; }

	float getProbeCoeff() const { return m_probeCoeff; }

	float getProbeSpread() const { return m_probeSpread; }

	float getProbeShadowSpread() const { return m_probeShadowSpread; }

	int32_t getDirectConvolveRadius() const { return m_directConvolveRadius; }

	int32_t getIndirectTraceSamples() const { return m_indirectTraceSamples; }

	int32_t getIndirectTraceIterations() const { return m_indirectTraceIterations; }

	int32_t getIndirectConvolveRadius() const { return m_indirectConvolveRadius; }

	bool highDynamicRange() const { return m_highDynamicRange; }

	bool compressLightMap() const { return m_compressLightMap; }

	const RefArray< world::EntityData >& getEntityData() const { return m_entityData; }

private:
	Guid m_seedGuid;
	bool m_occlusion;
	bool m_directLighting;
	bool m_indirectLighting;
	float m_lumelDensity;
	float m_pointLightRadius;
	int32_t m_shadowSamples;
	int32_t m_probeSamples;
	float m_probeCoeff;
	float m_probeSpread;
	float m_probeShadowSpread;
	int32_t m_directConvolveRadius;
	int32_t m_indirectTraceSamples;
	int32_t m_indirectTraceIterations;
	int32_t m_indirectConvolveRadius;
	bool m_highDynamicRange;
	bool m_compressLightMap;
	RefArray< world::EntityData > m_entityData;
};

	}
}

#endif	// game_IlluminateEntityData_H
