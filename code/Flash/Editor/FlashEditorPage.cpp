/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/FileSystem.h"
#include "Core/Io/MemoryStream.h"
#include "Core/Math/Format.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Instance.h"
#include "Editor/IDocument.h"
#include "Editor/IEditor.h"
#include "Flash/Button.h"
#include "Flash/ButtonInstance.h"
#include "Flash/Frame.h"
#include "Flash/Movie.h"
#include "Flash/MovieFactory.h"
#include "Flash/MoviePlayer.h"
#include "Flash/Optimizer.h"
#include "Flash/Shape.h"
#include "Flash/ShapeInstance.h"
#include "Flash/Sprite.h"
#include "Flash/SpriteInstance.h"
#include "Flash/SwfReader.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionObject.h"
#include "Flash/Action/Common/Array.h"
#include "Flash/Editor/FlashEditorPage.h"
#include "Flash/Editor/FlashMovieAsset.h"
#include "Flash/Editor/FlashPathControl.h"
#include "Flash/Editor/FlashPreviewControl.h"
#include "I18N/Text.h"
#include "Render/IRenderSystem.h"
#include "Render/Resource/ShaderFactory.h"
#include "Resource/ResourceManager.h"
#include "Sound/Player/ISoundPlayer.h"
#include "Ui/Application.h"
#include "Ui/Container.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/AspectLayout.h"
#include "Ui/Custom/CenterLayout.h"
#include "Ui/Custom/Splitter.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/Custom/ToolBar/ToolBarSeparator.h"
#include "Ui/Custom/TreeView/TreeView.h"
#include "Ui/Custom/TreeView/TreeViewItem.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashEditorPage", FlashEditorPage, editor::IEditorPage)

FlashEditorPage::FlashEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document)
:	m_editor(editor)
,	m_site(site)
,	m_document(document)
{
}

bool FlashEditorPage::create(ui::Container* parent)
{
	Ref< render::IRenderSystem > renderSystem = m_editor->getStoreObject< render::IRenderSystem >(L"RenderSystem");
	if (!renderSystem)
		return false;

	Ref< sound::ISoundPlayer > soundPlayer = m_editor->getStoreObject< sound::ISoundPlayer >(L"SoundPlayer");

	Ref< FlashMovieAsset > asset = m_document->getObject< FlashMovieAsset >(0);
	if (!asset)
		return false;

	std::wstring assetPath = m_editor->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
	traktor::Path fileName = FileSystem::getInstance().getAbsolutePath(assetPath, asset->getFileName());
	Ref< IStream > stream = FileSystem::getInstance().open(fileName, File::FmRead);
	if (!stream)
		return false;

	uint32_t assetSize = stream->available();
	std::vector< uint8_t > assetBlob(assetSize);

	uint32_t offset = 0;
	while (offset < assetSize)
	{
		int nread = stream->read(&assetBlob[offset], assetSize - offset);
		if (nread < 0)
			return false;
		offset += nread;
	}

	stream->close();

	Ref< MemoryStream > memoryStream = new MemoryStream(&assetBlob[0], int(assetSize), true, false);
	Ref< SwfReader > swf = new SwfReader(memoryStream);

	m_movie = flash::MovieFactory(true).createMovie(swf);
	if (!m_movie)
		return false;

	Ref< db::Database > database = m_editor->getOutputDatabase();

	m_resourceManager = new resource::ResourceManager(database, true);
	m_resourceManager->addFactory(new render::ShaderFactory(renderSystem));

	Ref< ui::Container > container = new ui::Container();
	container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0));

	m_toolBarPlay = new ui::custom::ToolBar();
	m_toolBarPlay->create(container);
	m_toolBarPlay->addImage(new ui::StyleBitmap(L"Flash.Playback"), 6);
	m_toolBarPlay->addImage(new ui::StyleBitmap(L"Flash.Flash"), 2);
	m_toolBarPlay->addItem(new ui::custom::ToolBarButton(i18n::Text(L"FLASH_EDITOR_REWIND"), 0, ui::Command(L"Flash.Editor.Rewind")));
	m_toolBarPlay->addItem(new ui::custom::ToolBarButton(i18n::Text(L"FLASH_EDITOR_PLAY"), 1, ui::Command(L"Flash.Editor.Play")));
	m_toolBarPlay->addItem(new ui::custom::ToolBarButton(i18n::Text(L"FLASH_EDITOR_STOP"), 2, ui::Command(L"Flash.Editor.Stop")));
	m_toolBarPlay->addItem(new ui::custom::ToolBarButton(i18n::Text(L"FLASH_EDITOR_FORWARD"), 3, ui::Command(L"Flash.Editor.Forward")));

	m_toolBarPlay->addEventHandler< ui::custom::ToolBarButtonClickEvent >(this, &FlashEditorPage::eventToolClick);

	Ref< ui::custom::Splitter > splitter = new ui::custom::Splitter();
	splitter->create(container, true, ui::scaleBySystemDPI(300));

	Ref< ui::custom::Splitter > splitterV = new ui::custom::Splitter();
	splitterV->create(splitter, false, ui::scaleBySystemDPI(-200));

	m_treeMovie = new ui::custom::TreeView();
	m_treeMovie->create(splitterV, ui::custom::TreeView::WsTreeButtons | ui::custom::TreeView::WsTreeLines | ui::WsDoubleBuffer);
	m_treeMovie->addEventHandler< ui::SelectionChangeEvent >(this, &FlashEditorPage::eventTreeMovieSelect);

	m_pathControl = new FlashPathControl();
	m_pathControl->create(splitterV, ui::WsAccelerated | ui::WsDoubleBuffer);

	m_previewControl = new FlashPreviewControl(m_editor);
	if (!m_previewControl->create(splitter, ui::WsNone, database, m_resourceManager, renderSystem, soundPlayer))
		return false;

	m_previewControl->setMovie(m_movie);
	m_previewControl->update();

	return true;
}

