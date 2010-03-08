#include "Core/Io/StringOutputStream.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPageSite.h"
#include "Editor/Settings.h"
#include "I18N/Text.h"
#include "Render/IRenderSystem.h"
#include "Render/Resource/ShaderFactory.h"
#include "Render/Resource/TextureFactory.h"
#include "Resource/ResourceManager.h"
#include "Spray/Effect.h"
#include "Spray/EffectFactory.h"
#include "Spray/EffectLayer.h"
#include "Spray/Editor/EffectEditorPage.h"
#include "Spray/Editor/EffectPreviewControl.h"
#include "Ui/Bitmap.h"
#include "Ui/Command.h"
#include "Ui/Container.h"
#include "Ui/MethodHandler.h"
#include "Ui/TableLayout.h"
#include "Ui/Events/CommandEvent.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarSeparator.h"
#include "Ui/Custom/Sequencer/SequencerControl.h"
#include "Ui/Custom/Sequencer/Sequence.h"
#include "Ui/Custom/Sequencer/Range.h"
#include "Ui/Custom/Splitter.h"

// Resources
#include "Resources/Playback.h"
#include "Resources/SceneEdit.h"
#include "Resources/EffectEdit.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.EffectEditorPage", EffectEditorPage, editor::IEditorPage)

EffectEditorPage::EffectEditorPage(editor::IEditor* editor)
:	m_editor(editor)
,	m_velocityVisible(false)
,	m_guideVisible(true)
{
}

bool EffectEditorPage::create(ui::Container* parent, editor::IEditorPageSite* site)
{
	render::IRenderSystem* renderSystem = m_editor->getStoreObject< render::IRenderSystem >(L"RenderSystem");
	if (!renderSystem)
		return false;

	m_site = site;
	T_ASSERT (m_site);

	Ref< db::Database > database = m_editor->getOutputDatabase();

	m_resourceManager = new resource::ResourceManager();
	m_resourceManager->addFactory(
		new render::TextureFactory(database, renderSystem, 0)
	);
	m_resourceManager->addFactory(
		new render::ShaderFactory(database, renderSystem)
	);
	m_resourceManager->addFactory(
		new EffectFactory(database)
	);

	activate();

	Ref< ui::Container > container = new ui::Container();
	container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0));

	m_toolToggleGuide = new ui::custom::ToolBarButton(i18n::Text(L"EFFECT_EDITOR_TOGGLE_GUIDE"), ui::Command(L"Effect.Editor.ToggleGuide"), 11, ui::custom::ToolBarButton::BsDefaultToggle);

	Ref< editor::Settings > settings = m_editor->getSettings();
	T_ASSERT (settings);

	m_guideVisible = settings->getProperty< editor::PropertyBoolean >(L"EffectEditor.ToggleGuide", m_guideVisible);
	m_toolToggleGuide->setToggled(m_guideVisible);

	m_toolBar = new ui::custom::ToolBar();
	m_toolBar->create(container);
	m_toolBar->addImage(ui::Bitmap::load(c_ResourcePlayback, sizeof(c_ResourcePlayback), L"png"), 6);
	m_toolBar->addImage(ui::Bitmap::load(c_ResourceSceneEdit, sizeof(c_ResourceSceneEdit), L"png"), 12);
	m_toolBar->addImage(ui::Bitmap::load(c_ResourceEffectEdit, sizeof(c_ResourceEffectEdit), L"png"), 1);
	m_toolBar->addItem(new ui::custom::ToolBarButton(i18n::Text(L"EFFECT_EDITOR_REWIND"), ui::Command(L"Effect.Editor.Rewind"), 0));
	m_toolBar->addItem(new ui::custom::ToolBarButton(i18n::Text(L"EFFECT_EDITOR_PLAY"), ui::Command(L"Effect.Editor.Play"), 1));
	m_toolBar->addItem(new ui::custom::ToolBarButton(i18n::Text(L"EFFECT_EDITOR_STOP"), ui::Command(L"Effect.Editor.Stop"), 2));
	m_toolBar->addItem(new ui::custom::ToolBarSeparator());
	m_toolBar->addItem(m_toolToggleGuide);
	m_toolBar->addItem(new ui::custom::ToolBarButton(i18n::Text(L"EFFECT_EDITOR_RANDOMIZE_SEED"), ui::Command(L"Effect.Editor.RandomizeSeed"), 18));

	m_toolBar->addClickEventHandler(ui::createMethodHandler(this, &EffectEditorPage::eventToolClick));

	Ref< ui::custom::Splitter > splitter = new ui::custom::Splitter();
	splitter->create(container, false, -150, false);

	m_previewControl = new EffectPreviewControl();
	m_previewControl->create(splitter, ui::WsClientBorder, m_resourceManager, renderSystem);
	m_previewControl->showGuide(m_guideVisible);

	m_sequencer = new ui::custom::SequencerControl();
	m_sequencer->create(splitter, ui::WsDoubleBuffer | ui::WsClientBorder);
	m_sequencer->addSelectEventHandler(ui::createMethodHandler(this, &EffectEditorPage::eventLayerSelect));
	m_sequencer->addCursorMoveEventHandler(ui::createMethodHandler(this, &EffectEditorPage::eventTimeCursorMove));

	updateSequencer();
	return true;
}

