#include <cmath>
#include "Animation/Joint.h"
#include "Animation/Skeleton.h"
#include "Animation/SkeletonUtils.h"
#include "Animation/Animation/Animation.h"
#include "Animation/Animation/AnimationFactory.h"
#include "Animation/Editor/AnimationEditorPage.h"
#include "Animation/Editor/VolumePicker.h"
#include "Animation/IK/IKPoseController.h"
#include "Core/Settings/PropertyColor.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Database/Instance.h"
#include "Editor/IDocument.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPageSite.h"
#include "Editor/TypeBrowseFilter.h"
#include "Resource/ResourceManager.h"
#include "Ui/Application.h"
#include "Ui/Clipboard.h"
#include "Ui/Container.h"
#include "Ui/PopupMenu.h"
#include "Ui/MenuItem.h"
#include "Ui/TableLayout.h"
#include "Ui/Bitmap.h"
#include "Ui/Custom/QuadSplitter.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/Custom/ToolBar/ToolBarSeparator.h"
#include "Ui/Custom/Sequencer/CursorMoveEvent.h"
#include "Ui/Custom/Sequencer/SequencerControl.h"
#include "Ui/Custom/Sequencer/Sequence.h"
#include "Ui/Custom/Sequencer/Tick.h"
#include "Ui/Itf/IWidget.h"
#include "I18N/Text.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/PrimitiveRenderer.h"
#include "Render/Resource/TextureFactory.h"
#include "Render/Resource/ShaderFactory.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Math/Const.h"
#include "Core/Math/Vector2.h"
#include "Core/Log/Log.h"

// Resources
#include "Resources/Playback.h"
#include "Resources/Skeleton.h"

namespace traktor
{
	namespace animation
	{
		namespace
		{

const float c_fov = 80.0f;
const float c_orthoSize = 100.0f;
const float c_orthoSizeMin = 10.0f;
const float c_nearZ = 0.1f;
const float c_farZ = 1000.0f;
const float c_wheelRotationDelta = -10.0f;

class RenderWidgetData : public Object
{
	T_RTTI_CLASS;

public:
	RenderWidgetData()
	:	orthogonal(false)
	,	size(c_orthoSize)
	,	cameraAngleX(0.0f)
	,	cameraAngleY(0.0f)
	,	cameraOffset(0.0f, 0.0f, 100.0f)
	{
	}

	Ref< render::IRenderView > renderView;
	bool orthogonal;
	float size;
	float cameraAngleX;
	float cameraAngleY;
	Vector4 cameraOffset;
	Ref< VolumePicker > picker;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.RenderWidgetData", RenderWidgetData, Object)

class KeyPoseClipboardData : public ISerializable
{
	T_RTTI_CLASS;

public:
	KeyPoseClipboardData()
	{
	}

	KeyPoseClipboardData(const Animation::KeyPose& pose)
	:	m_pose(pose)
	{
	}

	const Animation::KeyPose& getPose() const
	{
		return m_pose;
	}

	virtual void serialize(ISerializer& s)
	{
		s >> MemberComposite< Animation::KeyPose >(L"pose", m_pose);
	}

private:
	Animation::KeyPose m_pose;
};

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.KeyPoseClipboardData", 0, KeyPoseClipboardData, ISerializable)

class PoseIdData : public Object
{
	T_RTTI_CLASS;

public:
	int m_id;

	PoseIdData(int id)
	:	m_id(id)
	{
	}
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.PoseIdData", PoseIdData, Object)

		}

const int c_animationLength = 10000;

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.AnimationEditorPage", AnimationEditorPage, editor::IEditorPage)

AnimationEditorPage::AnimationEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document)
:	m_editor(editor)
,	m_site(site)
,	m_document(document)
,	m_selectedBone(0)
,	m_showGhostTrail(false)
,	m_twistLock(false)
,	m_ikEnabled(false)
,	m_relativeTwist(0.0f)
,	m_haveRelativeTwist(0.0f)
,	m_editMode(false)
,	m_previewAnimation(false)
,	m_cameraOffsetScale(1.0f, 1.0f, 1.0f, 1.0f)
,	m_cameraSizeScale(1.0f)
{
}

bool AnimationEditorPage::create(ui::Container* parent)
{
	render::IRenderSystem* renderSystem = m_editor->getStoreObject< render::IRenderSystem >(L"RenderSystem");
	if (!renderSystem)
		return false;

	m_animation = m_document->getObject< Animation >(0);
	if (!m_animation)
		return false;

	Ref< ui::custom::QuadSplitter > splitter = new ui::custom::QuadSplitter();
	splitter->create(parent, ui::Point(50, 50), true);

	for (int i = 0; i < 4; ++i)
	{
		m_renderWidgets[i] = new ui::Widget();
		m_renderWidgets[i]->create(splitter, ui::WsClientBorder);
		m_renderWidgets[i]->addEventHandler< ui::MouseButtonDownEvent >(this, &AnimationEditorPage::eventRenderButtonDown);
		m_renderWidgets[i]->addEventHandler< ui::MouseButtonUpEvent >(this, &AnimationEditorPage::eventRenderButtonUp);
		m_renderWidgets[i]->addEventHandler< ui::MouseMoveEvent >(this, &AnimationEditorPage::eventRenderMouseMove);
		m_renderWidgets[i]->addEventHandler< ui::MouseWheelEvent >(this, &AnimationEditorPage::eventRenderMouseWheel);
		m_renderWidgets[i]->addEventHandler< ui::SizeEvent >(this, &AnimationEditorPage::eventRenderSize);
		m_renderWidgets[i]->addEventHandler< ui::PaintEvent >(this, &AnimationEditorPage::eventRenderPaint);
		
		Ref< RenderWidgetData > data = new RenderWidgetData();
		m_renderWidgets[i]->setData(L"DATA", data);
	}

	// Create sequencer panel.
	m_sequencerPanel = new ui::Container();
	m_sequencerPanel->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0));
	m_sequencerPanel->setText(i18n::Text(L"ANIMATION_EDITOR_SEQUENCER"));

