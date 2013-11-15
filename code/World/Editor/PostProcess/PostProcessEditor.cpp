#include "Database/Instance.h"
#include "Ui/Container.h"
#include "Ui/Event.h"
#include "Ui/FloodLayout.h"
#include "Ui/MethodHandler.h"
#include "Ui/Custom/Splitter.h"
#include "World/PostProcess/PostProcessDefineTarget.h"
#include "World/PostProcess/PostProcessSettings.h"
#include "World/PostProcess/PostProcessStepBlur.h"
#include "World/PostProcess/PostProcessStepBokeh.h"
#include "World/PostProcess/PostProcessStepChain.h"
#include "World/PostProcess/PostProcessStepGodRay.h"
#include "World/PostProcess/PostProcessStepGrain.h"
#include "World/PostProcess/PostProcessStepLuminance.h"
#include "World/PostProcess/PostProcessStepRepeat.h"
#include "World/PostProcess/PostProcessStepSetTarget.h"
#include "World/PostProcess/PostProcessStepSimple.h"
#include "World/PostProcess/PostProcessStepSmProj.h"
#include "World/PostProcess/PostProcessStepSsao.h"
#include "World/PostProcess/PostProcessStepSwapTargets.h"
#include "World/Editor/PostProcess/PostProcessDefineItem.h"
#include "World/Editor/PostProcess/PostProcessDefineTargetFacade.h"
#include "World/Editor/PostProcess/PostProcessDefineView.h"
#include "World/Editor/PostProcess/PostProcessEditor.h"
#include "World/Editor/PostProcess/PostProcessStepBlurFacade.h"
#include "World/Editor/PostProcess/PostProcessStepBokehFacade.h"
#include "World/Editor/PostProcess/PostProcessStepChainFacade.h"
#include "World/Editor/PostProcess/PostProcessStepGodRayFacade.h"
#include "World/Editor/PostProcess/PostProcessStepGrainFacade.h"
#include "World/Editor/PostProcess/PostProcessStepLuminanceFacade.h"
#include "World/Editor/PostProcess/PostProcessStepRepeatFacade.h"
#include "World/Editor/PostProcess/PostProcessStepSetTargetFacade.h"
#include "World/Editor/PostProcess/PostProcessStepSimpleFacade.h"
#include "World/Editor/PostProcess/PostProcessStepSmProjFacade.h"
#include "World/Editor/PostProcess/PostProcessStepSsaoFacade.h"
#include "World/Editor/PostProcess/PostProcessStepSwapTargetsFacade.h"
#include "World/Editor/PostProcess/PostProcessStepItem.h"
#include "World/Editor/PostProcess/PostProcessStepProperties.h"
#include "World/Editor/PostProcess/PostProcessView.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.PostProcessEditor", PostProcessEditor, editor::IObjectEditor)

PostProcessEditor::PostProcessEditor(editor::IEditor* editor)
:	m_editor(editor)
{
}

