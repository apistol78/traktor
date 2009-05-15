#include "Render/ShaderFactory.h"
#include "Render/ShaderResource.h"
#include "Render/ShaderGraph.h"
#include "Render/Shader.h"
#include "Render/Nodes.h"
#include "Render/RenderSystem.h"
#include "Render/ProgramResource.h"
#include "Render/Program.h"
#include "Render/TextureLinker.h"
#include "Resource/Proxy.h"
#include "Database/Database.h"
#include "Database/Instance.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

class TextureReaderAdapter : public TextureLinker::TextureReader
{
public:
	virtual Texture* read(const Guid& textureGuid)
	{
		return resource::Proxy< Texture >(textureGuid);
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderFactory", ShaderFactory, resource::ResourceFactory)

ShaderFactory::ShaderFactory(
	db::Database* db,
	RenderSystem* renderSystem
)
:	m_db(db)
,	m_renderSystem(renderSystem)
{
}

const TypeSet ShaderFactory::getResourceTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< Shader >());
	return typeSet;
}

Object* ShaderFactory::create(const Type& resourceType, const Guid& guid, bool& outCacheable)
{
	Ref< ShaderResource > shaderResource = m_db->getObjectReadOnly< ShaderResource >(guid);
	if (!shaderResource)
		return 0;

	Ref< Shader > shader = gc_new< Shader >();

	// Create combination parameter mapping.
	const std::map< std::wstring, uint32_t >& parameterBits = shaderResource->getParameterBits();
	for (std::map< std::wstring, uint32_t >::const_iterator i = parameterBits.begin(); i != parameterBits.end(); ++i)
		shader->m_parameterBits[getParameterHandle(i->first)] = i->second;

	// Create shader techniques.
	const std::vector< ShaderResource::Technique >& techniques = shaderResource->getTechniques();
	for (std::vector< ShaderResource::Technique >::const_iterator i = techniques.begin(); i != techniques.end(); ++i)
	{
		Shader::Technique& technique = shader->m_techniques[getParameterHandle(i->name)];
		technique.parameterMask = i->parameterMask;

		for (std::vector< ShaderResource::Combination >::const_iterator j = i->combinations.begin(); j != i->combinations.end(); ++j)
		{
			if (!j->program)
				continue;

			Ref< render::ProgramResource > programResource;
			if (render::ShaderGraph* programShaderGraph = dynamic_type_cast< render::ShaderGraph* >(j->program))
			{
				programResource = m_renderSystem->compileProgram(programShaderGraph, 4, false);
				if (!programResource)
					return 0;

				RefArray< render::Sampler > samplerNodes;
				programShaderGraph->findNodesOf< render::Sampler >(samplerNodes);

				for (RefArray< render::Sampler >::iterator i = samplerNodes.begin(); i != samplerNodes.end(); ++i)
				{
					const Guid& textureGuid = (*i)->getExternal();
					if (!textureGuid.isNull() && textureGuid.isValid())
					{
						programResource->m_textures.push_back(std::make_pair(
							(*i)->getParameterName(),
							textureGuid
						));
					}
				}
			}
			else
				programResource = checked_type_cast< render::ProgramResource* >(j->program);

			Shader::Combination combination;
			combination.parameterValue = j->parameterValue;
			combination.program = m_renderSystem->createProgram(programResource);
			if (!combination.program)
				return 0;

			TextureReaderAdapter textureReader;
			if (!render::TextureLinker(textureReader).link(programResource, combination.program))
				return 0;

			technique.combinations.push_back(combination);
		}
	}

	shader->setTechnique(L"Default");
	return shader;
}

	}
}