	m_toolBarPlay = new ui::custom::ToolBar();
	m_toolBarPlay->create(m_sequencerPanel);
	m_toolBarPlay->addImage(ui::Bitmap::load(c_ResourcePlayback, sizeof(c_ResourcePlayback), L"image"), 6);
	m_toolBarPlay->addImage(ui::Bitmap::load(c_ResourceSkeleton, sizeof(c_ResourceSkeleton), L"image"), 5);
	m_toolBarPlay->addItem(new ui::custom::ToolBarButton(i18n::Text(L"ANIMATION_EDITOR_REWIND"), 0, ui::Command(L"Animation.Editor.Rewind")));
	m_toolBarPlay->addItem(new ui::custom::ToolBarButton(i18n::Text(L"ANIMATION_EDITOR_PLAY"), 1, ui::Command(L"Animation.Editor.Play")));
	m_toolBarPlay->addItem(new ui::custom::ToolBarButton(i18n::Text(L"ANIMATION_EDITOR_STOP"), 2, ui::Command(L"Animation.Editor.Stop")));
	m_toolBarPlay->addItem(new ui::custom::ToolBarButton(i18n::Text(L"ANIMATION_EDITOR_FORWARD"), 3, ui::Command(L"Animation.Editor.Forward")));
	m_toolBarPlay->addItem(new ui::custom::ToolBarSeparator());
	m_toolBarPlay->addItem(new ui::custom::ToolBarButton(i18n::Text(L"ANIMATION_EDITOR_BROWSE_SKELETON"), 6, ui::Command(L"Animation.Editor.BrowseSkeleton")));
	m_toolBarPlay->addItem(new ui::custom::ToolBarButton(i18n::Text(L"ANIMATION_EDITOR_BROWSE_SKIN"), 9, ui::Command(L"Animation.Editor.BrowseSkin")));
	m_toolBarPlay->addItem(new ui::custom::ToolBarSeparator());
	m_toolBarPlay->addItem(new ui::custom::ToolBarButton(i18n::Text(L"ANIMATION_EDITOR_TOGGLE_TRAIL"), 7, ui::Command(L"Animation.Editor.ToggleTrail"), m_showGhostTrail ? ui::custom::ToolBarButton::BsDefaultToggled : ui::custom::ToolBarButton::BsDefaultToggle));
	m_toolBarPlay->addItem(new ui::custom::ToolBarButton(i18n::Text(L"ANIMATION_EDITOR_TOGGLE_TWIST_LOCK"), 8, ui::Command(L"Animation.Editor.ToggleTwistLock"), m_twistLock ? ui::custom::ToolBarButton::BsDefaultToggled : ui::custom::ToolBarButton::BsDefaultToggle));
	m_toolBarPlay->addItem(new ui::custom::ToolBarButton(i18n::Text(L"ANIMATION_EDITOR_TOGGLE_IK"), 10, ui::Command(L"Animation.Editor.ToggleIK"), m_ikEnabled ? ui::custom::ToolBarButton::BsDefaultToggled : ui::custom::ToolBarButton::BsDefaultToggle));
	m_toolBarPlay->addEventHandler< ui::custom::ToolBarButtonClickEvent >(this, &AnimationEditorPage::eventToolClick);

