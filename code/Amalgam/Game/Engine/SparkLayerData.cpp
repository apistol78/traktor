#include "Amalgam/Game/IEnvironment.h"
#include "Amalgam/Game/Engine/SparkLayer.h"
#include "Amalgam/Game/Engine/SparkLayerData.h"
#include "Core/Serialization/ISerializer.h"
#include "Spark/Sprite.h"
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
	resource::Proxy< spark::Sprite > sprite;

	// Bind proxies to resource manager.
	if (!resourceManager->bind(m_sprite, sprite))
		return 0;

	// Create layer instance.
	return new SparkLayer(
		stage,
		m_name,
		m_permitTransition,
		environment,
		sprite,
		m_background
	);
}

void SparkLayerData::serialize(ISerializer& s)
{
	LayerData::serialize(s);
	s >> resource::Member< spark::Sprite >(L"sprite", m_sprite);
	s >> Member< Color4ub >(L"background", m_background);
}

	}
}