void EffectEditorPage::destroy()
{
	Ref< editor::Settings > settings = m_editor->getSettings();
	T_ASSERT (settings);

	settings->setProperty< editor::PropertyBoolean >(L"EffectEditor.ToggleGuide", m_guideVisible);

	m_previewControl->destroy();
}

void EffectEditorPage::activate()
{
}

void EffectEditorPage::deactivate()
{
}

bool EffectEditorPage::setDataObject(db::Instance* instance, Object* data)
{
	m_effectInstance = instance;

	m_effect = dynamic_type_cast< Effect* >(data);
	if (!m_effect)
		return false;

	if (!m_effect->bind(m_resourceManager))
		return false;

	m_previewControl->setEffect(m_effect);
	m_site->setPropertyObject(m_effect);

	updateSequencer();
	return true;
}

Ref< db::Instance > EffectEditorPage::getDataInstance()
{
	return m_effectInstance;
}

Ref< Object > EffectEditorPage::getDataObject()
{
	return m_effect;
}

bool EffectEditorPage::dropInstance(db::Instance* instance, const ui::Point& position)
{
	return false;
}

bool EffectEditorPage::handleCommand(const ui::Command& command)
{
	if (!m_previewControl)
		return false;

	if (command == L"Editor.PropertiesChanged")
	{
		m_previewControl->syncEffect();
		updateSequencer();
	}
	else if (command == L"Effect.Editor.Reset")
	{
		m_previewControl->setTotalTime(0.0f);
		m_previewControl->syncEffect();
	}
	else if (command == L"Effect.Editor.Rewind")
	{
		m_previewControl->setTotalTime(0.0f);
		m_previewControl->syncEffect();
	}
	else if (command == L"Effect.Editor.Play")
		m_previewControl->setTimeScale(1.0f);
	else if (command == L"Effect.Editor.Stop")
		m_previewControl->setTimeScale(0.0f);
	else if (command == L"Effect.Editor.ToggleGuide")
	{
		m_guideVisible = !m_guideVisible;
		m_previewControl->showGuide(m_guideVisible);
		m_toolToggleGuide->setToggled(m_guideVisible);
	}
	else if (command == L"Effect.Editor.ToggleVelocity")
	{
		m_velocityVisible = !m_velocityVisible;
		m_previewControl->showVelocity(m_velocityVisible);
	}
	else if (command == L"Effect.Editor.RandomizeSeed")
	{
		m_previewControl->randomizeSeed();
		m_previewControl->syncEffect();
	}

	return true;
}

void EffectEditorPage::handleDatabaseEvent(const Guid& eventId)
{
	if (m_resourceManager)
		m_resourceManager->update(eventId, true);
}

void EffectEditorPage::updateSequencer()
{
	m_sequencer->removeAllSequenceItems();

	if (!m_effect)
	{
		m_sequencer->setEnable(false);
		return;
	}

	const RefArray< EffectLayer >& layers = m_effect->getLayers();
	for (RefArray< EffectLayer >::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		StringOutputStream ss;
		ss << L"Layer " << uint32_t(std::distance(layers.begin(), i) + 1);

		Ref< ui::custom::Sequence > layerItem = new ui::custom::Sequence(ss.str());
		layerItem->setData(L"LAYER", *i);
		
		float start = (*i)->getTime();
		float end = (*i)->getTime() + (*i)->getDuration();

		Ref< ui::custom::Range > layerRange = new ui::custom::Range(
			int(start * 1000),
			int(end * 1000)
		);

		layerItem->addKey(layerRange);

		m_sequencer->addSequenceItem(layerItem);
	}

	m_sequencer->setEnable(true);
	m_sequencer->setLength(int(m_effect->getDuration() * 1000.0f));
}

void EffectEditorPage::eventToolClick(ui::Event* event)
{
	const ui::Command& command = checked_type_cast< ui::CommandEvent* >(event)->getCommand();
	handleCommand(command);
}

void EffectEditorPage::eventLayerSelect(ui::Event* event)
{
	RefArray< ui::custom::SequenceItem > selectedItems;
	if (m_sequencer->getSequenceItems(selectedItems, ui::custom::SequencerControl::GfSelectedOnly) == 1)
	{
		Ref< EffectLayer > layer = selectedItems.front()->getData< EffectLayer >(L"LAYER");
		T_ASSERT (layer);

		m_site->setPropertyObject(layer);
	}
	else
		m_site->setPropertyObject(m_effect);
}

void EffectEditorPage::eventTimeCursorMove(ui::Event* event)
{
	float time = m_sequencer->getCursor() / 1000.0f;

	m_previewControl->setTimeScale(0.0f);
	m_previewControl->setTotalTime(time);
	m_previewControl->syncEffect();

}

	}
}