	m_sequencer = new ui::custom::SequencerControl();
	m_sequencer->create(m_sequencerPanel);
	m_sequencer->setLength(c_animationLength);
	m_sequencer->addEventHandler< ui::MouseButtonDownEvent >(this, &AnimationEditorPage::eventSequencerButtonDown);
	m_sequencer->addEventHandler< ui::custom::CursorMoveEvent >(this, &AnimationEditorPage::eventSequencerCursorMove);
	m_sequencer->addEventHandler< ui::TimerEvent >(this, &AnimationEditorPage::eventSequencerTimer);
	m_sequencer->startTimer(30);

	m_site->createAdditionalPanel(m_sequencerPanel, ui::scaleBySystemDPI(100), true);

	// Build popup menu.
	m_menuPopup = new ui::PopupMenu();
	m_menuPopup->create();
	m_menuPopup->add(new ui::MenuItem(ui::Command(L"Animation.Editor.Create"), i18n::Text(L"ANIMATION_EDITOR_CREATE")));
	m_menuPopup->add(new ui::MenuItem(ui::Command(L"Editor.Delete"), i18n::Text(L"ANIMATION_EDITOR_DELETE")));

	// Create render views.
	const float c_cameraAngles[][2] =
	{
		{ 0.0f, 0.0f },
		{ 0.0f, 0.0f },
		{ PI / 2.0f, 0.0f },
		{ 0.0f, -PI / 2.0f }
	};

	for (int i = 0; i < sizeof_array(m_renderWidgets); ++i)
	{
		Ref< RenderWidgetData > data = m_renderWidgets[i]->getData< RenderWidgetData >(L"DATA");

		render::RenderViewEmbeddedDesc desc;
		desc.depthBits = 16;
		desc.stencilBits = 0;
		desc.multiSample = m_editor->getSettings()->getProperty< PropertyInteger >(L"Editor.MultiSample", 4);
		desc.waitVBlank = false;
		desc.nativeWindowHandle = m_renderWidgets[i]->getIWidget()->getSystemHandle();

		data->orthogonal = i > 0;
		data->cameraAngleX = c_cameraAngles[i][0];
		data->cameraAngleY = c_cameraAngles[i][1];

		data->renderView = renderSystem->createRenderView(desc);
		if (!data->renderView)
			return false;

		data->picker = new VolumePicker();
	}

	Ref< db::Database > database = m_editor->getOutputDatabase();

	m_resourceManager = new resource::ResourceManager(true);
	m_resourceManager->addFactory(
		new render::TextureFactory(database, renderSystem, 0)
	);
	m_resourceManager->addFactory(
		new render::ShaderFactory(database, renderSystem)
	);
	m_resourceManager->addFactory(
		new AnimationFactory(database)
	);

	m_primitiveRenderer = new render::PrimitiveRenderer();
	if (!m_primitiveRenderer->create(m_resourceManager, renderSystem))
		return false;

	updateSettings();
	updateSequencer();

