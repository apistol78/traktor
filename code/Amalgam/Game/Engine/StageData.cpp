#include "Amalgam/Game/IEnvironment.h"
#include "Amalgam/Game/Engine/LayerData.h"
#include "Amalgam/Game/Engine/Stage.h"
#include "Amalgam/Game/Engine/StageData.h"
#include "Core/Class/IRuntimeClass.h"
#include "Core/Log/Log.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Database/Database.h"
#include "Render/IRenderSystem.h"
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Resource/ResourceBundle.h"
#include "Script/IScriptContext.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.amalgam.StageData", 8, StageData, ISerializable)

StageData::StageData()
:	m_fadeRate(1.5f)
{
}

Ref< Stage > StageData::createInstance(IEnvironment* environment, const Object* params) const
{
	render::IRenderSystem* renderSystem = environment->getRender()->getRenderSystem();
	resource::IResourceManager* resourceManager = environment->getResource()->getResourceManager();
	resource::Proxy< IRuntimeClass > clazz;
	resource::Proxy< script::IScriptContext > script;
	resource::Proxy< render::Shader > shaderFade;

#if !defined(_DEBUG)
	// Load resource bundle.
	if (m_resourceBundle.isNotNull())
	{
		bool skipPreload = environment->getSettings()->getProperty< PropertyBoolean >(L"Amalgam.SkipPreloadResources", false);
		if (!skipPreload)
		{
			uint32_t preloadLimit = environment->getSettings()->getProperty< PropertyInteger >(L"Amalgam.SkipPreloadLimit", 768) * 1024 * 1024;

			// Get amount of dedicated video memory; we cannot preload if too little amount of memory available or unknown vendor.
			render::RenderSystemInformation rsi;
			renderSystem->getInformation(rsi);
			if (
				(rsi.vendor == render::AvtNVidia || rsi.vendor == render::AvtAMD) &&
				rsi.dedicatedMemoryTotal >= preloadLimit
			)
			{
				Ref< const resource::ResourceBundle > resourceBundle = environment->getDatabase()->getObjectReadOnly< resource::ResourceBundle >(m_resourceBundle);
				if (resourceBundle)
					resourceManager->load(resourceBundle);
			}
			else
				log::warning << L"Pre-loading of resources skipped due to limited graphics adapter." << Endl;
		}
		else
			log::warning << L"Pre-loading of resources ignored" << Endl;
	}
#endif

	// Bind proxies to resource manager.
	if (m_class && !resourceManager->bind(m_class, clazz))
		return 0;
	if (m_script && !resourceManager->bind(m_script, script))
		return 0;
	if (m_shaderFade && !resourceManager->bind(m_shaderFade, shaderFade))
		return 0;

	// Create layers.
	Ref< Stage > stage = new Stage(m_name, environment, clazz, script, shaderFade, m_fadeRate, m_transitions, params);
	for (RefArray< LayerData >::const_iterator i = m_layers.begin(); i != m_layers.end(); ++i)
	{
		Ref< Layer > layer = (*i)->createInstance(stage, environment);
		if (!layer)
			return 0;

		stage->addLayer(layer);
	}

	return stage;
}

void StageData::serialize(ISerializer& s)
{
	if (s.getVersion() >= 7)
		s >> Member< std::wstring >(L"name", m_name);

	if (s.getVersion() >= 6)
		s >> Member< Guid >(L"inherit", m_inherit, AttributeType(type_of< StageData >()));

	s >> MemberRefArray< LayerData >(L"layers", m_layers);

	if (s.getVersion() >= 8)
		s >> resource::Member< IRuntimeClass >(L"class", m_class);

	s >> resource::Member< script::IScriptContext >(L"script", m_script);

	if (s.getVersion() >= 4)
		s >> resource::Member< render::Shader >(L"shaderFade", m_shaderFade);
	else
	{
		const resource::Id< render::Shader > c_shaderFade(Guid(L"{DC104971-11AE-5743-9AB1-53B830F74391}"));
		m_shaderFade = c_shaderFade;
	}

	if (s.getVersion() >= 5)
		s >> Member< float >(L"fadeRate", m_fadeRate, AttributeRange(0.1f));

	s >> MemberStlMap< std::wstring, Guid >(L"transitions", m_transitions);

	if (s.getVersion() >= 1)
		s >> Member< Guid >(L"resourceBundle", m_resourceBundle, AttributeType(type_of< resource::ResourceBundle >()));

	if (s.getVersion() == 2)
	{
		Guid dummy;
		s >> Member< Guid >(L"localizationDictionary", dummy);
	}
}

	}
}
