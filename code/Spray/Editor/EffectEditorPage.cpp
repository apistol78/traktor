#include "Core/Io/StringOutputStream.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/RfpMemberName.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Database/Instance.h"
#include "Editor/IDocument.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPageSite.h"
#include "I18N/Text.h"
#include "Mesh/MeshFactory.h"
#include "Render/IRenderSystem.h"
#include "Render/ITexture.h"
#include "Render/Resource/ShaderFactory.h"
#include "Render/Resource/TextureFactory.h"
#include "Resource/ResourceManager.h"
#include "Sound/SoundFactory.h"
#include "Sound/SoundSystem.h"
#include "Spray/EffectData.h"
#include "Spray/EffectFactory.h"
#include "Spray/EffectLayerData.h"
#include "Spray/EmitterData.h"
#include "Spray/SequenceData.h"
#include "Spray/SourceData.h"
#include "Spray/Editor/ClipboardData.h"
#include "Spray/Editor/EffectEditorPage.h"
#include "Spray/Editor/EffectPreviewControl.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/Clipboard.h"
#include "Ui/Command.h"
#include "Ui/Container.h"
#include "Ui/PopupMenu.h"
#include "Ui/MenuItem.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/Custom/ToolBar/ToolBarSeparator.h"
#include "Ui/Custom/Sequencer/CursorMoveEvent.h"
#include "Ui/Custom/Sequencer/KeyMoveEvent.h"
#include "Ui/Custom/Sequencer/Marker.h"
#include "Ui/Custom/Sequencer/Sequence.h"
#include "Ui/Custom/Sequencer/SequenceButtonClickEvent.h"
#include "Ui/Custom/Sequencer/SequenceMovedEvent.h"
#include "Ui/Custom/Sequencer/SequencerControl.h"
#include "Ui/Custom/Sequencer/Range.h"
#include "Ui/Custom/Sequencer/Tick.h"
#include "Ui/Custom/Splitter.h"
#include "World/PostProcess/PostProcessFactory.h"
#include "World/PostProcess/PostProcessSettings.h"

// Resources
#include "Resources/Playback.h"
#include "Resources/EffectEdit.h"
#include "Resources/LayerVisible.h"
#include "Resources/LayerHidden.h"

namespace traktor
{
	namespace spray
	{
		namespace
		{

class SequenceDataKey : public Object
{
	T_RTTI_CLASS;

public:
	SequenceDataKey(SequenceData* sequenceData, int32_t key)
	:	m_sequenceData(sequenceData)
	,	m_key(key)
	{
	}

	SequenceData* getSequenceData() { return m_sequenceData; }

	int32_t getKeyIndex() const { return m_key; }

private:
	Ref< SequenceData > m_sequenceData;
	int32_t m_key;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.SequenceDataKey", SequenceDataKey, Object)

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.EffectEditorPage", EffectEditorPage, editor::IEditorPage)

EffectEditorPage::EffectEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document)
:	m_editor(editor)
,	m_site(site)
,	m_document(document)
,	m_velocityVisible(false)
,	m_guideVisible(true)
,	m_moveEmitter(false)
,	m_groundClip(false)
{
}

bool EffectEditorPage::create(ui::Container* parent)
{
	render::IRenderSystem* renderSystem = m_editor->getStoreObject< render::IRenderSystem >(L"RenderSystem");
	if (!renderSystem)
		return false;

	m_soundSystem = m_editor->getStoreObject< sound::SoundSystem >(L"SoundSystem");

	Ref< db::Database > database = m_editor->getOutputDatabase();
	T_ASSERT (database);

	m_resourceManager = new resource::ResourceManager(true);
	m_resourceManager->addFactory(new mesh::MeshFactory(database, renderSystem));
	m_resourceManager->addFactory(new render::TextureFactory(database, renderSystem, 0));
	m_resourceManager->addFactory(new render::ShaderFactory(database, renderSystem));
	m_resourceManager->addFactory(new sound::SoundFactory(database));
	m_resourceManager->addFactory(new world::PostProcessFactory(database));
	m_resourceManager->addFactory(new EffectFactory(database, 0));

	m_effectData = m_document->getObject< EffectData >(0);
	if (!m_effectData)
		return false;

	activate();

	Ref< ui::Container > container = new ui::Container();
	container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0));