	return true;
}

void AnimationEditorPage::destroy()
{
	m_site->destroyAdditionalPanel(m_sequencerPanel);

	// Destroy render widgets.
	for (int i = 0; i < sizeof_array(m_renderWidgets); ++i)
	{
		Ref< RenderWidgetData > data = m_renderWidgets[i]->getData< RenderWidgetData >(L"DATA");
		data->renderView->close();
		m_renderWidgets[i]->destroy();
	}

	// Destroy widgets.
	m_menuPopup->destroy();
	m_sequencerPanel->destroy();
}

void AnimationEditorPage::activate()
{
}

void AnimationEditorPage::deactivate()
{
}

bool AnimationEditorPage::dropInstance(db::Instance* instance, const ui::Point& position)
{
	Ref< Skeleton > skeleton = instance->getObject< Skeleton >();
	if (!skeleton)
		return false;

	setSkeleton(skeleton);
	return true;
}

bool AnimationEditorPage::handleCommand(const ui::Command& command)
{
	if (command == L"Editor.SettingsChanged")
	{
		updateSettings();
	}
	if (command == L"Editor.Cut" || command == L"Editor.Copy")
	{
		int poseIndex;
		if (!getSelectedPoseId(poseIndex))
			return false;

		const Animation::KeyPose& keyPose = m_animation->getKeyPose(poseIndex);
		ui::Application::getInstance()->getClipboard()->setObject(new KeyPoseClipboardData(keyPose));

		if (command == L"Editor.Cut")
		{
			m_document->push();
			m_animation->removeKeyPose(poseIndex);
			updateRenderWidgets();
			updateSequencer();
		}
	}
	else if (command == L"Editor.Paste")
	{
		Ref< KeyPoseClipboardData > data = dynamic_type_cast< KeyPoseClipboardData* >(
			ui::Application::getInstance()->getClipboard()->getObject()
		);
		if (!data)
			return false;

		m_document->push();
		
		float time = float(m_sequencer->getCursor() / 1000.0f);

		Animation::KeyPose keyPose;
		keyPose.at = time;
		keyPose.pose = data->getPose().pose;
		m_animation->addKeyPose(keyPose);

		updateRenderWidgets();
		updateSequencer();
	}
	else if (command == L"Editor.Undo")
	{
		if (!m_document->undo())
			return false;

		m_animation = m_document->getObject< Animation >(0);
		T_ASSERT (m_animation);

		updateRenderWidgets();
		updateSequencer();
	}
	else if (command == L"Editor.Redo")
	{
		if (!m_document->redo())
			return false;

		m_animation = m_document->getObject< Animation >(0);
		T_ASSERT (m_animation);

		updateRenderWidgets();
		updateSequencer();
	}
	else if (command == L"Animation.Editor.Rewind")
	{
		m_sequencer->setCursor(0);
		m_sequencer->update();
		updateRenderWidgets();
	}
	else if (command == L"Animation.Editor.Play")
	{
		m_previewAnimation = true;
	}
	else if (command == L"Animation.Editor.Stop")
	{
		m_previewAnimation = false;
	}
	else if (command == L"Animation.Editor.Forward")
	{
		if (!m_animation->empty())
		{
			m_sequencer->setCursor(int(m_animation->getLastKeyPose().at * 1000.0f));
			m_sequencer->update();
			updateRenderWidgets();
		}
	}
	else if (command == L"Animation.Editor.BrowseSkeleton")
	{
		editor::TypeBrowseFilter filter(type_of< Skeleton >());
		Ref< db::Instance > skeletonInstance = m_editor->browseInstance(&filter);
		if (skeletonInstance)
		{
			Ref< Skeleton > skeleton = skeletonInstance->getObject< Skeleton >();
			if (skeleton)
				setSkeleton(skeleton);
		}
	}
	else if (command == L"Animation.Editor.BrowseSkin")
	{
	}
	else if (command == L"Animation.Editor.ToggleTrail")
	{
		m_showGhostTrail = !m_showGhostTrail;
		updateRenderWidgets();
	}
	else if (command == L"Animation.Editor.ToggleTwistLock")
	{
		m_twistLock = !m_twistLock;
	}
	else if (command == L"Animation.Editor.ToggleIK")
	{
		m_ikEnabled = !m_ikEnabled;
		updateRenderWidgets();
	}
	//else if (command == L"Animation.Editor.SelectPreviousBone")
	//{
	//	if (m_skeleton)
	//	{
	//		if (m_selectedBone > 0)
	//			m_selectedBone--;
	//		else
	//			m_selectedBone = m_skeleton->getBoneCount() - 1;
	//		updateRenderWidgets();
	//	}
	//}
	//else if (command == L"Animation.Editor.SelectNextBone")
	//{
	//	if (m_skeleton)
	//	{
	//		if (m_selectedBone < int(m_skeleton->getBoneCount()) - 1)
	//			m_selectedBone++;
	//		else
	//			m_selectedBone = 0;
	//		updateRenderWidgets();
	//	}
	//}
	else if (command == L"Animation.Editor.Create")
	{
		float time = float(m_sequencer->getCursor() / 1000.0f);

		m_document->push();

		Animation::KeyPose keyPose;
		keyPose.at = time;

		int32_t indexHint = -1;
		m_animation->getPose(time, false, indexHint, keyPose.pose);
		m_animation->addKeyPose(keyPose);

		updateRenderWidgets();
		updateSequencer();
	}
	else if (command == L"Editor.Delete")
	{
		int poseIndex;
		if (!getSelectedPoseId(poseIndex))
			return false;

		m_document->push();

		m_animation->removeKeyPose(poseIndex);
		updateRenderWidgets();
		updateSequencer();
	}
	else
		return false;

	return true;
}

void AnimationEditorPage::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
	if (m_resourceManager)
		m_resourceManager->reload(eventId, false);
}

void AnimationEditorPage::setSkeleton(Skeleton* skeleton)
{
	m_skeleton = skeleton;
	T_ASSERT (m_skeleton);

	Aabb3 boundingBox = calculateBoundingBox(m_skeleton);
	
	int majorAxis = majorAxis3(boundingBox.getExtent());
	float majorExtent = boundingBox.getExtent()[majorAxis];

	m_cameraOffsetScale.set(
		majorExtent / 20.0f,
		majorExtent / 20.0f,
		majorExtent / 20.0f,
		1.0f
	);

	m_cameraSizeScale = majorExtent / 20.0f;

	updateRenderWidgets();
}

