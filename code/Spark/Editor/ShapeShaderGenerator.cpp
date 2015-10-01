#include "Database/Database.h"
#include "Editor/IPipelineDepends.h"
#include "Render/Resource/FragmentLinker.h"
#include "Render/Shader/External.h"
#include "Render/Shader/Nodes.h"
#include "Render/Shader/ShaderGraph.h"
#include "Spark/Editor/ShapeShaderGenerator.h"
#include "Spark/Editor/Shape/Gradient.h"
#include "Spark/Editor/Shape/Style.h"

namespace traktor
{
	namespace spark
	{
		namespace
		{

const Guid c_vectorMaterialShader(L"{8D5DD962-4142-A549-B124-F8006F4E4088}");
const Guid c_imageMaterialShader(L"{90E966C9-FAB6-194F-93EC-87D669FA3E78}");
const Guid c_tplColor(L"{E976CF04-9B52-164F-809D-00C05EFC6195}");
const Guid c_implSolidColor(L"{1142089D-807C-E344-A9E1-C148AEF2D302}");
const Guid c_implGradientColor(L"{9A57BCE6-C54E-824B-BD1B-637E720BC321}");

class FragmentReaderAdapter : public render::FragmentLinker::IFragmentReader
{
public:
	FragmentReaderAdapter(db::Database* database)
	:	m_database(database)
	{
	}

	virtual Ref< const render::ShaderGraph > read(const Guid& fragmentGuid) const
	{
		return m_database->getObjectReadOnly< render::ShaderGraph >(fragmentGuid);
	}

private:
	Ref< db::Database > m_database;
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.ShapeShaderGenerator", ShapeShaderGenerator, Object)

Ref< render::ShaderGraph > ShapeShaderGenerator::generate(db::Database* database, const Style* style) const
{
	Ref< render::ShaderGraph > materialShaderGraph = database->getObjectReadOnly< render::ShaderGraph >(c_vectorMaterialShader);
	if (!materialShaderGraph)
		return 0;

	RefArray< render::External > externalNodes;
	materialShaderGraph->findNodesOf< render::External >(externalNodes);

	for (RefArray< render::External >::iterator i = externalNodes.begin(); i != externalNodes.end(); ++i)
	{
		const Guid& fragmentGuid = (*i)->getFragmentGuid();
		T_ASSERT (fragmentGuid.isValid());

		if (fragmentGuid == c_tplColor)
		{
			if (!style->getFillGradient())
				(*i)->setFragmentGuid(c_implSolidColor);
			else
				(*i)->setFragmentGuid(c_implGradientColor);
		}
	}

	FragmentReaderAdapter fragmentReader(database);
	materialShaderGraph = render::FragmentLinker(fragmentReader).resolve(materialShaderGraph, false);
	if (!materialShaderGraph)
		return 0;

	const RefArray< render::Node >& nodes = materialShaderGraph->getNodes();
	for (RefArray< render::Node >::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
	{
		std::wstring comment = (*i)->getComment();
		if (comment == L"Tag_FillColor")
		{
			render::Color* colorNode = checked_type_cast< render::Color* >(*i);
			colorNode->setComment(L"");
			colorNode->setColor(Color4ub(
				style->getFill().r,
				style->getFill().g,
				style->getFill().b,
				int32_t(style->getOpacity() * 255)
			));
		}
		else if (comment == L"Tag_GradientColor0")
		{
			render::Color* colorNode = checked_type_cast< render::Color* >(*i);
			colorNode->setComment(L"");
			colorNode->setColor(style->getFillGradient()->getStops()[0].color);
		}
		else if (comment == L"Tag_GradientColor1")
		{
			render::Color* colorNode = checked_type_cast< render::Color* >(*i);
			colorNode->setComment(L"");
			colorNode->setColor(style->getFillGradient()->getStops()[1].color);
		}
	}

	return materialShaderGraph;
}

Ref< render::ShaderGraph > ShapeShaderGenerator::generate(db::Database* database, const Guid& textureId) const
{
	Ref< render::ShaderGraph > materialShaderGraph = database->getObjectReadOnly< render::ShaderGraph >(c_imageMaterialShader);
	if (!materialShaderGraph)
		return 0;

	RefArray< render::Texture > textureNodes;
	materialShaderGraph->findNodesOf< render::Texture >(textureNodes);

	for (RefArray< render::Texture >::iterator i = textureNodes.begin(); i != textureNodes.end(); ++i)
	{
		if ((*i)->getComment() == L"Tag_Image")
		{
			(*i)->setComment(L"");
			(*i)->setExternal(textureId);
		}
	}

	FragmentReaderAdapter fragmentReader(database);
	materialShaderGraph = render::FragmentLinker(fragmentReader).resolve(materialShaderGraph, false);
	if (!materialShaderGraph)
		return 0;

	return materialShaderGraph;
}

void ShapeShaderGenerator::addDependencies(editor::IPipelineDepends* pipelineDepends)
{
	pipelineDepends->addDependency(c_vectorMaterialShader, editor::PdfUse);
	pipelineDepends->addDependency(c_imageMaterialShader, editor::PdfUse);
	pipelineDepends->addDependency(c_tplColor, editor::PdfUse);
	pipelineDepends->addDependency(c_implSolidColor, editor::PdfUse);
	pipelineDepends->addDependency(c_implGradientColor, editor::PdfUse);
}

	}
}
