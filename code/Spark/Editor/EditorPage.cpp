/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileSystem.h"
#include "Core/Io/MemoryStream.h"
#include "Core/Math/Format.h"
#include "Core/Misc/ObjectStore.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IDocument.h"
#include "Editor/IEditor.h"
#include "Spark/Button.h"
#include "Spark/ButtonInstance.h"
#include "Spark/Context.h"
#include "Spark/Frame.h"
#include "Spark/Movie.h"
#include "Spark/MoviePlayer.h"
#include "Spark/Optimizer.h"
#include "Spark/Shape.h"
#include "Spark/ShapeInstance.h"
#include "Spark/Sprite.h"
#include "Spark/SpriteInstance.h"
#include "Spark/Swf/SwfTypes.h"
#include "Spark/Editor/EditorPage.h"
#include "Spark/Editor/MovieAsset.h"
#include "Spark/Editor/PreviewControl.h"
#include "I18N/Text.h"
#include "Render/IRenderSystem.h"
#include "Render/Resource/ShaderFactory.h"
#include "Resource/ResourceManager.h"
#include "Sound/Player/ISoundPlayer.h"
#include "Ui/Application.h"
#include "Ui/Container.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"
#include "Ui/AspectLayout.h"
#include "Ui/CenterLayout.h"
#include "Ui/Splitter.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/ToolBar/ToolBarSeparator.h"
#include "Ui/TreeView/TreeView.h"
#include "Ui/TreeView/TreeViewItem.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.EditorPage", EditorPage, editor::IEditorPage)

EditorPage::EditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document)
:	m_editor(editor)
,	m_site(site)
,	m_document(document)
{
}

bool EditorPage::create(ui::Container* parent)
{
	Ref< render::IRenderSystem > renderSystem = m_editor->getObjectStore()->get< render::IRenderSystem >();
	if (!renderSystem)
		return false;

	Ref< sound::ISoundPlayer > soundPlayer = m_editor->getObjectStore()->get< sound::ISoundPlayer >();
	Ref< db::Database > database = m_editor->getOutputDatabase();

	// Read movie from output database.
	m_movie = database->getObjectReadOnly< Movie >(m_document->getInstance(0)->getGuid());
	if (!m_movie)
		return false;

	m_resourceManager = new resource::ResourceManager(database, true);
	m_resourceManager->addFactory(new render::ShaderFactory(renderSystem));

	Ref< ui::Container > container = new ui::Container();
	container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0_ut, 0_ut));

	m_toolBarPlay = new ui::ToolBar();
	m_toolBarPlay->create(container);
	for (int32_t i = 0; i < 6; ++i)
		m_toolBarPlay->addImage(new ui::StyleBitmap(L"Flash.Playback", i));
	m_toolBarPlay->addItem(new ui::ToolBarButton(i18n::Text(L"FLASH_EDITOR_REWIND"), 0, ui::Command(L"Flash.Editor.Rewind")));
	m_toolBarPlay->addItem(new ui::ToolBarButton(i18n::Text(L"FLASH_EDITOR_PLAY"), 1, ui::Command(L"Flash.Editor.Play")));
	m_toolBarPlay->addItem(new ui::ToolBarButton(i18n::Text(L"FLASH_EDITOR_STOP"), 2, ui::Command(L"Flash.Editor.Stop")));
	m_toolBarPlay->addItem(new ui::ToolBarButton(i18n::Text(L"FLASH_EDITOR_FORWARD"), 3, ui::Command(L"Flash.Editor.Forward")));

	m_toolBarPlay->addEventHandler< ui::ToolBarButtonClickEvent >(this, &EditorPage::eventToolClick);

	Ref< ui::Splitter > splitter = new ui::Splitter();
	splitter->create(container, true, 300_ut);

	m_treeMovie = new ui::TreeView();
	m_treeMovie->create(splitter, ui::TreeView::WsTreeButtons | ui::TreeView::WsTreeLines | ui::WsDoubleBuffer);
	m_treeMovie->addEventHandler< ui::SelectionChangeEvent >(this, &EditorPage::eventTreeMovieSelect);

	m_previewControl = new PreviewControl(m_editor);
	if (!m_previewControl->create(splitter, ui::WsNone, database, m_resourceManager, renderSystem, soundPlayer))
		return false;

	m_previewControl->setMovie(m_movie);
	m_previewControl->update();

	return true;
}

