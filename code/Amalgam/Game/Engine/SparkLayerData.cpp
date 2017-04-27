/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Amalgam/Game/IEnvironment.h"
#include "Amalgam/Game/Engine/SparkLayer.h"
#include "Amalgam/Game/Engine/SparkLayerData.h"
#include "Core/Serialization/ISerializer.h"
#include "Render/ImageProcess/ImageProcessSettings.h"
#include "Spark/SpriteData.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.amalgam.SparkLayerData", LayerData::Version, SparkLayerData, LayerData)

SparkLayerData::SparkLayerData()
:	m_background(0, 0, 0, 0)
{
}

Ref< Layer > SparkLayerData::createInstance(Stage* stage, IEnvironment* environment) const
{
	resource::IResourceManager* resourceManager = environment->getResource()->getResourceManager();
	resource::Proxy< spark::SpriteData > sprite;
	resource::Proxy< render::ImageProcessSettings > imageProcess;

	// Bind proxies to resource manager.
	if (!resourceManager->bind(m_sprite, sprite))
		return 0;

	// Bind optional post processing.
	if (m_imageProcess)
	{
		if (!resourceManager->bind(m_imageProcess, imageProcess))
			return 0;
	}

	// Create layer instance.
	return new SparkLayer(
		stage,
		m_name,
		m_permitTransition,
		environment,
		sprite,
		imageProcess,
		m_background
	);
}

void SparkLayerData::serialize(ISerializer& s)
{
	LayerData::serialize(s);
	s >> resource::Member< spark::SpriteData >(L"sprite", m_sprite);
	s >> resource::Member< render::ImageProcessSettings >(L"imageProcess", m_imageProcess);
	s >> Member< Color4ub >(L"background", m_background);
}

	}
}