bool AnimationEditorPage::getSelectedPoseId(int& outPoseId) const
{
	RefArray< ui::custom::SequenceItem > selectedItems;
	m_sequencer->getSequenceItems(selectedItems, ui::custom::SequencerControl::GfSelectedOnly);
	if (selectedItems.size() != 1)
		return false;

	Ref< ui::custom::Sequence > selectedSequence = checked_type_cast< ui::custom::Sequence* >(selectedItems.front());
	Ref< ui::custom::Key > selectedKey = selectedSequence->getSelectedKey();
	if (!selectedKey)
		return false;

	Ref< PoseIdData > id = selectedKey->getData< PoseIdData >(L"ID");
	T_ASSERT (id);

	outPoseId = id->m_id;
	return true;
}

void AnimationEditorPage::updateRenderWidgets()
{
	for (int i = 0; i < sizeof_array(m_renderWidgets); ++i)
		m_renderWidgets[i]->update();
}

void AnimationEditorPage::updateSequencer()
{
	uint32_t poseCount = m_animation->getKeyPoseCount();
	if (!poseCount)
		return;

	Ref< ui::custom::Sequence > sequence = new ui::custom::Sequence(i18n::Text(L"ANIMATION_EDITOR_SEQUENCE"));
	for (uint32_t i = 0; i < poseCount; ++i)
	{
		int ms = int(m_animation->getKeyPose(i).at * 1000.0f);

		Ref< ui::custom::Tick > tick = new ui::custom::Tick(ms, true);
		tick->setData(L"ID", new PoseIdData(i));

		sequence->addKey(tick);
	}

	m_sequencer->removeAllSequenceItems();
	m_sequencer->addSequenceItem(sequence);
	m_sequencer->update();
}

bool AnimationEditorPage::calculateRelativeTwist(int poseIndex, int boneIndex, float& outRelativeTwist) const
{
	if (!m_skeleton)
		return false;

	//int parentId = m_skeleton->getBone(m_selectedBone)->getParent();
	//if (parentId < 0)
	//	return false;

	//Pose& pose = m_animation->getKeyPose(poseIndex).pose;

	//AlignedVector< Transform > poseTransforms;
	//calculatePoseTransforms(m_skeleton, &pose, poseTransforms);

	//Vector4 parentAxisX = poseTransforms[parentId] * Vector4(1.0f, 0.0f, 0.0f);
	//Vector4 parentAxisY = poseTransforms[parentId] * Vector4(0.0f, 1.0f, 0.0f);
	//Vector4 parentAxisZ = poseTransforms[parentId] * Vector4(0.0f, 0.0f, 1.0f);

	//Vector4 currentAxisX = poseTransforms[m_selectedBone] * Vector4(1.0f, 0.0f, 0.0f);
	//Vector4 currentAxisZ = poseTransforms[m_selectedBone] * Vector4(0.0f, 0.0f, 1.0f);
	//Vector4 currentAxisXinParent = (currentAxisX - parentAxisZ * dot3(parentAxisZ, currentAxisX)).normalized();

	//float relativeDirection = dot3(currentAxisZ, parentAxisZ);
	//if (abs(relativeDirection) < FUZZY_EPSILON)
	//	return false;

	//float ax = dot3(currentAxisXinParent, parentAxisY);
	//float ay = dot3(currentAxisXinParent, parentAxisX);
	//outRelativeTwist = atan2f(ax, ay);

	//if (relativeDirection < 0.0f)
	//	outRelativeTwist = -outRelativeTwist;

	return true;
}