void EditorPage::destroy()
{
	safeDestroy(m_previewControl);
}

bool EditorPage::dropInstance(db::Instance* instance, const ui::Point& position)
{
	return false;
}

bool EditorPage::handleCommand(const ui::Command& command)
{
	bool result = true;

	if (command == L"Flash.Editor.Rewind")
	{
		m_previewControl->rewind();
		updateTreeMovie();
	}
	else if (command == L"Flash.Editor.Play")
	{
		m_previewControl->play();
		updateTreeMovie();
	}
	else if (command == L"Flash.Editor.Stop")
	{
		m_previewControl->stop();
		updateTreeMovie();
	}
	else if (command == L"Flash.Editor.Forward")
	{
		m_previewControl->forward();
		updateTreeMovie();
	}
	else
		result = false;

	return result;
}

void EditorPage::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
	bool shouldRedraw = false;

	if (m_resourceManager)
		shouldRedraw |= m_resourceManager->reload(eventId, false);

	if (eventId == m_document->getInstance(0)->getGuid())
	{
		Ref< Movie > movie = database->getObjectReadOnly< Movie >(m_document->getInstance(0)->getGuid());
		if (movie)
		{
			m_selectedCharacterInstance = nullptr;
			m_movie = movie;

			m_previewControl->setMovie(m_movie);
			shouldRedraw |= true;
		}
	}

	if (shouldRedraw)
		m_previewControl->update();
}

