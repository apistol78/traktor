#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes/BoxedAlignedVector.h"
#include "Core/Class/Boxes/BoxedVector4.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Class/IRuntimeDelegate.h"
#include "Mesh/IMeshParameterCallback.h"
#include "Mesh/MeshClassFactory.h"
#include "Mesh/MeshComponent.h"
#include "Mesh/Blend/BlendMesh.h"
#include "Mesh/Blend/BlendMeshComponent.h"
#include "Mesh/Composite/CompositeMeshComponent.h"
#include "Mesh/Indoor/IndoorMesh.h"
#include "Mesh/Indoor/IndoorMeshComponent.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "Mesh/Instance/InstanceMeshComponent.h"
#include "Mesh/Lod/AutoLodMesh.h"
#include "Mesh/Lod/AutoLodMeshComponent.h"
#include "Mesh/Partition/PartitionMesh.h"
#include "Mesh/Partition/PartitionMeshComponent.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "Mesh/Skinned/SkinnedMeshComponent.h"
#include "Mesh/Static/StaticMesh.h"
#include "Mesh/Static/StaticMeshComponent.h"
#include "Mesh/Stream/StreamMesh.h"
#include "Mesh/Stream/StreamMeshComponent.h"
#include "Render/ITexture.h"
#include "Render/Context/ProgramParameters.h"
#include "Resource/ResourceCast.h"

namespace traktor
{
	namespace mesh
	{
		namespace
		{

class BoxedProgramParameters : public Object
{
	T_RTTI_CLASS;

public:
	BoxedProgramParameters()
	:	m_programParameters(nullptr)
	{
	}

	void setProgramParameters(render::ProgramParameters* programParameters)
	{
		m_programParameters = programParameters;
	}

	void setFloatParameter(const std::wstring& name, float param)
	{
		auto handle = render::getParameterHandle(name);
		m_programParameters->setFloatParameter(handle, param);
	}

	void setVectorParameter(const std::wstring& name, const Vector4& param)
	{
		auto handle = render::getParameterHandle(name);
		m_programParameters->setVectorParameter(handle, param);
	}

	void setVectorArrayParameter(const std::wstring& name, const AlignedVector< Vector4 >& param)
	{
		auto handle = render::getParameterHandle(name);
		m_programParameters->setVectorArrayParameter(handle, param.c_ptr(), (int)param.size());
	}

	void setTextureParameter(const std::wstring& name, render::ITexture* texture)
	{
		auto handle = render::getParameterHandle(name);
		m_programParameters->setTextureParameter(handle, texture);
	}

	void setStencilReference(uint32_t stencilReference)
	{
		m_programParameters->setStencilReference(stencilReference);
	}

private:
	render::ProgramParameters* m_programParameters;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.ProgramParameters", BoxedProgramParameters, Object)

class DelegateMeshParameterCallback : public RefCountImpl< IMeshParameterCallback >
{
public:
	DelegateMeshParameterCallback(IRuntimeDelegate* callback)
	:	m_callback(callback)
	{
	}

	virtual void setCombination(render::Shader* shader) const
	{
	}

