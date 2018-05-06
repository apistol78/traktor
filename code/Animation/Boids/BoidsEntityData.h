/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_animation_BoidsEntityData_H
#define traktor_animation_BoidsEntityData_H

#include "Core/Ref.h"
#include "World/EntityData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class IEntityBuilder;

	}

	namespace animation
	{

class BoidsEntity;

/*! \brief
 * \ingroup Animation
 */
class T_DLLCLASS BoidsEntityData : public world::EntityData
{
	T_RTTI_CLASS;

public:
	BoidsEntityData();

	Ref< BoidsEntity > createEntity(const world::IEntityBuilder* builder) const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	const Ref< world::EntityData >& getBoidEntityData() const { return m_boidEntityData; }

private:
	Ref< world::EntityData > m_boidEntityData;
	uint32_t m_boidCount;
	Vector4 m_spawnPositionDiagonal;
	Vector4 m_spawnVelocityDiagonal;
	Vector4 m_constrain;
	float m_followForce;
	float m_repelDistance;
	float m_repelForce;
	float m_matchVelocityStrength;
	float m_centerForce;
	float m_maxVelocity;
};

	}
}

#endif	// traktor_animation_BoidsEntityData_H