void AnimationEditorPage::drawSkeleton(float time, const Color4ub& defaultColor, const Color4ub& selectedColor, bool drawAxis) const
{
	Pose pose;

	int32_t indexHint = -1;
	m_animation->getPose(time, false, indexHint, pose);

	AlignedVector< Transform > boneTransforms;
	calculatePoseTransforms(
		m_skeleton,
		&pose,
		boneTransforms
	);

	if (m_ikEnabled)
	{
		bool continueUpdate = true;
		IKPoseController(0, 0, 10).evaluate(
			0.0f,
			Transform::identity(),
			m_skeleton,
			boneTransforms,
			boneTransforms,
			continueUpdate
		);
	}

	//for (int i = 0; i < int(m_skeleton->getBoneCount()); ++i)
	//{
	//	const Bone* bone = m_skeleton->getBone(i);

	//	Vector4 start = boneTransforms[i].translation();
	//	Vector4 end = boneTransforms[i].translation() + boneTransforms[i] * Vector4(0.0f, 0.0f, bone->getLength(), 0.0f);

	//	const Color4ub& color = (m_selectedBone == i) ? selectedColor : defaultColor;

	//	Vector4 d = boneTransforms[i].axisZ();
	//	Vector4 a = boneTransforms[i].axisX();
	//	Vector4 b = boneTransforms[i].axisY();

	//	Scalar radius = bone->getRadius();
	//	d *= radius;
	//	a *= radius;
	//	b *= radius;

	//	m_primitiveRenderer->drawLine(start, start + d + a + b, color);
	//	m_primitiveRenderer->drawLine(start, start + d - a + b, color);
	//	m_primitiveRenderer->drawLine(start, start + d + a - b, color);
	//	m_primitiveRenderer->drawLine(start, start + d - a - b, color);

	//	m_primitiveRenderer->drawLine(start + d + a + b, end, color);
	//	m_primitiveRenderer->drawLine(start + d - a + b, end, color);
	//	m_primitiveRenderer->drawLine(start + d + a - b, end, color);
	//	m_primitiveRenderer->drawLine(start + d - a - b, end, color);

	//	m_primitiveRenderer->drawLine(start + d + a + b, start + d - a + b, color);
	//	m_primitiveRenderer->drawLine(start + d - a + b, start + d - a - b, color);
	//	m_primitiveRenderer->drawLine(start + d - a - b, start + d + a - b, color);
	//	m_primitiveRenderer->drawLine(start + d + a - b, start + d + a + b, color);

	//	m_primitiveRenderer->drawLine(start, end, color);

	//	if (drawAxis)
	//	{
	//		m_primitiveRenderer->drawLine(start, start + a * Scalar(2.0f), Color4ub(255, 0, 0, color.a));
	//		m_primitiveRenderer->drawLine(start, start + b * Scalar(2.0f), Color4ub(0, 255, 0, color.a));
	//	}
	//}
}

void AnimationEditorPage::updateSettings()
{
	m_colorClear = m_editor->getSettings()->getProperty< PropertyColor >(L"Editor.Colors/Background");
	m_colorGrid = m_editor->getSettings()->getProperty< PropertyColor >(L"Editor.Colors/Grid");
	m_colorBone = m_editor->getSettings()->getProperty< PropertyColor >(L"Editor.Colors/BoneWire");
	m_colorBoneSel = m_editor->getSettings()->getProperty< PropertyColor >(L"Editor.Colors/BoneWireSelected");
	updateRenderWidgets();
}

void AnimationEditorPage::eventRenderButtonDown(ui::MouseButtonDownEvent* event)
{
	Ref< ui::Widget > renderWidget = checked_type_cast< ui::Widget* >(event->getSender());

	Ref< RenderWidgetData > data = renderWidget->getData< RenderWidgetData >(L"DATA");
	T_ASSERT (data);

	if (!m_skeleton)
		return;

	m_editMode = (event->getKeyState() & ui::KsControl) != ui::KsControl;
	if (m_editMode)
		m_document->push();

	m_lastMousePosition = event->getPosition();

	// Trace volume
	// @fixme Only orthogonal views work with picking.
	if (m_editMode && data->orthogonal)
	{
		render::Viewport viewport = data->renderView->getViewport();

		Vector4 position(
			2.0f * float(m_lastMousePosition.x) / viewport.width - 1.0f,
			1.0f - 2.0f * float(m_lastMousePosition.y) / viewport.height,
			0.0f/*c_nearZ*/,
			1.0f
		);
		int hit = data->picker->traceVolume(position);
		if (hit >= 0)
			m_selectedBone = hit;
	}

	int poseIndex;
	if (getSelectedPoseId(poseIndex))
		m_haveRelativeTwist = calculateRelativeTwist(poseIndex, m_selectedBone, m_relativeTwist);
	else
		m_haveRelativeTwist = false;

	updateRenderWidgets();

	renderWidget->setCapture();
	renderWidget->setFocus();
}

void AnimationEditorPage::eventRenderButtonUp(ui::MouseButtonUpEvent* event)
{
	Ref< ui::Widget > renderWidget = checked_type_cast< ui::Widget* >(event->getSender());

	if (renderWidget->hasCapture())
		renderWidget->releaseCapture();

	m_haveRelativeTwist = false;
}