	virtual void setParameters(render::ProgramParameters* programParameters) const
	{
		m_programParameters.setProgramParameters(programParameters);
		Any argv[] =
		{
			CastAny< BoxedProgramParameters* >::set(&m_programParameters)
		};
		m_callback->call(sizeof_array(argv), argv);
	}

private:
	Ref< IRuntimeDelegate > m_callback;
	mutable BoxedProgramParameters m_programParameters;
};

void MeshComponent_setParameterCallback(MeshComponent* self, IRuntimeDelegate* callback)
{
	if (callback)
		self->setParameterCallback(new DelegateMeshParameterCallback(callback));
	else
		self->setParameterCallback(nullptr);
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.MeshClassFactory", 0, MeshClassFactory, IRuntimeClassFactory)

void MeshClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	auto classBoxedProgramParameters = new AutoRuntimeClass< BoxedProgramParameters >();
	classBoxedProgramParameters->addMethod("setFloatParameter", &BoxedProgramParameters::setFloatParameter);
	classBoxedProgramParameters->addMethod("setVectorParameter", &BoxedProgramParameters::setVectorParameter);
	classBoxedProgramParameters->addMethod("setVectorArrayParameter", &BoxedProgramParameters::setVectorArrayParameter);
	classBoxedProgramParameters->addMethod("setTextureParameter", &BoxedProgramParameters::setTextureParameter);
	classBoxedProgramParameters->addMethod("setStencilReference", &BoxedProgramParameters::setStencilReference);
	registrar->registerClass(classBoxedProgramParameters);

	auto classIMesh = new AutoRuntimeClass< IMesh >();
	registrar->registerClass(classIMesh);

	auto classMeshComponent = new AutoRuntimeClass< MeshComponent >();
	classMeshComponent->addMethod("setParameterCallback", MeshComponent_setParameterCallback);
	registrar->registerClass(classMeshComponent);

	auto classBlendMesh = new AutoRuntimeClass< BlendMesh >();
	registrar->registerClass(classBlendMesh);

	auto classBlendMeshComponent = new AutoRuntimeClass< BlendMeshComponent >();
	classBlendMeshComponent->addConstructor< const resource::Proxy< BlendMesh >&, bool >();
	classBlendMeshComponent->addMethod("setBlendWeights", &BlendMeshComponent::setBlendWeights);
	classBlendMeshComponent->addMethod("getBlendWeights", &BlendMeshComponent::getBlendWeights);
	registrar->registerClass(classBlendMeshComponent);

	auto classCompositeMeshComponent = new AutoRuntimeClass< CompositeMeshComponent >();
	classCompositeMeshComponent->addConstructor();
	classCompositeMeshComponent->addMethod("removeAll", &CompositeMeshComponent::removeAll);
	classCompositeMeshComponent->addMethod("remove", &CompositeMeshComponent::remove);
	classCompositeMeshComponent->addMethod("add", &CompositeMeshComponent::add);
	registrar->registerClass(classCompositeMeshComponent);

	auto classIndoorMesh = new AutoRuntimeClass< IndoorMesh >();
	registrar->registerClass(classIndoorMesh);

	auto classIndoorMeshComponent = new AutoRuntimeClass< IndoorMeshComponent >();
	classIndoorMeshComponent->addConstructor< const resource::Proxy< IndoorMesh >&, bool >();
	registrar->registerClass(classIndoorMeshComponent);

	auto classInstanceMesh = new AutoRuntimeClass< InstanceMesh >();
	registrar->registerClass(classInstanceMesh);

	auto classInstanceMeshComponent = new AutoRuntimeClass< InstanceMeshComponent >();
	classInstanceMeshComponent->addConstructor< const resource::Proxy< InstanceMesh >&, bool >();
	registrar->registerClass(classInstanceMeshComponent);

	auto classAutoLodMesh = new AutoRuntimeClass< AutoLodMesh >();
	registrar->registerClass(classAutoLodMesh);

	auto classAutoLodMeshComponent = new AutoRuntimeClass< AutoLodMeshComponent >();
	classAutoLodMeshComponent->addConstructor< const resource::Proxy< AutoLodMesh >&, bool >();
	registrar->registerClass(classAutoLodMeshComponent);

	auto classPartitionMesh = new AutoRuntimeClass< PartitionMesh >();
	registrar->registerClass(classPartitionMesh);

	auto classPartitionMeshComponent = new AutoRuntimeClass< PartitionMeshComponent >();
	classPartitionMeshComponent->addConstructor< const resource::Proxy< PartitionMesh >&, bool >();
	registrar->registerClass(classPartitionMeshComponent);

	auto classSkinnedMesh = new AutoRuntimeClass< SkinnedMesh >();
	registrar->registerClass(classSkinnedMesh);

	auto classSkinnedMeshComponent = new AutoRuntimeClass< SkinnedMeshComponent >();
	classSkinnedMeshComponent->addConstructor< const resource::Proxy< SkinnedMesh >&, bool >();
	registrar->registerClass(classSkinnedMeshComponent);

	auto classStaticMesh = new AutoRuntimeClass< StaticMesh >();
	registrar->registerClass(classStaticMesh);

	auto classStaticMeshComponent = new AutoRuntimeClass< StaticMeshComponent >();
	classStaticMeshComponent->addConstructor< const resource::Proxy< StaticMesh >&, bool >();
	registrar->registerClass(classStaticMeshComponent);

	auto classStreamMesh = new AutoRuntimeClass< StreamMesh >();
	registrar->registerClass(classStreamMesh);

	auto classStreamMeshComponent = new AutoRuntimeClass< StreamMeshComponent >();
	classStreamMeshComponent->addConstructor< const resource::Proxy< StreamMesh >&, bool >();
	classStreamMeshComponent->addProperty("frameCount", &StreamMeshComponent::getFrameCount);
	classStreamMeshComponent->addProperty("frame", &StreamMeshComponent::setFrame, &StreamMeshComponent::getFrame);
	registrar->registerClass(classStreamMeshComponent);
}

	}
}
