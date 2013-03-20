#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "Editor/TypeBrowseFilter.h"
#include "Resource/ResourceManager.h"
#include "Sound/Sound.h"
#include "Sound/SoundChannel.h"
#include "Sound/SoundFactory.h"
#include "Sound/SoundSystem.h"
#include "Sound/Resound/BankBuffer.h"
#include "Sound/Resound/BlendGrainData.h"
#include "Sound/Resound/EnvelopeGrainData.h"
#include "Sound/Resound/InLoopOutGrainData.h"
#include "Sound/Resound/MuteGrainData.h"
#include "Sound/Resound/PlayGrainData.h"
#include "Sound/Resound/RandomGrainData.h"
#include "Sound/Resound/RepeatGrainData.h"
#include "Sound/Resound/SequenceGrainData.h"
#include "Sound/Resound/SimultaneousGrainData.h"
#include "Sound/Resound/TriggerGrainData.h"
#include "Sound/Editor/SoundAsset.h"
#include "Sound/Editor/Resound/BankAsset.h"
#include "Sound/Editor/Resound/BankAssetEditor.h"
#include "Sound/Editor/Resound/BlendGrainFacade.h"
#include "Sound/Editor/Resound/EnvelopeGrainFacade.h"
#include "Sound/Editor/Resound/InLoopOutGrainFacade.h"
#include "Sound/Editor/Resound/GrainProperties.h"
#include "Sound/Editor/Resound/GrainView.h"
#include "Sound/Editor/Resound/GrainViewItem.h"
#include "Sound/Editor/Resound/MuteGrainFacade.h"
#include "Sound/Editor/Resound/PlayGrainFacade.h"
#include "Sound/Editor/Resound/RandomGrainFacade.h"
#include "Sound/Editor/Resound/RepeatGrainFacade.h"
#include "Sound/Editor/Resound/SequenceGrainFacade.h"
#include "Sound/Editor/Resound/SimultaneousGrainFacade.h"
#include "Sound/Editor/Resound/TriggerGrainFacade.h"
#include "Ui/Container.h"
#include "Ui/PopupMenu.h"
#include "Ui/MenuItem.h"
#include "Ui/MethodHandler.h"
#include "Ui/Slider.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"
#include "Ui/Events/CommandEvent.h"
#include "Ui/Events/MouseEvent.h"
#include "Ui/Custom/Panel.h"
#include "Ui/Custom/Splitter.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

class HandleWrapper : public Object
{
	T_RTTI_CLASS;

public:
	HandleWrapper(int32_t id) : m_id(id) {}