void AnimationEditorPage::eventRenderMouseMove(ui::MouseMoveEvent* event)
{
	Ref< ui::Widget > renderWidget = checked_type_cast< ui::Widget* >(event->getSender());

	Ref< RenderWidgetData > data = renderWidget->getData< RenderWidgetData >(L"DATA");
	T_ASSERT (data);

	if (!m_skeleton || !renderWidget->hasCapture())
		return;

	ui::Point mousePosition = event->getPosition();

	Vector2 mouseDelta(
		float(m_lastMousePosition.x - mousePosition.x),
		float(m_lastMousePosition.y - mousePosition.y)
	);

	mouseDelta /= 60.0f;

	//if (m_editMode)
	//{
	//	int poseIndex;
	//	if (getSelectedPoseId(poseIndex))
	//	{
	//		Pose& pose = m_animation->getKeyPose(poseIndex).pose;

	//		if ((mouseEvent->getKeyState() & ui::KsMenu) != ui::KsMenu)
	//		{
	//			if (mouseEvent->getButton() == ui::MbtLeft)
	//			{
	//				Vector4 orientation = pose.getBoneOrientation(m_selectedBone);
	//				orientation += Vector4(mouseDelta.x, mouseDelta.y, 0.0f, 0.0f);
	//				pose.setBoneOrientation(m_selectedBone, orientation);

	//				// Compensate for applied twist.
	//				if (m_twistLock && m_haveRelativeTwist)
	//				{
	//					float relativeTwist;
	//					if (calculateRelativeTwist(poseIndex, m_selectedBone, relativeTwist))
	//					{
	//						orientation += Vector4(0.0f, 0.0f, m_relativeTwist - relativeTwist);
	//						pose.setBoneOrientation(m_selectedBone, orientation);
	//					}
	//				}
	//			}
	//			else
	//			{
	//				Vector4 orientation = pose.getBoneOrientation(m_selectedBone);
	//				orientation += Vector4(0.0f, 0.0f, mouseDelta.x, 0.0f);
	//				pose.setBoneOrientation(m_selectedBone, orientation);
	//			}
	//		}
	//		else
	//		{
	//			Vector4 delta;
	//			if (mouseEvent->getButton() == ui::MbtLeft)
	//				delta = Vector4(mouseDelta.x, mouseDelta.y, 0.0f, 0.0f);
	//			else
	//				delta = Vector4(0.0f, 0.0f, mouseDelta.y, 0.0f);

	//			Vector4 offset = pose.getBoneOffset(m_selectedBone);
	//			offset += delta;
	//			pose.setBoneOffset(m_selectedBone, offset);
	//		}

	//		updateRenderWidgets();
	//	}
	//}
	//else
	{
		if (event->getButton() == ui::MbtLeft)
		{
			data->cameraAngleX -= mouseDelta.x / 2.0f;
			data->cameraAngleY += mouseDelta.y / 2.0f;
		}
		else
		{
			if (data->orthogonal)
				data->cameraOffset += Vector4(0.0f, mouseDelta.y * 10.0f, 0.0f, 0.0f);
			else
				data->cameraOffset += Vector4(0.0f, 0.0f, -mouseDelta.y * 10.0f, 0.0f);

			data->cameraOffset += Vector4(-mouseDelta.x * 10.0f, 0.0f, 0.0f, 0.0f);
		}

		renderWidget->update();
	}

	m_lastMousePosition = mousePosition;
}

void AnimationEditorPage::eventRenderMouseWheel(ui::MouseWheelEvent* event)
{
	Ref< ui::Widget > renderWidget = checked_type_cast< ui::Widget* >(event->getSender());

	Ref< RenderWidgetData > data = renderWidget->getData< RenderWidgetData >(L"DATA");
	T_ASSERT (data);

	if (data->orthogonal)
	{
		data->size += event->getRotation() * c_wheelRotationDelta;
		data->size = std::max(data->size, c_orthoSizeMin);
		renderWidget->update();
	}
}

void AnimationEditorPage::eventRenderSize(ui::SizeEvent* event)
{
	Ref< ui::Widget > renderWidget = checked_type_cast< ui::Widget* >(event->getSender());

	Ref< RenderWidgetData > data = renderWidget->getData< RenderWidgetData >(L"DATA");
	T_ASSERT (data);

	ui::Size sz = event->getSize();

	data->renderView->reset(sz.cx, sz.cy);
	data->renderView->setViewport(render::Viewport(0, 0, sz.cx, sz.cy, 0, 1));
}