void EditorPage::updateTreeCharacter(ui::TreeViewItem* parentItem, CharacterInstance* characterInstance, std::map< const void*, uint32_t >& pointerHash, uint32_t& nextPointerHash)
{
	StringOutputStream ss;
	ss << type_name(characterInstance);

	if (!characterInstance->getName().empty())
		ss << std::wstring(L" \"") << mbstows(characterInstance->getName()) << std::wstring(L"\"");

	Ref< ui::TreeViewItem > characterItem = m_treeMovie->createItem(parentItem, ss.str(), 1);
	characterItem->setImage(0, 0);
	characterItem->setData(L"CHARACTER", characterInstance);

	//const SwfCxTransform& cxform = characterInstance->getColorTransform();
	//ss.reset();
	//ss << L"Color transform: [*] = {" << cxform.red[0] << L", " << cxform.green[0] << L", " << cxform.blue[0] << L", " << cxform.alpha[0] << L"}, [+] = {" << cxform.red[1] << L", " << cxform.green[1] << L", " << cxform.blue[1] << L", " << cxform.alpha[1] << L"}";
	//m_treeMovie->createItem(characterItem, ss.str());

	ss.reset();
	ss << L"Blend mode: ";
	switch (characterInstance->getBlendMode())
	{
	case SbmDefault:
		ss << L"Default";
		break;
	case SbmNormal:
		ss << L"Normal";
		break;
	case SbmLayer:
		ss << L"Layer";
		break;
	case SbmMultiply:
		ss << L"Multiply";
		break;
	case SbmScreen:
		ss << L"Screen";
		break;
	case SbmLighten:
		ss << L"Lighten";
		break;
	case SbmDarken:
		ss << L"Darken";
		break;
	case SbmDifference:
		ss << L"Difference";
		break;
	case SbmAdd:
		ss << L"Add";
		break;
	case SbmSubtract:
		ss << L"Subtract";
		break;
	case SbmInvert:
		ss << L"Invert";
		break;
	case SbmAlpha:
		ss << L"Alpha";
		break;
	case SbmErase:
		ss << L"Erase";
		break;
	case SbmOverlay:
		ss << L"Overlay";
		break;
	case SbmHardlight:
		ss << L"Hard light";
		break;
	default:
		ss << L"[Invalid]";
		break;
	}
	m_treeMovie->createItem(characterItem, ss.str(), 1);

	const Aabb2& bounds = characterInstance->getBounds();
	ss.reset();
	if (!bounds.empty())
		ss << L"Bounds: " << bounds.mn << L" - " << bounds.mx << IncreaseIndent;
	else
		ss << L"Bounds: [Empty]" << IncreaseIndent;
	m_treeMovie->createItem(characterItem, ss.str(), 1);

	if (SpriteInstance* spriteInstance = dynamic_type_cast< SpriteInstance* >(characterInstance))
	{
		const Sprite* sprite = spriteInstance->getSprite();
		if (sprite)
		{
			const uint32_t frameCount = sprite->getFrameCount();
			m_treeMovie->createItem(characterItem, toString(frameCount) + L" frame(s)", 1);

			Ref< ui::TreeViewItem > labelsItem = m_treeMovie->createItem(characterItem, L"Label(s)", 1);
			labelsItem->setImage(0, 0);

			for (uint32_t i = 0; i < frameCount; ++i)
			{
				const Frame* frame = sprite->getFrame(i);
				if (frame)
				{
					const std::string& label = frame->getLabel();
					if (!label.empty())
						m_treeMovie->createItem(labelsItem, toString(i) + L". \"" + mbstows(label) + L"\"", 1);
				}
			}
		}

		Ref< ui::TreeViewItem > layersItem = m_treeMovie->createItem(characterItem, L"Layer(s)", 1);
		layersItem->setImage(0, 0);

		const DisplayList::layer_map_t& layers = spriteInstance->getDisplayList().getLayers();
		for (DisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
		{
			ss.reset();
			ss << i->first;
			if (i->second.clipDepth != 0)
				ss << L", Clip depth: " << i->second.clipDepth << Endl;

			Ref< ui::TreeViewItem > layerItem = m_treeMovie->createItem(layersItem, ss.str(), 1);
			layerItem->setImage(0, 0);

			if (i->second.instance)
				updateTreeCharacter(layerItem, i->second.instance, pointerHash, nextPointerHash);

			layerItem->setData(L"CHARACTER", i->second.instance);
		}
	}
	else if (ButtonInstance* buttonInstance = dynamic_type_cast< ButtonInstance* >(characterInstance))
	{
		const Button* button = buttonInstance->getButton();
		uint8_t buttonState = buttonInstance->getState();

		m_treeMovie->createItem(characterItem, L"State " + toString(int32_t(buttonState)), 1);

		Ref< ui::TreeViewItem > layersItem = m_treeMovie->createItem(characterItem, L"Layers(s)", 1);
		layersItem->setImage(0, 0);

		const Button::button_layers_t& layers = button->getButtonLayers();
		for (int32_t j = 0; j < int32_t(layers.size()); ++j)
		{
			const Button::ButtonLayer& layer = layers[j];

			Ref< ui::TreeViewItem > layerItem = m_treeMovie->createItem(layersItem, toString(j) + L": " + toString(int32_t(layer.state)), 1);
			layerItem->setImage(0, 0);

			CharacterInstance* referenceInstance = buttonInstance->getCharacterInstance(layer.characterId);
			if (referenceInstance)
				updateTreeCharacter(layerItem, referenceInstance, pointerHash, nextPointerHash);

			layerItem->setData(L"CHARACTER", referenceInstance);
		}
	}
	else if (ShapeInstance* shapeInstance = dynamic_type_cast< ShapeInstance* >(characterInstance))
	{
		const Shape* shape = shapeInstance->getShape();

		const AlignedVector< FillStyle >& fillStyles = shape->getFillStyles();
		if (!fillStyles.empty())
		{
			ss.reset();
			ss << int32_t(fillStyles.size()) << L" fill(s)";
			Ref< ui::TreeViewItem > styleItem = m_treeMovie->createItem(characterItem, ss.str(), 1);

			for (uint32_t i = 0; i < uint32_t(fillStyles.size()); ++i)
			{
				const FillStyle& fs = fillStyles[i];
				const AlignedVector< FillStyle::ColorRecord >& cr = fs.getColorRecords();

				ss.reset();
				ss << (cr[0].color.getRed()) << L", " << (cr[0].color.getGreen()) << L", " << (cr[0].color.getBlue()) << L", " << (cr[0].color.getAlpha());
				m_treeMovie->createItem(styleItem, ss.str(), 1);
			}
		}

		const AlignedVector< Path >& paths = shape->getPaths();
		if (!paths.empty())
		{
			Ref< ui::TreeViewItem > pathsItem = m_treeMovie->createItem(characterItem, L"Path(s)", 1);
			pathsItem->setImage(0, 0);

			for (uint32_t i = 0; i < uint32_t(paths.size()); ++i)
			{
				const Path& p = paths[i];

				ss.reset();
				ss << i << L". " << int32_t(p.getPoints().size()) << L" point(s), " << int32_t(p.getSubPaths().size()) << L" subpath(s)";

				Ref< ui::TreeViewItem > pathItem = m_treeMovie->createItem(pathsItem, ss.str(), 1);
				pathItem->setData(L"CHARACTER", characterInstance);
				pathItem->setData(L"PATH", new PropertyInteger(i));
			}
		}
	}
}

void EditorPage::updateTreeMovie()
{
	m_treeMovie->removeAllItems();
	if (!m_previewControl->playing())
	{
		MoviePlayer* moviePlayer = m_previewControl->getMoviePlayer();
		if (moviePlayer)
		{
			SpriteInstance* movieInstance = moviePlayer->getMovieInstance();
			if (movieInstance)
			{
				std::map< const void*, uint32_t > pointerHash;
				uint32_t nextPointerHash = 0;

				Ref< ui::TreeViewItem > memberItemRoot = m_treeMovie->createItem(0, L"_root", 1);
				memberItemRoot->setImage(0, 0);

				updateTreeCharacter(memberItemRoot, movieInstance, pointerHash, nextPointerHash);

				Ref< ui::TreeViewItem > memberItemExports = m_treeMovie->createItem(0, L"Export(s)", 1);
				memberItemExports->setImage(0, 0);

				const SmallMap< std::string, uint16_t >& exports = m_movie->getExports();
				for (SmallMap< std::string, uint16_t >::const_iterator i = exports.begin(); i != exports.end(); ++i)
					m_treeMovie->createItem(memberItemExports, toString(i->second) + L": \"" + mbstows(i->first) + L"\"", 1);
			}
		}
	}
	else if (m_selectedCharacterInstance)
	{
		m_selectedCharacterInstance->setColorTransform(m_selectedCharacterInstanceCxForm);
		m_selectedCharacterInstance = nullptr;
		m_previewControl->update();
	}
}

void EditorPage::eventToolClick(ui::ToolBarButtonClickEvent* event)
{
	const ui::Command& command = event->getCommand();
	handleCommand(command);
}

void EditorPage::eventTreeMovieSelect(ui::SelectionChangeEvent* event)
{
	RefArray< ui::TreeViewItem > selectedItems;
	m_treeMovie->getItems(selectedItems, ui::TreeView::GfDescendants | ui::TreeView::GfSelectedOnly);

	if (m_selectedCharacterInstance)
	{
		m_selectedCharacterInstance->setColorTransform(m_selectedCharacterInstanceCxForm);
		m_selectedCharacterInstance->setBlendMode(m_selectedCharacterInstanceBlendMode);
		m_selectedCharacterInstance = nullptr;
	}

	if (selectedItems.size() == 1)
	{
		m_selectedCharacterInstance = selectedItems[0]->getData< CharacterInstance >(L"CHARACTER");
		if (m_selectedCharacterInstance)
		{
			m_selectedCharacterInstanceCxForm = m_selectedCharacterInstance->getColorTransform();
			m_selectedCharacterInstanceBlendMode = m_selectedCharacterInstance->getBlendMode();

			ColorTransform cxform;
			cxform.mul = Color4f(0.5f, 0.5f, 0.5f, 1.0f);
			cxform.add = Color4f(0.0f, 0.5f, 1.0f, 0.0f);

			m_selectedCharacterInstance->setColorTransform(cxform);
			m_selectedCharacterInstance->setBlendMode(SbmDefault);
		}
	}

	m_previewControl->update();
}

	}
}
