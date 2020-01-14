#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "Render/Image/Steps/ImageProcessStepCompute.h"
#include "Render/Editor/Image/ImageProcessStepComputeFacade.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageProcessStepComputeFacade", 0, ImageProcessStepComputeFacade, IImageProcessStepFacade)

int32_t ImageProcessStepComputeFacade::getImage(editor::IEditor* editor, const ImageProcessStep* step) const
{
	return 5;
}

std::wstring ImageProcessStepComputeFacade::getText(editor::IEditor* editor, const ImageProcessStep* step) const
{
	const ImageProcessStepCompute* compute = mandatory_non_null_type_cast< const ImageProcessStepCompute* >(step);
	const resource::Id< Shader >& shader = compute->getShader();
	Ref< db::Instance > instance = editor->getSourceDatabase()->getInstance(shader);
	if (instance)
		return L"Compute \"" + instance->getName() + L"\"";
	else
		return L"Compute <No such shader>";
}

void ImageProcessStepComputeFacade::getSources(const ImageProcessStep* step, std::vector< std::wstring >& outSources) const
{
	const ImageProcessStepCompute* compute = mandatory_non_null_type_cast< const ImageProcessStepCompute* >(step);
	for (const auto& source : compute->getSources())
		outSources.push_back(source.source);
}

bool ImageProcessStepComputeFacade::canHaveChildren() const
{
	return false;
}

bool ImageProcessStepComputeFacade::addChild(ImageProcessStep* parentStep, ImageProcessStep* childStep) const
{
	return false;
}

bool ImageProcessStepComputeFacade::removeChild(ImageProcessStep* parentStep, ImageProcessStep* childStep) const
{
	return false;
}

bool ImageProcessStepComputeFacade::getChildren(const ImageProcessStep* step, RefArray< ImageProcessStep >& outChildren) const
{
	return false;
}

	}
}
