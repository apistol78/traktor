#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "Render/ImageProcess/ImageProcessStepSimple.h"
#include "Render/Editor/ImageProcess/ImageProcessStepSimpleFacade.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageProcessStepSimpleFacade", 0, ImageProcessStepSimpleFacade, IImageProcessStepFacade)

int32_t ImageProcessStepSimpleFacade::getImage(editor::IEditor* editor, const ImageProcessStep* step) const
{
	return 5;
}

std::wstring ImageProcessStepSimpleFacade::getText(editor::IEditor* editor, const ImageProcessStep* step) const
{
	const ImageProcessStepSimple* simple = mandatory_non_null_type_cast< const ImageProcessStepSimple* >(step);
	const resource::Id< Shader >& shader = simple->getShader();
	Ref< db::Instance > instance = editor->getSourceDatabase()->getInstance(shader);
	if (instance)
		return L"Simple \"" + instance->getName() + L"\"";
	else
		return L"Simple <No such shader>";
}

void ImageProcessStepSimpleFacade::getSources(const ImageProcessStep* step, std::vector< std::wstring >& outSources) const
{
	const ImageProcessStepSimple* simple = mandatory_non_null_type_cast< const ImageProcessStepSimple* >(step);
	for (std::vector< ImageProcessStepSimple::Source >::const_iterator i = simple->getSources().begin(); i != simple->getSources().end(); ++i)
		outSources.push_back(i->source);
}

bool ImageProcessStepSimpleFacade::canHaveChildren() const
{
	return false;
}

bool ImageProcessStepSimpleFacade::addChild(ImageProcessStep* parentStep, ImageProcessStep* childStep) const
{
	return false;
}

bool ImageProcessStepSimpleFacade::removeChild(ImageProcessStep* parentStep, ImageProcessStep* childStep) const
{
	return false;
}

bool ImageProcessStepSimpleFacade::getChildren(const ImageProcessStep* step, RefArray< ImageProcessStep >& outChildren) const
{
	return false;
}

	}
}
