/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_weather_SkyComponentData_H
#define traktor_weather_SkyComponentData_H

#include "Core/Math/Vector4.h"
#include "Resource/Id.h"
#include "World/IEntityComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WEATHER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace render
	{

class IRenderSystem;
class Shader;

	}

	namespace weather
	{

class SkyComponent;

class T_DLLCLASS SkyComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	SkyComponentData();

	Ref< SkyComponent > createComponent(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	const resource::Id< render::Shader >& getShader() const { return m_shader; }

private:
	resource::Id< render::Shader > m_shader;
	Vector4 m_sunDirection;
	float m_offset;
};

	}
}

#endif	// traktor_weather_SkyComponentData_H