void FlashEditorPage::destroy()
{
	safeDestroy(m_previewControl);
}

bool FlashEditorPage::dropInstance(db::Instance* instance, const ui::Point& position)
{
	return false;
}

bool FlashEditorPage::handleCommand(const ui::Command& command)
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

void FlashEditorPage::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
	if (m_resourceManager)
		m_resourceManager->reload(eventId, false);
}

void FlashEditorPage::updateTreeObject(ui::custom::TreeViewItem* parentItem, const ActionObject* asObject, std::set< const ActionObject* >& objectStack, std::map< const void*, uint32_t >& pointerHash, uint32_t& nextPointerHash)
{
	if (!asObject)
		return;

	const ActionObject::member_map_t& asMembers = asObject->getLocalMembers();
	for (ActionObject::member_map_t::const_iterator i = asMembers.begin(); i != asMembers.end(); ++i)
	{
		std::string memberName = asObject->getContext()->getString(i->first);

		StringOutputStream ss;
		ss << mbstows(memberName) << L" = \"" << i->second.getWideString() << L"\"";

		if (i->second.isObject())
		{
			std::map< const void*, uint32_t >::const_iterator j = pointerHash.find(i->second.getObject());
			if (j != pointerHash.end())
				ss << L" @" << j->second;
			else
			{
				uint32_t hash = nextPointerHash++;
				pointerHash.insert(std::make_pair(i->second.getObject(), hash));
				ss << L" @" << hash;
			}
		}

		Ref< ui::custom::TreeViewItem > memberItem = m_treeMovie->createItem(
			parentItem,
			ss.str(),
			0,
			0
		);

		if (i->second.isObject())
		{
			if (objectStack.find(i->second.getObject()) == objectStack.end())
			{
				objectStack.insert(i->second.getObject());
				updateTreeObject(memberItem, i->second.getObject(), objectStack, pointerHash, nextPointerHash);
			}
		}
	}

	const Array* asArray = asObject->getRelay< Array >();
	if (asArray)
	{
		const AlignedVector< ActionValue >& asValues = asArray->getValues();
		for (uint32_t i = 0; i < asValues.size(); ++i)
		{
			StringOutputStream ss;
			ss << L"[" << i << L"] = \"" << asValues[i].getWideString() << L"\"";

			if (asValues[i].isObject())
			{
				std::map< const void*, uint32_t >::const_iterator j = pointerHash.find(asValues[i].getObject());
				if (j != pointerHash.end())
					ss << L" @" << j->second;
				else
				{
					uint32_t hash = nextPointerHash++;
					pointerHash.insert(std::make_pair(asValues[i].getObject(), hash));
					ss << L" @" << hash;
				}
			}

			Ref< ui::custom::TreeViewItem > memberItem = m_treeMovie->createItem(
				parentItem,
				ss.str(),
				0,
				0
			);

			if (asValues[i].isObject())
			{
				if (objectStack.find(asValues[i].getObject()) == objectStack.end())
				{
					objectStack.insert(asValues[i].getObject());
					updateTreeObject(memberItem, asValues[i].getObject(), objectStack, pointerHash, nextPointerHash);
				}
			}
		}
	}
}