void AnimationEditorPage::eventRenderPaint(ui::PaintEvent* event)
{
	Ref< ui::Widget > renderWidget = checked_type_cast< ui::Widget* >(event->getSender());

	Ref< RenderWidgetData > data = renderWidget->getData< RenderWidgetData >(L"DATA");
	T_ASSERT (data);

	ui::Rect rc = renderWidget->getInnerRect();

	T_ASSERT (m_primitiveRenderer);

	if (!m_animation)
		return;

	if (!data->renderView->begin(render::EtCyclop))
		return;

	float tmp[4];
	m_colorClear.getRGBA32F(tmp);
	Color4f c4f(tmp[0], tmp[1], tmp[2], tmp[3]);

	data->renderView->clear(
		render::CfColor | render::CfDepth,
		&c4f,
		1.0f,
		128
	);

	// Calculate transformation matrices.
	render::Viewport viewport = data->renderView->getViewport();

	Matrix44 projection;
	if (!data->orthogonal)
	{
		projection = perspectiveLh(
			deg2rad(c_fov),
			float(viewport.width) / viewport.height,
			c_nearZ,
			c_farZ
		);
	}
	else
	{
		float ratio = float(viewport.width) / viewport.height;
		projection = orthoLh(
			data->size * m_cameraSizeScale * ratio,
			data->size * m_cameraSizeScale,
			c_nearZ,
			c_farZ
		);
	}

	Matrix44 view = translate(data->cameraOffset * m_cameraOffsetScale) * rotateX(data->cameraAngleY) * rotateY(data->cameraAngleX);

	// Set transformation in picker.
	data->picker->setPerspectiveTransform(projection);
	data->picker->setViewTransform(view);

	// Begin rendering primitives.
	if (m_primitiveRenderer->begin(data->renderView, projection))
	{
		m_primitiveRenderer->pushView(view);

		for (int x = -10; x <= 10; ++x)
		{
			m_primitiveRenderer->drawLine(
				Vector4(float(x), 0.0f, -10.0f, 1.0f),
				Vector4(float(x), 0.0f, 10.0f, 1.0f),
				m_colorGrid
			);
			m_primitiveRenderer->drawLine(
				Vector4(-10.0f, 0.0f, float(x), 1.0f),
				Vector4(10.0f, 0.0f, float(x), 1.0f),
				m_colorGrid
			);
		}

		m_primitiveRenderer->drawArrowHead(
			Vector4(11.0f, 0.0f, 0.0f, 1.0f),
			Vector4(13.0f, 0.0f, 0.0f, 1.0f),
			0.8f,
			Color4ub(255, 0, 0)
		);
		m_primitiveRenderer->drawArrowHead(
			Vector4(0.0f, 0.0f, 11.0f, 1.0f),
			Vector4(0.0f, 0.0f, 13.0f, 1.0f),
			0.8f,
			Color4ub(0, 0, 255)
		);

		if (m_skeleton)
		{
			float time = float(m_sequencer->getCursor() / 1000.0f);

			// Render ghost trail.
			if (m_showGhostTrail)
			{
				for (int i = 0; i < 10; ++i)
				{
					float ghostTime = time - (i + 1) * 0.1f;
					if (ghostTime < 0.0f)
						break;

					uint8_t alpha = 100 - i * 100 / 10;
					drawSkeleton(ghostTime, Color4ub(0, 0, 0, alpha), Color4ub(0, 0, 0, alpha), false);
				}
			}

			// Render current skeleton.
			drawSkeleton(
				time,
				m_colorBone,
				m_colorBoneSel,
				true
			);

			// Update picker volumes.
			Pose pose;

			int32_t indexHint = -1;
			m_animation->getPose(time, false, indexHint, pose);

			AlignedVector< Transform > boneTransforms;
			calculatePoseTransforms(
				m_skeleton,
				&pose,
				boneTransforms
			);

			data->picker->removeAllVolumes();
			//for (int i = 0; i < int(m_skeleton->getBoneCount()); ++i)
			//{
			//	const Bone* bone = m_skeleton->getBone(i);

			//	Vector4 center(0.0f, 0.0f, bone->getLength() / 2.0f, 1.0f);
			//	Vector4 extent(bone->getRadius(), bone->getRadius(), bone->getLength() / 2.0f, 0.0f);

			//	Aabb3 boneVolume(
			//		center - extent,
			//		center + extent
			//	);

			//	data->picker->addVolume(boneTransforms[i].toMatrix44(), boneVolume, i);
			//}
		}

		m_primitiveRenderer->end();
	}

	data->renderView->end();
	data->renderView->present();

	event->consume();
}

void AnimationEditorPage::eventSequencerButtonDown(ui::MouseButtonDownEvent* event)
{
	if (event->getButton() != ui::MbtRight)
		return;

	Ref< ui::MenuItem > selectedItem = m_menuPopup->show(m_sequencer, event->getPosition());
	if (selectedItem)
		handleCommand(selectedItem->getCommand());

	event->consume();
}

void AnimationEditorPage::eventSequencerCursorMove(ui::custom::CursorMoveEvent* event)
{
	updateRenderWidgets();
}

void AnimationEditorPage::eventSequencerTimer(ui::TimerEvent* event)
{
	if (!m_previewAnimation || !m_animation)
		return;

	int end = int(m_animation->getLastKeyPose().at * 1000.0f);

	int cursor = 0;
	if (end > 0)
		cursor = (m_sequencer->getCursor() + 1000 / 30) % end;

	m_sequencer->setCursor(cursor);
	m_sequencer->update();

	updateRenderWidgets();
}

void AnimationEditorPage::eventToolClick(ui::custom::ToolBarButtonClickEvent* event)
{
	const ui::Command& command = event->getCommand();
	handleCommand(command);
}

	}
}