	int32_t get() const { return m_id; }

private:
	int32_t m_id;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.BankAssetEditor.HandleWrapper", HandleWrapper, Object)

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.BankAssetEditor", BankAssetEditor, editor::IObjectEditor)

BankAssetEditor::BankAssetEditor(editor::IEditor* editor)
:	m_editor(editor)
{
}

bool BankAssetEditor::create(ui::Widget* parent, db::Instance* instance, ISerializable* object)
{
	m_instance = instance;
	if (!m_instance)
		return false;

	m_asset = dynamic_type_cast< BankAsset* >(object);
	if (!m_asset)
		return false;

	parent->addTimerEventHandler(ui::createMethodHandler(this, &BankAssetEditor::eventTimer));
	parent->startTimer(100);

	Ref< ui::custom::Splitter > splitter = new ui::custom::Splitter();
	splitter->create(parent, true, 40, true);

	Ref< ui::custom::Splitter > splitter2 = new ui::custom::Splitter();
	splitter2->create(splitter, false, -150);

	Ref< ui::custom::Panel > containerGrains = new ui::custom::Panel();
	containerGrains->create(splitter2, L"Grains", new ui::TableLayout(L"100%", L"*,100%", 0, 0));

	m_toolBarItemPlay = new ui::custom::ToolBarButton(L"Play", ui::Command(L"Bank.PlayGrain"), 0, ui::custom::ToolBarButton::BsText | ui::custom::ToolBarButton::BsToggle);
	m_toolBarItemRepeat = new ui::custom::ToolBarButton(L"Repeat", ui::Command(L"Bank.RepeatGrain"), 0, ui::custom::ToolBarButton::BsText | ui::custom::ToolBarButton::BsToggle);

	m_toolBar = new ui::custom::ToolBar();
	m_toolBar->create(containerGrains);
	m_toolBar->addItem(m_toolBarItemPlay);
	m_toolBar->addItem(m_toolBarItemRepeat);
	m_toolBar->addClickEventHandler(ui::createMethodHandler(this, &BankAssetEditor::eventToolBarClick));

	m_grainView = new GrainView();
	m_grainView->create(containerGrains);
	m_grainView->addEventHandler(ui::EiSelectionChange, ui::createMethodHandler(this, &BankAssetEditor::eventGrainSelect));
	m_grainView->addButtonUpEventHandler(ui::createMethodHandler(this, &BankAssetEditor::eventGrainButtonUp));

	m_containerDynamicParameters = new ui::custom::Panel();
	m_containerDynamicParameters->create(splitter2, L"Dynamic Parameters", new ui::TableLayout(L"*,100%", L"*", 4, 0));

	m_containerGrainProperties = new ui::Container();
	m_containerGrainProperties->create(splitter, ui::WsClientBorder, new ui::TableLayout(L"100%", L"100%", 0, 4));

	m_grainProperties = new GrainProperties(m_editor);
	m_grainProperties->create(m_containerGrainProperties);
	m_grainProperties->addEventHandler(ui::EiUser + 1, ui::createMethodHandler(this, &BankAssetEditor::eventGrainPropertiesChange));

	m_menuGrains = new ui::PopupMenu();
	m_menuGrains->create();
	m_menuGrains->add(new ui::MenuItem(ui::Command(L"Bank.AddGrain"), L"Add grain..."));
	m_menuGrains->add(new ui::MenuItem(ui::Command(L"Bank.RemoveGrain"), L"Remove grain..."));

	// Create grain editor facades.
	m_grainFacades[&type_of< BlendGrainData >()] = new BlendGrainFacade();
	m_grainFacades[&type_of< EnvelopeGrainData >()] = new EnvelopeGrainFacade();
	m_grainFacades[&type_of< InLoopOutGrainData >()] = new InLoopOutGrainFacade();
	m_grainFacades[&type_of< MuteGrainData >()] = new MuteGrainFacade();
	m_grainFacades[&type_of< PlayGrainData >()] = new PlayGrainFacade();
	m_grainFacades[&type_of< RandomGrainData >()] = new RandomGrainFacade();
	m_grainFacades[&type_of< RepeatGrainData >()] = new RepeatGrainFacade();
	m_grainFacades[&type_of< SequenceGrainData >()] = new SequenceGrainFacade();
	m_grainFacades[&type_of< SimultaneousGrainData >()] = new SimultaneousGrainFacade();
	m_grainFacades[&type_of< TriggerGrainData >()] = new TriggerGrainFacade();

	// Get sound system for preview.
	m_soundSystem = m_editor->getStoreObject< SoundSystem >(L"SoundSystem");
	if (m_soundSystem)
	{
		m_soundChannel = m_soundSystem->getChannel(0);
		if (!m_soundChannel)
			m_soundSystem = 0;
	}
	if (!m_soundSystem)
		log::warning << L"Unable to create preview sound system; preview unavailable" << Endl;

	m_resourceManager = new resource::ResourceManager(true);
	m_resourceManager->addFactory(new SoundFactory(
		m_editor->getOutputDatabase()
	));

	updateGrainView();
	updateProperties();
	return true;
}

void BankAssetEditor::destroy()
{
	if (m_soundChannel)
	{
		m_soundChannel->stop();
		m_soundChannel = 0;
	}

	if (m_resourceManager)
		m_resourceManager = 0;

	m_soundSystem = 0;
	safeDestroy(m_menuGrains);
}

void BankAssetEditor::apply()
{
	m_instance->setObject(m_asset);
}

bool BankAssetEditor::handleCommand(const ui::Command& command)
{
	if (command == L"Bank.AddGrain")
	{
		IGrainData* parentGrain = 0;
		IGrainFacade* parentGrainFacade = 0;

		GrainViewItem* selectedItem = m_grainView->getSelected();
		if (selectedItem)
		{
			parentGrain = selectedItem->getGrain();
			T_ASSERT (parentGrain);

			parentGrainFacade = m_grainFacades[&type_of(parentGrain)];
			if (!parentGrainFacade || !parentGrainFacade->canHaveChildren())
				return true;
		}

		const TypeInfo* grainType = m_editor->browseType(&type_of< IGrainData >());
		if (grainType)
		{
			Ref< IGrainData > grain = checked_type_cast< IGrainData*, false >(grainType->createInstance());

			if (parentGrainFacade)
				parentGrainFacade->addChild(parentGrain, grain);
			else
				m_asset->addGrain(grain);

			updateGrainView();
			updateProperties();
		}
	}
	else if (command == L"Bank.RemoveGrain")
	{
		GrainViewItem* selectedItem = m_grainView->getSelected();
		if (selectedItem)
		{
			IGrainData* grain = selectedItem->getGrain();
			T_ASSERT (grain);

			if (selectedItem->getParent())
			{
				IGrainData* parentGrain = selectedItem->getParent()->getGrain();
				T_ASSERT (parentGrain);

				IGrainFacade* parentGrainFacade = m_grainFacades[&type_of(parentGrain)];
				if (!parentGrainFacade || !parentGrainFacade->canHaveChildren())
					return true;

				parentGrainFacade->removeChild(parentGrain, grain);
			}
			else
				m_asset->removeGrain(grain);

			updateGrainView();
			updateProperties();
		}
	}
	else if (command == L"Bank.PlayGrain")
	{
		if (m_soundChannel && !m_soundChannel->isPlaying())
		{
			RefArray< IGrainData > grainData;
			RefArray< IGrain > grains;

			// Play only selected grain.
			GrainViewItem* selectedItem = m_grainView->getSelected();
			if (selectedItem)
			{
				IGrainData* grain = selectedItem->getGrain();
				T_ASSERT (grain);

				grainData.push_back(grain);
			}
			else
			{
				grainData = m_asset->getGrains();
			}

			// Create grains from data.
			for (uint32_t i = 0; i < grainData.size(); ++i)
			{
				Ref< IGrain > grain = grainData[i]->createInstance(m_resourceManager);
				if (grain)
					grains.push_back(grain);
				else
					log::warning << L"Unable to create grain " << i << L" of type " << type_name(grainData[i]) << Endl;
			}

			if (!grains.empty())
			{
				m_bankBuffer = new BankBuffer(grains);
				m_soundChannel->play(m_bankBuffer, 1.0f, 0.0f, 1.0f);

				for (RefArray< ui::Slider >::const_iterator i = m_sliderParameters.begin(); i != m_sliderParameters.end(); ++i)
				{
					const HandleWrapper* id = (*i)->getData< HandleWrapper >(L"ID");
					T_ASSERT (id);

					m_soundChannel->setParameter(
						id->get(),
						(*i)->getValue() / 100.0f
					);
				}
			}
			else
				log::error << L"No grains to play" << Endl;
		}
		else if (m_soundChannel && m_soundChannel->isPlaying())
		{
			m_soundChannel->stop();
			m_toolBarItemPlay->setToggled(false);
			m_toolBar->update();
		}
	}
	else
		return false;

	return true;
}

ui::Size BankAssetEditor::getPreferredSize() const
{
	return ui::Size(800, 600);
}

void BankAssetEditor::updateGrainView(GrainViewItem* parent, const RefArray< IGrainData >& grains)
{
	for (RefArray< IGrainData >::const_iterator i = grains.begin(); i != grains.end(); ++i)
	{
		if (!(*i))
			continue;

		IGrainFacade* grainFacade = m_grainFacades[&type_of(*i)];
		if (!grainFacade)
			continue;

		Ref< GrainViewItem > item = new GrainViewItem(
			parent,
			*i,
			grainFacade->getText(*i),
			grainFacade->getImage(*i)
		);
		m_grainView->add(item);

		RefArray< IGrainData > childGrains;
		if (grainFacade->getChildren(*i, childGrains))
			updateGrainView(item, childGrains);
	}
}

void BankAssetEditor::updateGrainView()
{
	m_grainView->removeAll();

	const RefArray< IGrainData >& grains = m_asset->getGrains();
	updateGrainView(0, grains);

	m_grainView->update();
}

void BankAssetEditor::updateProperties()
{
	std::set< std::wstring > properties;

	// Scan grains for all dynamic properties.
	RefArray< IGrainData > grains = m_asset->getGrains();
	while (!grains.empty())
	{
		Ref< IGrainData > grain = grains.front();
		grains.pop_front();

		if (!grain)
			continue;

		IGrainFacade* grainFacade = m_grainFacades[&type_of(grain)];
		if (grainFacade)
		{
			grainFacade->getProperties(grain, properties);

			RefArray< IGrainData > childGrains;
			if (grainFacade->getChildren(grain, childGrains))
				grains.insert(grains.end(), childGrains.begin(), childGrains.end());
		}
	}

	// Destroy previous sliders.
	m_sliderParameters.clear();
	while (m_containerDynamicParameters->getLastChild())
		m_containerDynamicParameters->getLastChild()->destroy();

	// Create slider for each dynamic property.
	for (std::set< std::wstring >::const_iterator i = properties.begin(); i != properties.end(); ++i)
	{
		if (i->empty())
			continue;

		Ref< ui::Static > staticParameter = new ui::Static();
		staticParameter->create(m_containerDynamicParameters, *i);

		Ref< ui::Slider > sliderParameter = new ui::Slider();
		sliderParameter->create(m_containerDynamicParameters);
		sliderParameter->setRange(0, 100);
		sliderParameter->addChangeEventHandler(ui::createMethodHandler(this, &BankAssetEditor::eventParameterChange));
		sliderParameter->setData(L"ID", new HandleWrapper(
			getParameterHandle(*i)
		));

		m_sliderParameters.push_back(sliderParameter);
	}

	m_containerDynamicParameters->update();
}

void BankAssetEditor::eventParameterChange(ui::Event* event)
{
	const ui::Slider* slider = checked_type_cast< const ui::Slider*, false >(event->getSender());
	const HandleWrapper* id = slider->getData< HandleWrapper >(L"ID");
	T_ASSERT (id);

	if (m_soundChannel)
		m_soundChannel->setParameter(id->get(), slider->getValue() / 100.0f);
}

void BankAssetEditor::eventToolBarClick(ui::Event* event)
{
	ui::CommandEvent* commandEvent = checked_type_cast< ui::CommandEvent*, false >(event);
	handleCommand(commandEvent->getCommand());
}

void BankAssetEditor::eventGrainSelect(ui::Event* event)
{
	GrainViewItem* item = checked_type_cast< GrainViewItem* >(event->getItem());
	if (item)
		m_grainProperties->set(item->getGrain());
	else
		m_grainProperties->set(0);
}

void BankAssetEditor::eventGrainButtonUp(ui::Event* event)
{
	ui::MouseEvent* mouseEvent = checked_type_cast< ui::MouseEvent*, false >(event);
	if (mouseEvent->getButton() == ui::MouseEvent::BtRight)
	{
		Ref< ui::MenuItem > selectedItem = m_menuGrains->show(m_grainView, mouseEvent->getPosition());
		if (selectedItem)
			handleCommand(selectedItem->getCommand());
		mouseEvent->consume();
	}
}

void BankAssetEditor::eventGrainPropertiesChange(ui::Event* event)
{
	// Stop playing if properties has changed, need to reflect changes
	// without interference otherwise filter instances will be incorrect.
	if (m_soundChannel && m_bankBuffer)
	{
		ISoundBufferCursor* cursor = m_soundChannel->getCursor();
		if (cursor)
			m_bankBuffer->updateCursor(cursor);
	}

	updateGrainView();
	updateProperties();
}

void BankAssetEditor::eventTimer(ui::Event* event)
{
	if (!m_soundChannel)
		return;

	if (!m_soundChannel->isPlaying() && m_toolBarItemPlay->isToggled())
	{
		if (!m_toolBarItemRepeat->isToggled())
		{
			m_toolBarItemPlay->setToggled(false);
			m_toolBar->update();
		}
		else
		{
			T_ASSERT (m_bankBuffer);
			m_soundChannel->play(m_bankBuffer, 1.0f, 0.0f, 1.0f);

			for (RefArray< ui::Slider >::const_iterator i = m_sliderParameters.begin(); i != m_sliderParameters.end(); ++i)
			{
				const HandleWrapper* id = (*i)->getData< HandleWrapper >(L"ID");
				T_ASSERT (id);

				m_soundChannel->setParameter(
					id->get(),
					(*i)->getValue() / 100.0f
				);
			}
		}
	}
}

	}
}