void FlashEditorPage::updateTreeCharacter(ui::custom::TreeViewItem* parentItem, CharacterInstance* characterInstance, std::map< const void*, uint32_t >& pointerHash, uint32_t& nextPointerHash)
{
	StringOutputStream ss;
	ss << type_name(characterInstance);
	
	if (!characterInstance->getName().empty())
		ss << std::wstring(L" \"") << mbstows(characterInstance->getName()) << std::wstring(L"\"");

	const ActionObject* asObject = characterInstance->getAsObject();
	if (asObject)
	{
		std::map< const void*, uint32_t >::const_iterator j = pointerHash.find(asObject);
		if (j != pointerHash.end())
			ss << L" @" << j->second;
		else
		{
			uint32_t hash = nextPointerHash++;
			pointerHash.insert(std::make_pair(asObject, hash));
			ss << L" @" << hash;
		}
	}

	Ref< ui::custom::TreeViewItem > characterItem = m_treeMovie->createItem(parentItem, ss.str(), 0, 0);
	T_ASSERT (characterItem);

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
		ss << L"Default" << Endl;
		break;
	case SbmNormal:
		ss << L"Normal" << Endl;
		break;
	case SbmLayer:
		ss << L"Layer" << Endl;
		break;
	case SbmMultiply:
		ss << L"Multiply" << Endl;
		break;
	case SbmScreen:
		ss << L"Screen" << Endl;
		break;
	case SbmLighten:
		ss << L"Lighten" << Endl;
		break;
	case SbmDarken:
		ss << L"Darken" << Endl;
		break;
	case SbmDifference:
		ss << L"Difference" << Endl;
		break;
	case SbmAdd:
		ss << L"Add" << Endl;
		break;
	case SbmSubtract:
		ss << L"Subtract" << Endl;
		break;
	case SbmInvert:
		ss << L"Invert" << Endl;
		break;
	case SbmAlpha:
		ss << L"Alpha" << Endl;
		break;
	case SbmErase:
		ss << L"Erase" << Endl;
		break;
	case SbmOverlay:
		ss << L"Overlay" << Endl;
		break;
	case SbmHardlight:
		ss << L"Hard light" << Endl;
		break;
	default:
		ss << L"UNKNOWN" << Endl;
		break;
	}
	m_treeMovie->createItem(characterItem, ss.str());

	const Aabb2& bounds = characterInstance->getBounds();
	ss.reset();
	ss << L"Bounds: " << bounds.mn << L" - " << bounds.mx << IncreaseIndent;
	m_treeMovie->createItem(characterItem, ss.str());

	if (SpriteInstance* spriteInstance = dynamic_type_cast< SpriteInstance* >(characterInstance))
	{
		const Sprite* sprite = spriteInstance->getSprite();
		if (sprite)
		{
			uint32_t frameCount = sprite->getFrameCount();
			m_treeMovie->createItem(characterItem, toString(frameCount) + L" frame(s)");

			Ref< ui::custom::TreeViewItem > labelsItem = m_treeMovie->createItem(characterItem, L"Label(s)", 0, 0);
			for (uint32_t i = 0; i < frameCount; ++i)
			{
				const Frame* frame = sprite->getFrame(i);
				if (frame)
				{
					const std::string& label = frame->getLabel();
					if (!label.empty())
						m_treeMovie->createItem(labelsItem, toString(i) + L". \"" + mbstows(label) + L"\"");
				}
			}
		}

		Ref< ui::custom::TreeViewItem > layersItem = m_treeMovie->createItem(characterItem, L"Layer(s)", 0, 0);

		const DisplayList::layer_map_t& layers = spriteInstance->getDisplayList().getLayers();
		for (DisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
		{
			ss.reset();
			ss << i->first;
			if (i->second.clipDepth != 0)
				ss << L", Clip depth: " << i->second.clipDepth << Endl;
			Ref< ui::custom::TreeViewItem > layerItem = m_treeMovie->createItem(layersItem, ss.str(), 0, 0);
			T_ASSERT (layerItem);

			if (i->second.instance)
				updateTreeCharacter(layerItem, i->second.instance, pointerHash, nextPointerHash);

			layerItem->setData(L"CHARACTER", i->second.instance);
		}
	}
	else if (ButtonInstance* buttonInstance = dynamic_type_cast< ButtonInstance* >(characterInstance))
	{
		const Button* button = buttonInstance->getButton();
		uint8_t buttonState = buttonInstance->getState();

		m_treeMovie->createItem(characterItem, L"State " + toString(int32_t(buttonState)));

		Ref< ui::custom::TreeViewItem > layersItem = m_treeMovie->createItem(characterItem, L"Layers(s)", 0, 0);

		const Button::button_layers_t& layers = button->getButtonLayers();
		for (int32_t j = 0; j < int32_t(layers.size()); ++j)
		{
			const Button::ButtonLayer& layer = layers[j];

			Ref< ui::custom::TreeViewItem > layerItem = m_treeMovie->createItem(layersItem, toString(j) + L": " + toString(int32_t(layer.state)), 0, 0);
			T_ASSERT (layerItem);

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
			Ref< ui::custom::TreeViewItem > styleItem = m_treeMovie->createItem(characterItem, ss.str());

			for (uint32_t i = 0; i < uint32_t(fillStyles.size()); ++i)
			{
				const FillStyle& fs = fillStyles[i];
				const AlignedVector< FillStyle::ColorRecord >& cr = fs.getColorRecords();

				ss.reset();
				ss << (cr[0].color.getRed()) << L", " << (cr[0].color.getGreen()) << L", " << (cr[0].color.getBlue()) << L", " << (cr[0].color.getAlpha());
				m_treeMovie->createItem(styleItem, ss.str());
			}
		}

		const AlignedVector< Path >& paths = shape->getPaths();
		if (!paths.empty())
		{
			Ref< ui::custom::TreeViewItem > pathsItem = m_treeMovie->createItem(characterItem, L"Path(s)", 0, 0);
			for (uint32_t i = 0; i < uint32_t(paths.size()); ++i)
			{
				const Path& p = paths[i];

				ss.reset();
				ss << i << L". " << int32_t(p.getPoints().size()) << L" point(s), " << int32_t(p.getSubPaths().size()) << L" subpath(s)";

				Ref< ui::custom::TreeViewItem > pathItem = m_treeMovie->createItem(pathsItem, ss.str());
				pathItem->setData(L"CHARACTER", characterInstance);
				pathItem->setData(L"PATH", new PropertyInteger(i));
			}
		}
	}

	if (asObject)
	{
		Ref< ui::custom::TreeViewItem > membersItem = m_treeMovie->createItem(characterItem, L"Member(s)", 0, 0);
		std::set< const ActionObject* > objectStack;
		updateTreeObject(membersItem, asObject, objectStack, pointerHash, nextPointerHash);
	}
}

