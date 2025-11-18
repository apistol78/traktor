/*
 * TRAKTOR
 * Copyright (c) 2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Shader/Experiment/ShaderExperimentPipeline.h"

#include "Editor/IPipelineDepends.h"
#include "Render/Editor/Shader/Experiment/ShaderExperiment.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ShaderExperimentPipeline", 0, ShaderExperimentPipeline, editor::DefaultPipeline)

TypeInfoSet ShaderExperimentPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< ShaderExperiment >();
}

bool ShaderExperimentPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid) const
{
	Ref< const ShaderExperiment > shaderExperiment = dynamic_type_cast< const ShaderExperiment* >(sourceAsset);
	if (!shaderExperiment)
		return false;

	pipelineDepends->addDependency(shaderExperiment->getShader(), editor::PdfBuild);
	return true;
}

}
