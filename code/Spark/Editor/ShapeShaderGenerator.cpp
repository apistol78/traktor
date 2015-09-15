#include "Database/Database.h"
#include "Editor/IPipelineDepends.h"
//#include "Render/Editor/Shader/ShaderGraphValidator.h"
#include "Render/Resource/FragmentLinker.h"
#include "Render/Shader/External.h"
#include "Render/Shader/Nodes.h"
#include "Render/Shader/ShaderGraph.h"
#include "Spark/Editor/ShapeShaderGenerator.h"

namespace traktor
{
	namespace spark
	{
		namespace
		{

const Guid c_materialShader(L"{8D5DD962-4142-A549-B124-F8006F4E4088}");
const Guid c_tplOutput(L"{DCDFF93D-8178-EC4B-9978-5BC0097AC150}");

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

Ref< render::ShaderGraph > ShapeShaderGenerator::generate(db::Database* database, const Guid& outputFragment) const
{
	Guid templateGuid = c_materialShader;

	Ref< render::ShaderGraph > materialShaderGraph = database->getObjectReadOnly< render::ShaderGraph >(templateGuid);
	if (!materialShaderGraph)
		return 0;

	RefArray< render::External > externalNodes;
	materialShaderGraph->findNodesOf< render::External >(externalNodes);

	for (RefArray< render::External >::iterator i = externalNodes.begin(); i != externalNodes.end(); ++i)
	{
		const Guid& fragmentGuid = (*i)->getFragmentGuid();
		T_ASSERT (fragmentGuid.isValid());

		if (fragmentGuid == c_tplOutput)
			(*i)->setFragmentGuid(outputFragment);
	}

	FragmentReaderAdapter fragmentReader(database);
	materialShaderGraph = render::FragmentLinker(fragmentReader).resolve(materialShaderGraph, false);
	if (!materialShaderGraph)
		return 0;

	//render::ShaderGraphValidator(materialShaderGraph).validateIntegrity();
	return materialShaderGraph;
}

void ShapeShaderGenerator::addDependencies(editor::IPipelineDepends* pipelineDepends)
{
	pipelineDepends->addDependency(c_materialShader, editor::PdfUse);
	pipelineDepends->addDependency(c_tplOutput, editor::PdfUse);
}

	}
}