	m_toolToggleGuide = new ui::custom::ToolBarButton(i18n::Text(L"EFFECT_EDITOR_TOGGLE_GUIDE"), 7, ui::Command(L"Effect.Editor.ToggleGuide"), ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleMove = new ui::custom::ToolBarButton(i18n::Text(L"EFFECT_EDITOR_TOGGLE_MOVE"), 8, ui::Command(L"Effect.Editor.ToggleMove"), ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleGroundClip = new ui::custom::ToolBarButton(i18n::Text(L"EFFECT_EDITOR_TOGGLE_GROUND_CLIP"), 9, ui::Command(L"Effect.Editor.ToggleGroundClip"), ui::custom::ToolBarButton::BsDefaultToggle);

	Ref< const PropertyGroup > settings = m_editor->getSettings();
	T_ASSERT (settings);

	m_guideVisible = settings->getProperty< PropertyBoolean >(L"EffectEditor.ToggleGuide", m_guideVisible);
	m_toolToggleGuide->setToggled(m_guideVisible);

	m_moveEmitter = settings->getProperty< PropertyBoolean >(L"EffectEditor.ToggleMove", m_moveEmitter);
	m_toolToggleMove->setToggled(m_moveEmitter);

	m_groundClip = settings->getProperty< PropertyBoolean >(L"EffectEditor.ToggleGroundClip", m_groundClip);
	m_toolToggleGroundClip->setToggled(m_groundClip);

	m_toolBar = new ui::custom::ToolBar();
	m_toolBar->create(container);
	m_toolBar->addImage(ui::Bitmap::load(c_ResourcePlayback, sizeof(c_ResourcePlayback), L"png"), 6);
	m_toolBar->addImage(ui::Bitmap::load(c_ResourceEffectEdit, sizeof(c_ResourceEffectEdit), L"png"), 6);
	m_toolBar->addItem(new ui::custom::ToolBarButton(i18n::Text(L"EFFECT_EDITOR_REWIND"), 0, ui::Command(L"Effect.Editor.Rewind")));
	m_toolBar->addItem(new ui::custom::ToolBarButton(i18n::Text(L"EFFECT_EDITOR_PLAY"), 1, ui::Command(L"Effect.Editor.Play")));
	m_toolBar->addItem(new ui::custom::ToolBarButton(i18n::Text(L"EFFECT_EDITOR_STOP"), 2, ui::Command(L"Effect.Editor.Stop")));
	m_toolBar->addItem(new ui::custom::ToolBarSeparator());
	m_toolBar->addItem(m_toolToggleGuide);
	m_toolBar->addItem(m_toolToggleMove);
	m_toolBar->addItem(m_toolToggleGroundClip);
	m_toolBar->addItem(new ui::custom::ToolBarButton(i18n::Text(L"EFFECT_EDITOR_RANDOMIZE_SEED"), 6, ui::Command(L"Effect.Editor.RandomizeSeed")));
	m_toolBar->addItem(new ui::custom::ToolBarSeparator());
	m_toolBar->addItem(new ui::custom::ToolBarButton(i18n::Text(L"EFFECT_EDITOR_BROWSE_BACKGROUND"), 10, ui::Command(L"Effect.Editor.BrowseBackground")));
	m_toolBar->addItem(new ui::custom::ToolBarButton(i18n::Text(L"EFFECT_EDITOR_BROWSE_POSTPROCESS"), 11, ui::Command(L"Effect.Editor.BrowsePostProcess")));
	m_toolBar->addEventHandler< ui::custom::ToolBarButtonClickEvent >(this, &EffectEditorPage::eventToolClick);

	Ref< ui::custom::Splitter > splitter = new ui::custom::Splitter();
	splitter->create(container, false, -220, false);

	m_previewControl = new EffectPreviewControl(m_editor);
	m_previewControl->create(splitter, ui::WsClientBorder, m_resourceManager, renderSystem, m_soundSystem);
	m_previewControl->showGuide(m_guideVisible);
	m_previewControl->setMoveEmitter(m_moveEmitter);
	m_previewControl->setGroundClip(m_groundClip);

	m_sequencer = new ui::custom::SequencerControl();
	m_sequencer->create(splitter, ui::WsDoubleBuffer | ui::WsClientBorder);
	m_sequencer->addEventHandler< ui::SelectionChangeEvent >(this, &EffectEditorPage::eventSequencerLayerSelect);
	m_sequencer->addEventHandler< ui::custom::CursorMoveEvent >(this, &EffectEditorPage::eventSequencerTimeCursorMove);
	m_sequencer->addEventHandler< ui::custom::SequenceMovedEvent >(this, &EffectEditorPage::eventSequencerLayerRearranged);
	m_sequencer->addEventHandler< ui::custom::KeyMoveEvent >(this, &EffectEditorPage::eventSequencerKeyMove);
	m_sequencer->addEventHandler< ui::custom::SequenceButtonClickEvent >(this, &EffectEditorPage::eventSequencerLayerClick);
	m_sequencer->addEventHandler< ui::MouseButtonDownEvent >(this, &EffectEditorPage::eventSequencerButtonDown);

	m_popupMenu = new ui::PopupMenu();
	m_popupMenu->create();
	m_popupMenu->add(new ui::MenuItem(ui::Command(L"Effect.Editor.ReplaceEmitterSource"), i18n::Text(L"EFFECT_EDITOR_REPLACE_EMITTER_SOURCE")));

	m_site->setPropertyObject(m_effectData);

	updateSequencer();
	updateEffectPreview();

	return true;
}

void EffectEditorPage::destroy()
{
	Ref< PropertyGroup > settings = m_editor->checkoutGlobalSettings();
	T_ASSERT (settings);

	settings->setProperty< PropertyBoolean >(L"EffectEditor.ToggleGuide", m_guideVisible);
	settings->setProperty< PropertyBoolean >(L"EffectEditor.ToggleMove", m_moveEmitter);
	settings->setProperty< PropertyBoolean >(L"EffectEditor.ToggleGroundClip", m_groundClip);

	m_editor->commitGlobalSettings();
	m_soundSystem = 0;

	safeDestroy(m_popupMenu);
	safeDestroy(m_previewControl);
	safeDestroy(m_resourceManager);
}

void EffectEditorPage::activate()
{
}

void EffectEditorPage::deactivate()
{
}

bool EffectEditorPage::dropInstance(db::Instance* instance, const ui::Point& position)
{
	return false;
}

bool EffectEditorPage::handleCommand(const ui::Command& command)
{
	if (!m_previewControl)
		return false;

	if (command == L"Editor.SettingsChanged")
	{
		m_previewControl->updateSettings();
		m_previewControl->update();
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
	else if (command == L"Effect.Editor.ToggleMove")
	{
		m_moveEmitter = !m_moveEmitter;
		m_previewControl->setMoveEmitter(m_moveEmitter);
		m_previewControl->syncEffect();
		m_toolToggleMove->setToggled(m_moveEmitter);
	}
	else if (command == L"Effect.Editor.ToggleGroundClip")
	{
		m_groundClip = !m_groundClip;
		m_previewControl->setGroundClip(m_groundClip);
		m_toolToggleGroundClip->setToggled(m_groundClip);
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
	else if (command == L"Effect.Editor.BrowseBackground")
	{
		Ref< db::Instance > textureInstance = m_editor->browseInstance(type_of< render::ITexture >());
		if (textureInstance)
		{
			m_editor->buildAsset(textureInstance->getGuid(), false);
			m_previewControl->setBackground(resource::Id< render::ISimpleTexture >(textureInstance->getGuid()));
		}
	}
	else if (command == L"Effect.Editor.BrowsePostProcess")
	{
		Ref< db::Instance > postProcessInstance = m_editor->browseInstance(type_of< world::PostProcessSettings >());
		if (postProcessInstance)
		{
			m_editor->buildAsset(postProcessInstance->getGuid(), false);
			m_previewControl->setPostProcess(resource::Id< world::PostProcessSettings >(postProcessInstance->getGuid()));
		}
	}
	else if (command == L"Effect.Editor.ReplaceEmitterSource")
	{
		const TypeInfo* sourceType = m_editor->browseType(&type_of< SourceData >());
		if (sourceType)
		{
			RefArray< ui::custom::SequenceItem > selectedItems;
			if (m_sequencer->getSequenceItems(selectedItems, ui::custom::SequencerControl::GfSelectedOnly) > 0)
			{
				for (RefArray< ui::custom::SequenceItem >::iterator i = selectedItems.begin(); i != selectedItems.end(); ++i)
				{
					Ref< EffectLayerData > layer = (*i)->getData< EffectLayerData >(L"LAYER");
					
					EmitterData* emitter = layer->getEmitter();
					if (!emitter)
						continue;

					Ref< SourceData > source = checked_type_cast< SourceData*, false >(sourceType->createInstance());

					const SourceData* oldSource = emitter->getSource();
					if (oldSource)
					{
						// Extract parameters from old source and insert into new source.
						Ref< Reflection > sourceReflection = Reflection::create(source);
						T_ASSERT (sourceReflection);

						Ref< Reflection > oldSourceReflection = Reflection::create(oldSource);
						T_ASSERT (oldSourceReflection);

						for (uint32_t i = 0; i < sourceReflection->getMemberCount(); ++i)
						{
							ReflectionMember* sourceMember = sourceReflection->getMember(i);
							ReflectionMember* oldSourceMember = oldSourceReflection->findMember(RfpMemberName(sourceMember->getName()));
							if (oldSourceMember)
								sourceMember->replace(oldSourceMember);
						}

						sourceReflection->apply(source);
					}

					emitter->setSource(source);
				}
			}

			updateEffectPreview();
		}
	}
	else if (command == L"Editor.PropertiesChanging")
	{
		m_document->push();
	}
	else if (command == L"Editor.PropertiesChanged")
	{
		updateSequencer();
		updateEffectPreview();
	}
	else if (command == L"Editor.Copy")
	{
		RefArray< ui::custom::SequenceItem > selectedItems;
		if (m_sequencer->getSequenceItems(selectedItems, ui::custom::SequencerControl::GfSelectedOnly) > 0)
		{
			Ref< ClipboardData > clipboardData = new ClipboardData();
			for (RefArray< ui::custom::SequenceItem >::iterator i = selectedItems.begin(); i != selectedItems.end(); ++i)
			{
				Ref< EffectLayerData > layer = (*i)->getData< EffectLayerData >(L"LAYER");
				T_ASSERT (layer);

				clipboardData->addLayer(layer);
			}
			ui::Application::getInstance()->getClipboard()->setObject(clipboardData);
		}
	}
	else if (command == L"Editor.Paste")
	{
		Ref< ClipboardData > clipboardData = dynamic_type_cast< ClipboardData* >(
			ui::Application::getInstance()->getClipboard()->getObject()
		);
		if (clipboardData)
		{
			const RefArray< const EffectLayerData >& layers = clipboardData->getLayers();
			for (RefArray< const EffectLayerData >::const_iterator i = layers.begin(); i != layers.end(); ++i)
				m_effectData->addLayer(DeepClone(*i).create< EffectLayerData >());

			updateSequencer();
			updateEffectPreview();
		}
	}
	else if (command == L"Editor.Undo")
	{
		if (m_document->undo())
		{
			m_effectData = m_document->getObject< EffectData >(0);
			T_ASSERT (m_effectData);

			m_site->setPropertyObject(m_effectData);

			updateSequencer();
			updateEffectPreview();
		}
	}
	else if (command == L"Editor.Redo")
	{
		if (m_document->redo())
		{
			m_effectData = m_document->getObject< EffectData >(0);
			T_ASSERT (m_effectData);

			m_site->setPropertyObject(m_effectData);

			updateSequencer();
			updateEffectPreview();
		}
	}

	return true;
}

void EffectEditorPage::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
	if (m_resourceManager)
		m_resourceManager->reload(eventId);
}

void EffectEditorPage::updateEffectPreview()
{
	if (m_resourceManager)
	{
		RefArray< EffectLayerData > layersData;

		RefArray< ui::custom::SequenceItem > layers;
		m_sequencer->getSequenceItems(layers, ui::custom::SequencerControl::GfDefault);

		for (RefArray< ui::custom::SequenceItem >::const_iterator i = layers.begin(); i != layers.end(); ++i)
		{
			const ui::custom::Sequence* layerItem = checked_type_cast< ui::custom::Sequence*, false >(*i);
			if (layerItem->getButtonState(0))
				continue;

			layersData.push_back(layerItem->getData< EffectLayerData >(L"LAYER"));
		}

		Ref< EffectData > effectData = new EffectData(
			m_effectData->getDuration(),
			m_effectData->getLoopStart(),
			m_effectData->getLoopEnd(),
			layersData
		);

		Ref< Effect > effect = effectData->createEffect(m_resourceManager, 0);
		m_previewControl->setEffect(effect);

		float time = m_sequencer->getCursor() / 1000.0f;
		m_previewControl->setTotalTime(time);
		m_previewControl->syncEffect();
	}
	else
		m_previewControl->setEffect(0);
}

void EffectEditorPage::updateSequencer()
{
	m_sequencer->removeAllSequenceItems();

	if (!m_effectData)
	{
		m_sequencer->setEnable(false);
		m_sequencer->update();
		return;
	}

	Ref< ui::Bitmap > layerVisible = ui::Bitmap::load(c_ResourceLayerVisible, sizeof(c_ResourceLayerVisible), L"png");
	Ref< ui::Bitmap > layerHidden = ui::Bitmap::load(c_ResourceLayerHidden, sizeof(c_ResourceLayerHidden), L"png");

	const RefArray< EffectLayerData >& layers = m_effectData->getLayers();
	for (RefArray< EffectLayerData >::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		Ref< ui::custom::Sequence > layerItem = new ui::custom::Sequence((*i)->getName());
		layerItem->addButton(layerVisible, layerHidden, ui::Command(L"Effect.Editor.ToggleLayerVisible"));
		layerItem->setData(L"LAYER", *i);
		
		float start = (*i)->getTime();
		if ((*i)->getDuration() > 0.0f)
		{
			float end = (*i)->getTime() + (*i)->getDuration();
			Ref< ui::custom::Range > layerRange = new ui::custom::Range(
				int32_t(start * 1000.0f),
				int32_t(end * 1000.0f),
				true
			);
			layerRange->setData(L"LAYER", *i);
			layerItem->addKey(layerRange);
		}
		else
		{
			Ref< ui::custom::Tick > layerTick = new ui::custom::Tick(int32_t(start * 1000.0f), true);
			layerTick->setData(L"LAYER", *i);
			layerItem->addKey(layerTick);
		}

		// Add sequence entries.
		Ref< SequenceData > sequence = (*i)->getSequence();
		if (sequence)
		{
			const std::vector< SequenceData::Key >& keys = sequence->getKeys();
			for (int32_t j = 0; j < int32_t(keys.size()); ++j)
			{
				Ref< ui::custom::Marker > sequenceMarker = new ui::custom::Marker(int32_t(keys[j].T * 1000.0f), true);
				sequenceMarker->setData(L"SEQUENCE", new SequenceDataKey(sequence, j));
				sequenceMarker->setData(L"LAYER", *i);
				layerItem->addKey(sequenceMarker);
			}
		}

		m_sequencer->addSequenceItem(layerItem);
	}

	m_sequencer->setEnable(true);
	m_sequencer->setLength(int(m_effectData->getDuration() * 1000.0f));
	m_sequencer->update();
}

void EffectEditorPage::eventToolClick(ui::custom::ToolBarButtonClickEvent* event)
{
	const ui::Command& command = event->getCommand();
	handleCommand(command);
}

void EffectEditorPage::eventSequencerLayerSelect(ui::SelectionChangeEvent* event)
{
	RefArray< ui::custom::SequenceItem > selectedItems;
	if (m_sequencer->getSequenceItems(selectedItems, ui::custom::SequencerControl::GfSelectedOnly) == 1)
	{
		ui::custom::Sequence* selectedSequence = checked_type_cast< ui::custom::Sequence*, false >(selectedItems.front());

		Ref< EffectLayerData > layer = selectedSequence->getData< EffectLayerData >(L"LAYER");
		T_ASSERT (layer);

		ui::custom::Marker* selectedMarker = dynamic_type_cast< ui::custom::Marker* >(selectedSequence->getSelectedKey());
		if (selectedMarker)
		{
			// FIXME
			m_site->setPropertyObject(layer);
		}
		else
		{
			m_site->setPropertyObject(layer);
		}
	}
	else
		m_site->setPropertyObject(m_effectData);
}

void EffectEditorPage::eventSequencerTimeCursorMove(ui::custom::CursorMoveEvent* event)
{
	float time = m_sequencer->getCursor() / 1000.0f;
	m_previewControl->setTimeScale(0.0f);
	m_previewControl->setTotalTime(time);
	m_previewControl->syncEffect();
}

void EffectEditorPage::eventSequencerLayerRearranged(ui::custom::SequenceMovedEvent* event)
{
	Ref< EffectLayerData > movedLayer = event->getItem()->getData< EffectLayerData >(L"LAYER");
	T_ASSERT (movedLayer);

	RefArray< EffectLayerData > layers = m_effectData->getLayers();
	
	layers.remove(movedLayer);
	layers.insert(layers.begin() + event->getMovedTo(), movedLayer);

	m_effectData->setLayers(layers);
	updateEffectPreview();
}

void EffectEditorPage::eventSequencerKeyMove(ui::custom::KeyMoveEvent* event)
{
	ui::custom::Range* movedRange = dynamic_type_cast< ui::custom::Range* >(event->getKey());
	if (movedRange)
	{
		Ref< EffectLayerData > layer = movedRange->getData< EffectLayerData >(L"LAYER");
		T_ASSERT (layer);

		float start = movedRange->getStart() / 1000.0f;
		float end = movedRange->getEnd() / 1000.0f;
		m_document->push();
		layer->setTime(start);
		layer->setDuration(end - start);
		m_site->setPropertyObject(layer);
	}

	ui::custom::Tick* movedTick = dynamic_type_cast< ui::custom::Tick* >(event->getKey());
	if (movedTick)
	{
		Ref< EffectLayerData > layer = movedTick->getData< EffectLayerData >(L"LAYER");
		T_ASSERT (layer);

		float start = movedTick->getTime() / 1000.0f;
		m_document->push();
		layer->setTime(start);
		m_site->setPropertyObject(layer);
	}

	ui::custom::Marker* movedMarker = dynamic_type_cast< ui::custom::Marker* >(event->getKey());
	if (movedMarker)
	{
		Ref< EffectLayerData > layer = movedMarker->getData< EffectLayerData >(L"LAYER");
		T_ASSERT (layer);

		Ref< SequenceDataKey > sequenceDataKey = movedMarker->getData< SequenceDataKey >(L"SEQUENCE");
		T_ASSERT (sequenceDataKey);

		SequenceData* sequenceData = sequenceDataKey->getSequenceData();
		T_ASSERT (sequenceData);

		m_document->push();

		std::vector< SequenceData::Key >& keys = sequenceData->getKeys();
		keys[sequenceDataKey->getKeyIndex()].T = float(movedMarker->getTime() / 1000.0f);

		m_site->setPropertyObject(layer);
	}

	updateEffectPreview();
}

void EffectEditorPage::eventSequencerLayerClick(ui::custom::SequenceButtonClickEvent* event)
{
	updateEffectPreview();
}

void EffectEditorPage::eventSequencerButtonDown(ui::MouseButtonDownEvent* event)
{
	if (event->getButton() == ui::MbtRight)
	{
		Ref< ui::MenuItem > selectedItem = m_popupMenu->show(m_sequencer, event->getPosition());
		if (selectedItem != 0)
			handleCommand(selectedItem->getCommand());
	}
}

	}
}
