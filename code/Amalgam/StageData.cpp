#include "Amalgam/IEnvironment.h"
#include "Amalgam/LayerData.h"
#include "Amalgam/Stage.h"
#include "Amalgam/StageData.h"
#include "Core/Log/Log.h"
#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Core/Serialization/MemberStl.h"
#include "Database/Database.h"
#include "I18N/Dictionary.h"
#include "I18N/I18N.h"
#include "Render/IRenderSystem.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Resource/ResourceBundle.h"
#include "Script/IScriptContext.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.amalgam.StageData", 2, StageData, ISerializable)

Ref< Stage > StageData::createInstance(amalgam::IEnvironment* environment, const Object* params) const
{
	render::IRenderSystem* renderSystem = environment->getRender()->getRenderSystem();
	resource::IResourceManager* resourceManager = environment->getResource()->getResourceManager();
	resource::Proxy< script::IScriptContext > script;

#if !defined(_DEBUG)
	// Load resource bundle.
	if (m_resourceBundle.isNotNull())
	{
		// Get amount of dedicated video memory; we cannot preload
		// if too little amount of memory available.
		render::RenderSystemInformation rsi;
		renderSystem->getInformation(rsi);

		if (rsi.dedicatedMemoryTotal > 128 * 1024 * 1024)
		{
			Ref< const resource::ResourceBundle > resourceBundle = environment->getDatabase()->getObjectReadOnly< resource::ResourceBundle >(m_resourceBundle);
			if (resourceBundle)
				resourceManager->load(resourceBundle);
		}
		else
			log::warning << L"Pre-loading of resources skipped due to poor graphics card" << Endl;
	}
#endif

	// Load localization dictionary.
	if (m_localizationDictionary.isNotNull())
	{
		Ref< const i18n::Dictionary > dictionary = environment->getDatabase()->getObjectReadOnly< i18n::Dictionary >(m_localizationDictionary);
		if (dictionary)
			i18n::I18N::getInstance().appendDictionary(dictionary, true);
	}

	// Bind proxies to resource manager.
	if (m_script && !resourceManager->bind(m_script, script))
		return 0;

	// Create layers.
	Ref< Stage > stage = new Stage(environment, script, m_transitions, params);
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
	s >> MemberRefArray< LayerData >(L"layers", m_layers);
	s >> resource::Member< script::IScriptContext >(L"script", m_script);
	s >> MemberStlMap< std::wstring, Guid >(L"transitions", m_transitions);

	if (s.getVersion() >= 1)
		s >> Member< Guid >(L"resourceBundle", m_resourceBundle, AttributeType(type_of< resource::ResourceBundle >()));

	if (s.getVersion() >= 2)
		s >> Member< Guid >(L"localizationDictionary", m_localizationDictionary, AttributeType(type_of< i18n::Dictionary >()));
}

	}
}
