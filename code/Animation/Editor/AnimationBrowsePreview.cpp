/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Editor/AnimationAsset.h"
#include "Animation/Editor/AnimationBrowsePreview.h"
#include "Animation/Editor/SkeletonAsset.h"
#include "Core/Io/FileSystem.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Drawing/Raster.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Filters/ScaleFilter.h"
#include "Editor/IEditor.h"
#include "Model/Model.h"
#include "Model/Pose.h"
#include "Model/Operations/Triangulate.h"
#include "Model/ModelCache.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.AnimationBrowsePreview", 0, AnimationBrowsePreview, editor::IBrowsePreview)

TypeInfoSet AnimationBrowsePreview::getPreviewTypes() const
{
	return makeTypeInfoSet< AnimationAsset, SkeletonAsset >();
}

Ref< ui::Bitmap > AnimationBrowsePreview::generate(editor::IEditor* editor, db::Instance* instance) const
{
	Ref< const editor::Asset > asset = instance->getObject< editor::Asset >();
	if (!asset)
		return nullptr;

	Ref< drawing::Image > meshThumb = new drawing::Image(
		drawing::PixelFormat::getR8G8B8A8(),
		128,
		128
	);
	meshThumb->clear(Color4f(0.4f, 0.4f, 0.6f, 0.0f));

	const std::wstring assetPath = editor->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
	const std::wstring modelCachePath = editor->getSettings()->getProperty< std::wstring >(L"Pipeline.ModelCache.Path");

	const Path fileName = FileSystem::getInstance().getAbsolutePath(assetPath, asset->getFileName());

	Ref< const model::Model > model = model::ModelCache::getInstance().get(modelCachePath, fileName, L""); //, asset->getImportFilter());
	if (!model)
		return nullptr;

	const float hw = (float)(meshThumb->getWidth() / 2.0f);
	const float hh = (float)(meshThumb->getHeight() / 2.0f);
	const float r = (float)meshThumb->getWidth() / meshThumb->getHeight();

	drawing::Raster raster(meshThumb);
	const int32_t ls = raster.defineSolidStyle(Color4f(1.0f, 0.9f, 0.1f, 1.0f));

	const Matrix44 projection = perspectiveLh(deg2rad(70.0f), r, 0.1f, 100.0f);

	// Transform all joints into screen space.
	Aabb3 boundingBox;
	for (int32_t i = 0; i < model->getJointCount(); ++i)
	{
		const Transform Tjoint = model->getJointGlobalTransform(i);
		boundingBox.contain(Tjoint.translation().xyz1());
	}

	const Scalar maxExtent = (boundingBox.getExtent() * Vector4(1.0f, 1.0f, 0.0f, 0.0f)).max();
	const Scalar invMaxExtent = 1.0_simd / maxExtent;
	const Matrix44 modelView = translate(0.0f, 0.0f, 1.75f) * scale(invMaxExtent, invMaxExtent, invMaxExtent) * translate(-boundingBox.getCenter());

	AlignedVector< Vector2 > sp(model->getJointCount());

	if (is_a< AnimationAsset >(asset) && model->getAnimationCount() > 0)
	{
		const model::Animation* anim = model->getAnimation(0);
		const model::Pose* pose = anim->getKeyFramePose(0);
		for (int32_t i = 0; i < model->getJointCount(); ++i)
		{
			const Transform Tjoint = pose->getJointGlobalTransform(model, i);
			const Vector4 vp = modelView * Tjoint.translation().xyz1();
			Vector4 cp = projection * vp; cp = cp / cp.w();
			sp[i] = Vector2(
				cp.x() * hw + hw,
				hh - (cp.y() * hh)
			);
		}
	}
	else
	{
		for (int32_t i = 0; i < model->getJointCount(); ++i)
		{
			const Transform Tjoint = model->getJointGlobalTransform(i);
			const Vector4 vp = modelView * Tjoint.translation().xyz1();
			Vector4 cp = projection * vp; cp = cp / cp.w();
			sp[i] = Vector2(
				cp.x() * hw + hw,
				hh - (cp.y() * hh)
			);
		}
	}

	// Draw joint edges.
	raster.clear();
	for (int32_t i = 0; i < model->getJointCount(); ++i)
	{
		const model::Joint& joint = model->getJoint(i);
		if (joint.getParent() != model::c_InvalidIndex)
		{
			const Vector2& s = sp[joint.getParent()];
			const Vector2& e = sp[i];
			raster.moveTo(s);
			raster.lineTo(e);
		}
	}
	raster.stroke(ls, 2.0f, drawing::Raster::StrokeCap::Round);
	raster.submit();

	drawing::ScaleFilter scaleFilter(
		64,
		64,
		drawing::ScaleFilter::MnAverage,
		drawing::ScaleFilter::MgLinear
	);
	meshThumb->apply(&scaleFilter);

	return new ui::Bitmap(meshThumb);
}

}