bool PostProcessEditor::create(ui::Widget* parent, db::Instance* instance, ISerializable* object)
{
	m_instance = instance;
	if (!m_instance)
		return false;

	m_asset = dynamic_type_cast< PostProcessSettings* >(object);
	if (!m_asset)
		return false;

	Ref< ui::custom::Splitter > splitter = new ui::custom::Splitter();
	splitter->create(parent, true, 240, false);

	Ref< ui::custom::Splitter > splitterView = new ui::custom::Splitter();
	splitterView->create(splitter, false, 60, true);

	m_postProcessView = new PostProcessView();
	m_postProcessView->create(splitterView);
	m_postProcessView->addEventHandler(ui::EiSelectionChange, ui::createMethodHandler(this, &PostProcessEditor::eventStepSelect));
	//m_postProcessView->addButtonUpEventHandler(ui::createMethodHandler(this, &PostProcessEditor::eventGrainButtonUp));

	m_postProcessDefineView = new PostProcessDefineView();
	m_postProcessDefineView->create(splitterView);

	Ref< ui::Container > container = new ui::Container();
	container->create(splitter, ui::WsClientBorder, new ui::FloodLayout());

	m_postProcessProperties = new PostProcessStepProperties(m_editor);
	m_postProcessProperties->create(container);
	m_postProcessProperties->addEventHandler(ui::EiUser + 1, ui::createMethodHandler(this, &PostProcessEditor::eventStepPropertiesChange));

	// Create define facades.
	m_postProcessDefineFacades[&type_of < PostProcessDefineTarget >()] = new PostProcessDefineTargetFacade();

	// Create step facades.
	m_postProcessStepFacades[&type_of< PostProcessStepBlur >()] = new PostProcessStepBlurFacade();
	m_postProcessStepFacades[&type_of< PostProcessStepBokeh >()] = new PostProcessStepBokehFacade();
	m_postProcessStepFacades[&type_of< PostProcessStepChain >()] = new PostProcessStepChainFacade();
	m_postProcessStepFacades[&type_of< PostProcessStepGodRay >()] = new PostProcessStepGodRayFacade();
	m_postProcessStepFacades[&type_of< PostProcessStepGrain >()] = new PostProcessStepGrainFacade();
	m_postProcessStepFacades[&type_of< PostProcessStepLuminance >()] = new PostProcessStepLuminanceFacade();
	m_postProcessStepFacades[&type_of< PostProcessStepRepeat >()] = new PostProcessStepRepeatFacade();
	m_postProcessStepFacades[&type_of< PostProcessStepSetTarget >()] = new PostProcessStepSetTargetFacade();
	m_postProcessStepFacades[&type_of< PostProcessStepSimple >()] = new PostProcessStepSimpleFacade();
	m_postProcessStepFacades[&type_of< PostProcessStepSmProj >()] = new PostProcessStepSmProjFacade();
	m_postProcessStepFacades[&type_of< PostProcessStepSsao >()] = new PostProcessStepSsaoFacade();
	m_postProcessStepFacades[&type_of< PostProcessStepSwapTargets >()] = new PostProcessStepSwapTargetsFacade();

	updateViews();
	return true;
}

void PostProcessEditor::destroy()
{
}

void PostProcessEditor::apply()
{
	m_instance->setObject(m_asset);
}

ui::Size PostProcessEditor::getPreferredSize() const
{
	return ui::Size(900, 600);
}

bool PostProcessEditor::handleCommand(const ui::Command& command)
{
	return false;
}

void PostProcessEditor::updateStepView(PostProcessStepItem* parent, const RefArray< PostProcessStep >& steps)
{
	for (RefArray< PostProcessStep >::const_iterator i = steps.begin(); i != steps.end(); ++i)
	{
		const IPostProcessStepFacade* stepFacade = m_postProcessStepFacades[&type_of(*i)];
		if (!stepFacade)
			continue;

		Ref< PostProcessStepItem > item = new PostProcessStepItem(
			parent,
			*i,
			stepFacade->getText(*i),
			stepFacade->getImage(*i)
		);
		m_postProcessView->add(item);

		RefArray< PostProcessStep > children;
		if (stepFacade->getChildren(*i, children))
			updateStepView(item, children);
	}
}

void PostProcessEditor::updateViews()
{
	m_postProcessView->removeAll();
	m_postProcessDefineView->removeAll();

	const RefArray< PostProcessStep >& steps = m_asset->getSteps();
	updateStepView(0, steps);

	// Add implicit, read-only, definitions.
	m_postProcessDefineView->add(new PostProcessDefineItem(0, L"Output", 0));
	m_postProcessDefineView->add(new PostProcessDefineItem(0, L"InputColor", 0));
	m_postProcessDefineView->add(new PostProcessDefineItem(0, L"InputDepth", 0));
	m_postProcessDefineView->add(new PostProcessDefineItem(0, L"InputShadowMask", 0));

	// Add user definitions.
	const RefArray< PostProcessDefine >& definitions = m_asset->getDefinitions();
	for (RefArray< PostProcessDefine >::const_iterator i = definitions.begin(); i != definitions.end(); ++i)
	{
		const IPostProcessDefineFacade* defineFacade = m_postProcessDefineFacades[&type_of(*i)];
		if (!defineFacade)
			continue;

		m_postProcessDefineView->add(new PostProcessDefineItem(
			*i,
			defineFacade->getText(*i),
			defineFacade->getImage(*i)
		));
	}

	m_postProcessView->update();
	m_postProcessDefineView->update();
}

void PostProcessEditor::eventStepSelect(ui::Event* event)
{
	PostProcessStepItem* item = checked_type_cast< PostProcessStepItem* >(event->getItem());
	if (item)
		m_postProcessProperties->set(item->getStep());
	else
		m_postProcessProperties->set(0);
}

void PostProcessEditor::eventStepPropertiesChange(ui::Event* event)
{
	updateViews();
}

	}
}
