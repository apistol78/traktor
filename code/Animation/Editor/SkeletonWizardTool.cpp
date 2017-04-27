/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Animation/Skeleton.h"
#include "Animation/Editor/SkeletonFormatBvh.h"
#include "Animation/Editor/SkeletonFormatFbx.h"
#include "Animation/Editor/SkeletonFormatLws.h"
#include "Animation/Editor/SkeletonWizardTool.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/Vector4.h"
#include "Core/Misc/String.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "I18N/Text.h"
#include "Ui/FileDialog.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.SkeletonWizardTool", 0, SkeletonWizardTool, editor::IWizardTool)

std::wstring SkeletonWizardTool::getDescription() const
{
	return i18n::Text(L"SKELETON_WIZARDTOOL_DESCRIPTION");
}

uint32_t SkeletonWizardTool::getFlags() const
{
	return WfGroup;
}

bool SkeletonWizardTool::launch(ui::Widget* parent, editor::IEditor* editor, db::Group* group, db::Instance* /*instance*/)
{
	// Select source skeleton.
	ui::FileDialog fileDialog;
	if (!fileDialog.create(parent, i18n::Text(L"SKELETON_WIZARDTOOL_FILE_TITLE"), L"All files;*.*"))
		return 0;

	Path fileName;
	if (fileDialog.showModal(fileName) != ui::DrOk)
	{
		fileDialog.destroy();
		return true;
	}
	fileDialog.destroy();

	Ref< ISkeletonFormat > format;

	if (compareIgnoreCase< std::wstring >(fileName.getExtension(), L"bvh") == 0)
		format = new SkeletonFormatBvh();
	else if (compareIgnoreCase< std::wstring >(fileName.getExtension(), L"fbx") == 0)
		format = new SkeletonFormatFbx();
	else if (compareIgnoreCase< std::wstring >(fileName.getExtension(), L"lws") == 0)
		format = new SkeletonFormatLws();

	if (!format)
	{
		log::error << L"Unable to import skeleton; unsupported format" << Endl;
		return false;
	}

	Ref< IStream > file = FileSystem::getInstance().open(fileName, File::FmRead);
	if (!file)
	{
		log::error << L"Unable to import skeleton; no such file" << Endl;
		return false;
	}

	Ref< Skeleton > skeleton = format->import(
		file,
		Vector4::zero(),
		1.0f,
		0.1f,
		false,
		false
	);

	file->close();

	Ref< db::Instance > skeletonInstance = group->createInstance(
		fileName.getFileNameNoExtension(),
		db::CifReplaceExisting | db::CifKeepExistingGuid
	);
	if (!skeletonInstance)
		return false;

	skeletonInstance->setObject(skeleton);

	if (!skeletonInstance->commit())
		return false;

	return true;
}

	}
}