void FlashEditorPage::updateTreeMovie()
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

				Ref< ui::custom::TreeViewItem > memberItemRoot = m_treeMovie->createItem(0, L"_root", 0, 0);
				updateTreeCharacter(memberItemRoot, movieInstance, pointerHash, nextPointerHash);

				ActionContext* actionContext = movieInstance->getContext();
				T_ASSERT (actionContext);

				ActionObject* global = actionContext->getGlobal();
				T_ASSERT (global);

				Ref< ui::custom::TreeViewItem > memberItemGlobal = m_treeMovie->createItem(0, L"_global", 0, 0);
				std::set< const ActionObject* > objectStack;
				updateTreeObject(memberItemGlobal, global, objectStack, pointerHash, nextPointerHash);

				Ref< ui::custom::TreeViewItem > memberItemExports = m_treeMovie->createItem(0, L"Export(s)", 0, 0);
				const SmallMap< std::string, uint16_t >& exports = m_movie->getExports();
				for (SmallMap< std::string, uint16_t >::const_iterator i = exports.begin(); i != exports.end(); ++i)
					m_treeMovie->createItem(memberItemExports, toString(i->second) + L": \"" + mbstows(i->first) + L"\"");

				log::debug << L"Last object index @" << nextPointerHash << Endl;
			}
		}
	}
	else if (m_selectedCharacterInstance)
	{
		m_selectedCharacterInstance->setColorTransform(m_selectedCharacterInstanceCxForm);
		m_selectedCharacterInstance = 0;
		m_previewControl->update();
	}
}

void FlashEditorPage::eventToolClick(ui::custom::ToolBarButtonClickEvent* event)
{
	const ui::Command& command = event->getCommand();
	handleCommand(command);
}

void FlashEditorPage::eventTreeMovieSelect(ui::SelectionChangeEvent* event)
{
	RefArray< ui::custom::TreeViewItem > selectedItems;
	m_treeMovie->getItems(selectedItems, ui::custom::TreeView::GfDescendants | ui::custom::TreeView::GfSelectedOnly);

	if (m_selectedCharacterInstance)
	{
		m_selectedCharacterInstance->setColorTransform(m_selectedCharacterInstanceCxForm);
		m_selectedCharacterInstance->setBlendMode(m_selectedCharacterInstanceBlendMode);
		m_selectedCharacterInstance = 0;
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

			const PropertyInteger* pathIndex = selectedItems[0]->getData< PropertyInteger >(L"PATH");
			if (pathIndex)
			{
				ShapeInstance* shapeInstance = checked_type_cast< ShapeInstance*, false >(m_selectedCharacterInstance);
				const AlignedVector< Path >& paths = shapeInstance->getShape()->getPaths();
				m_pathControl->setPath(paths[PropertyInteger::get(pathIndex)]);
			}
		}
	}

	m_previewControl->update();
}

	}
}
