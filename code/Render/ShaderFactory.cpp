#include "Render/ShaderFactory.h"
#include "Render/ShaderResource.h"
#include "Render/ShaderGraph.h"
#include "Render/Shader.h"
#include "Render/Nodes.h"
#include "Render/IRenderSystem.h"
#include "Render/IProgramCompiler.h"
#include "Render/IProgram.h"
#include "Render/TextureLinker.h"
#include "Render/ProgramResource.h"
#include "Resource/IResourceManager.h"
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
	TextureReaderAdapter(resource::IResourceManager* resourceManager)
	:	m_resourceManager(resourceManager)
	{
	}

	virtual Ref< ITexture > read(const Guid& textureGuid)
	{
		Ref< resource::IResourceHandle > handle = m_resourceManager->bind< ITexture >(textureGuid);
		if (handle)
		{
			Ref< Object > texture = handle->get();
			return dynamic_type_cast< ITexture* >(texture);
		}
		else
			return 0;
	}

private:
	Ref< resource::IResourceManager > m_resourceManager;
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderFactory", ShaderFactory, resource::IResourceFactory)

ShaderFactory::ShaderFactory(
	db::Database* database,
	IRenderSystem* renderSystem
)
:	m_database(database)
,	m_renderSystem(renderSystem)
{
}

const TypeInfoSet ShaderFactory::getResourceTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Shader >());
	return typeSet;
}

bool ShaderFactory::isCacheable() const
{
	return true;
}

Ref< Object > ShaderFactory::create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid)
{
	Ref< ShaderResource > shaderResource = m_database->getObjectReadOnly< ShaderResource >(guid);
	if (!shaderResource)
		return 0;

	Ref< Shader > shader = new Shader();

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

			Ref< ProgramResource > programResource;
			if (ShaderGraph* programShaderGraph = dynamic_type_cast< ShaderGraph* >(j->program))
			{
				Ref< render::IProgramCompiler > programCompiler = m_renderSystem->createProgramCompiler();
				if (!programCompiler)
					return 0;

				programResource = programCompiler->compile(programShaderGraph, 4, false);
				if (!programResource)
					return 0;

				RefArray< Sampler > samplerNodes;
				programShaderGraph->findNodesOf< Sampler >(samplerNodes);

				for (RefArray< Sampler >::iterator i = samplerNodes.begin(); i != samplerNodes.end(); ++i)
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
				programResource = checked_type_cast< ProgramResource* >(j->program);

			Shader::Combination combination;
			combination.parameterValue = j->parameterValue;
			combination.program = m_renderSystem->createProgram(programResource);
			if (!combination.program)
				return 0;

			TextureReaderAdapter textureReader(resourceManager);
			if (!TextureLinker(textureReader).link(programResource, combination.program))
				return 0;

			technique.combinations.push_back(combination);
		}
	}

	shader->setTechnique(L"Default");
	return shader;
}

	}
}
