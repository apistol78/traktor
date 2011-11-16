#include "Database/Database.h"
#include "Editor/IPipelineDepends.h"
#include "Mesh/Editor/MaterialShaderGenerator.h"
#include "Model/Material.h"
#include "Render/Resource/FragmentLinker.h"
#include "Render/Shader/External.h"
#include "Render/Shader/Nodes.h"
#include "Render/Shader/ShaderGraph.h"

namespace traktor
{
	namespace mesh
	{
		namespace
		{

const Guid c_materialShader(L"{CCDB27F2-644B-0742-857D-680E846B5BA3}");
const Guid c_tplDiffuseParams(L"{4AC7418D-FF43-FE40-ADDC-33A162636FDC}");
const Guid c_tplNormalParams(L"{77489017-FBE8-4A4F-B11A-FDE48C69E021}");
const Guid c_tplOutput(L"{6DA4BE0A-BE19-4440-9B08-FC3FD1FFECDC}");
const Guid c_tplSpecularParams(L"{68DA66E7-1D9E-FD4C-9692-D947BEA3EBAD}");
const Guid c_tplVertexParams(L"{AEBE83FB-68D4-9D45-A672-0A8487A197CD}");
const Guid c_implDiffuseConst(L"{BA68E2CA-77EB-684E-AD2B-0CD4BC35608D}");
const Guid c_implNormalConst(L"{5D881AE1-B99D-8941-B949-4E95AEF1CB7A}");
const Guid c_implOutputAdd(L"{321B8969-32D7-D44A-BF91-B056E4728DE2}");
const Guid c_implOutputAlpha(L"{1CDA749C-D713-974F-8E84-895AFEE8D552}");
const Guid c_implOutputDecal(L"{31FD2B2B-3D3C-024F-9AA6-544B73D6009C}");
const Guid c_implOutputMultiply(L"{C635E09A-8DFD-BF40-A863-81301D2388AC}");
const Guid c_implSpecularConst(L"{6D818781-04A2-2E48-9340-BEFB493F1F9E}");
const Guid c_implVertex(L"{5CCADFD7-6421-9848-912E-205358848F37}");

class FragmentReaderAdapter : public render::FragmentLinker::FragmentReader
{
public:
	FragmentReaderAdapter(db::Database* database)
	:	m_database(database)
	{
	}

	virtual Ref< const render::ShaderGraph > read(const Guid& fragmentGuid)
	{
		return m_database->getObjectReadOnly< render::ShaderGraph >(fragmentGuid);
	}

private:
	Ref< db::Database > m_database;
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.MaterialShaderGenerator", MaterialShaderGenerator, Object)

MaterialShaderGenerator::MaterialShaderGenerator(db::Database* database)
:	m_database(database)
{
}

Ref< render::ShaderGraph > MaterialShaderGenerator::generate(const model::Material& material) const
{
	Ref< render::ShaderGraph > materialShaderGraph = m_database->getObjectReadOnly< render::ShaderGraph >(c_materialShader);
	T_ASSERT (materialShaderGraph);

	RefArray< render::External > externalNodes;
	materialShaderGraph->findNodesOf< render::External >(externalNodes);

	for (RefArray< render::External >::iterator i = externalNodes.begin(); i != externalNodes.end(); ++i)
	{
		const Guid& fragmentGuid = (*i)->getFragmentGuid();
		T_ASSERT (fragmentGuid.isValid());

		if (fragmentGuid == c_tplDiffuseParams)
			(*i)->setFragmentGuid(c_implDiffuseConst);
		else if (fragmentGuid == c_tplNormalParams)
			(*i)->setFragmentGuid(c_implNormalConst);
		else if (fragmentGuid == c_tplOutput)
		{
			switch (material.getBlendOperator())
			{
			default:
			case model::Material::BoDecal:
				(*i)->setFragmentGuid(c_implOutputDecal);
				break;
			case model::Material::BoAdd:
				(*i)->setFragmentGuid(c_implOutputAdd);
				break;
			case model::Material::BoMultiply:
				(*i)->setFragmentGuid(c_implOutputMultiply);
				break;
			case model::Material::BoAlpha:
				(*i)->setFragmentGuid(c_implOutputAlpha);
				break;
			}
		}
		else if (fragmentGuid == c_tplSpecularParams)
			(*i)->setFragmentGuid(c_implSpecularConst);
		else if (fragmentGuid == c_tplVertexParams)
			(*i)->setFragmentGuid(c_implVertex);
	}

	FragmentReaderAdapter fragmentReader(m_database);
	materialShaderGraph = render::FragmentLinker(fragmentReader).resolve(materialShaderGraph, false);
	if (!materialShaderGraph)
		return 0;

	const RefArray< render::Node >& nodes = materialShaderGraph->getNodes();
	for (RefArray< render::Node >::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
	{
		std::wstring comment = (*i)->getComment();
		if (comment == L"Tag_DiffuseColor")
		{
			render::Color* colorNode = checked_type_cast< render::Color* >(*i);
			colorNode->setComment(L"");
			colorNode->setColor(material.getColor());
		}
		else if (comment == L"Tag_DiffuseTerm")
		{
			render::Scalar* diffuseTermNode = checked_type_cast< render::Scalar* >(*i);
			diffuseTermNode->setComment(L"");
			diffuseTermNode->set(material.getDiffuseTerm());
		}
		else if (comment == L"Tag_SpecularTerm")
		{
			render::Scalar* specularTermNode = checked_type_cast< render::Scalar* >(*i);
			specularTermNode->setComment(L"");
			specularTermNode->set(material.getSpecularTerm());
		}
		else if (comment == L"Tag_SpecularRoughness")
		{
			render::Scalar* specularRoughnessNode = checked_type_cast< render::Scalar* >(*i);
			specularRoughnessNode->setComment(L"");
			specularRoughnessNode->set(material.getSpecularRoughness());
		}
	}

	return materialShaderGraph;
}

void MaterialShaderGenerator::addDependencies(editor::IPipelineDepends* pipelineDepends)
{
	pipelineDepends->addDependency(c_materialShader, editor::PdfUse);
	pipelineDepends->addDependency(c_tplDiffuseParams, editor::PdfUse);
	pipelineDepends->addDependency(c_tplNormalParams, editor::PdfUse);
	pipelineDepends->addDependency(c_tplOutput, editor::PdfUse);
	pipelineDepends->addDependency(c_tplSpecularParams, editor::PdfUse);
	pipelineDepends->addDependency(c_tplVertexParams, editor::PdfUse);
	pipelineDepends->addDependency(c_implDiffuseConst, editor::PdfUse);
	pipelineDepends->addDependency(c_implNormalConst, editor::PdfUse);
	pipelineDepends->addDependency(c_implOutputAdd, editor::PdfUse);
	pipelineDepends->addDependency(c_implOutputAlpha, editor::PdfUse);
	pipelineDepends->addDependency(c_implOutputDecal, editor::PdfUse);
	pipelineDepends->addDependency(c_implOutputMultiply, editor::PdfUse);
	pipelineDepends->addDependency(c_implSpecularConst, editor::PdfUse);
	pipelineDepends->addDependency(c_implVertex, editor::PdfUse);
}

	}
}
