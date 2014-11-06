#include "Amalgam/Engine/Classes/MeshClasses.h"
#include "Mesh/IMeshParameterCallback.h"
#include "Mesh/MeshEntity.h"
#include "Mesh/Blend/BlendMeshEntity.h"
#include "Mesh/Composite/CompositeMeshEntity.h"
#include "Mesh/Indoor/IndoorMeshEntity.h"
#include "Mesh/Instance/InstanceMeshEntity.h"
#include "Mesh/Lod/LodMeshEntity.h"
#include "Mesh/Partition/PartitionMeshEntity.h"
#include "Mesh/Skinned/SkinnedMeshEntity.h"
#include "Mesh/Static/StaticMeshEntity.h"
#include "Mesh/Stream/StreamMeshEntity.h"
#include "Render/ITexture.h"
#include "Render/Context/ProgramParameters.h"
#include "Script/AutoScriptClass.h"
#include "Script/Boxes.h"
#include "Script/IScriptDelegate.h"
#include "Script/IScriptManager.h"

namespace traktor
{
	namespace amalgam
	{
		namespace
		{

class BoxedProgramParameters : public Object
{
	T_RTTI_CLASS;

public:
	BoxedProgramParameters()
	:	m_programParameters(0)
	{
	}

	void setProgramParameters(render::ProgramParameters* programParameters)
	{
		m_programParameters = programParameters;
	}

	void setFloatParameter(const std::wstring& name, float param)
	{
		m_programParameters->setFloatParameter(name, param);
	}

	void setVectorParameter(const std::wstring& name, const Vector4& param)
	{
		m_programParameters->setVectorParameter(name, param);
	}

	void setTextureParameter(const std::wstring& name,render:: ITexture* texture)
	{
		m_programParameters->setTextureParameter(name, texture);
	}

	void setStencilReference(uint32_t stencilReference)
	{
		m_programParameters->setStencilReference(stencilReference);
	}

private:
	render::ProgramParameters* m_programParameters;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.BoxedProgramParameters", BoxedProgramParameters, Object)

class DelegateMeshParameterCallback : public mesh::IMeshParameterCallback
{
public:
	DelegateMeshParameterCallback(script::IScriptDelegate* callback)
	:	m_callback(callback)
	{
	}

	virtual void setCombination(render::Shader* shader) const
	{
	}

	virtual void setParameters(render::ProgramParameters* programParameters) const
	{
		m_programParameters.setProgramParameters(programParameters);
		script::Any argv[] =
		{
			script::CastAny< BoxedProgramParameters* >::set(&m_programParameters)
		};
		m_callback->call(sizeof_array(argv), argv);
	}

private:
	Ref< script::IScriptDelegate > m_callback;
	mutable BoxedProgramParameters m_programParameters;
};

void mesh_MeshEntity_setParameterCallback(mesh::MeshEntity* self, script::IScriptDelegate* callback)
{
	if (callback)
		self->setParameterCallback(new DelegateMeshParameterCallback(callback));
	else
		self->setParameterCallback(0);
}

		}

void registerMeshClasses(script::IScriptManager* scriptManager)
{
	Ref< script::AutoScriptClass< BoxedProgramParameters > > classBoxedProgramParameters = new script::AutoScriptClass< BoxedProgramParameters >();
	classBoxedProgramParameters->addMethod("setFloatParameter", &BoxedProgramParameters::setFloatParameter);
	classBoxedProgramParameters->addMethod("setVectorParameter", &BoxedProgramParameters::setVectorParameter);
	classBoxedProgramParameters->addMethod("setTextureParameter", &BoxedProgramParameters::setTextureParameter);
	classBoxedProgramParameters->addMethod("setStencilReference", &BoxedProgramParameters::setStencilReference);
	scriptManager->registerClass(classBoxedProgramParameters);

	Ref< script::AutoScriptClass< mesh::MeshEntity > > classMeshEntity = new script::AutoScriptClass< mesh::MeshEntity >();
	classMeshEntity->addMethod("setParameterCallback", mesh_MeshEntity_setParameterCallback);
	scriptManager->registerClass(classMeshEntity);

	Ref< script::AutoScriptClass< mesh::BlendMeshEntity > > classBlendMeshEntity = new script::AutoScriptClass< mesh::BlendMeshEntity >();
	classBlendMeshEntity->addMethod("setBlendWeights", &mesh::BlendMeshEntity::setBlendWeights);
	classBlendMeshEntity->addMethod("getBlendWeights", &mesh::BlendMeshEntity::getBlendWeights);
	scriptManager->registerClass(classBlendMeshEntity);

	Ref< script::AutoScriptClass< mesh::CompositeMeshEntity > > classCompositeMeshEntity = new script::AutoScriptClass< mesh::CompositeMeshEntity >();
	classCompositeMeshEntity->addMethod("addMeshEntity", &mesh::CompositeMeshEntity::addMeshEntity);
	classCompositeMeshEntity->addMethod("getMeshEntity", &mesh::CompositeMeshEntity::getMeshEntity);
	scriptManager->registerClass(classCompositeMeshEntity);

	Ref< script::AutoScriptClass< mesh::IndoorMeshEntity > > classIndoorMeshEntity = new script::AutoScriptClass< mesh::IndoorMeshEntity >();
	scriptManager->registerClass(classIndoorMeshEntity);

	Ref< script::AutoScriptClass< mesh::InstanceMeshEntity > > classInstanceMeshEntity = new script::AutoScriptClass< mesh::InstanceMeshEntity >();
	scriptManager->registerClass(classInstanceMeshEntity);

	Ref< script::AutoScriptClass< mesh::LodMeshEntity > > classLodMeshEntity = new script::AutoScriptClass< mesh::LodMeshEntity >();
	scriptManager->registerClass(classLodMeshEntity);

	Ref< script::AutoScriptClass< mesh::PartitionMeshEntity > > classPartitionMeshEntity = new script::AutoScriptClass< mesh::PartitionMeshEntity >();
	scriptManager->registerClass(classPartitionMeshEntity);

	Ref< script::AutoScriptClass< mesh::SkinnedMeshEntity > > classSkinnedMeshEntity = new script::AutoScriptClass< mesh::SkinnedMeshEntity >();
	scriptManager->registerClass(classSkinnedMeshEntity);

	Ref< script::AutoScriptClass< mesh::StaticMeshEntity > > classStaticMeshEntity = new script::AutoScriptClass< mesh::StaticMeshEntity >();
	scriptManager->registerClass(classStaticMeshEntity);

	Ref< script::AutoScriptClass< mesh::StreamMeshEntity > > classStreamMeshEntity = new script::AutoScriptClass< mesh::StreamMeshEntity >();
	classStreamMeshEntity->addMethod("getFrameCount", &mesh::StreamMeshEntity::getFrameCount);
	classStreamMeshEntity->addMethod("setFrame", &mesh::StreamMeshEntity::setFrame);
	scriptManager->registerClass(classStreamMeshEntity);
}

	}
}
