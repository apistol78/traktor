#ifndef traktor_flash_AccShapeResourcem_H
#define traktor_flash_AccShapeResourcem_H

#include "Core/Object.h"
#include "Render/Shader.h"
#include "Resource/Proxy.h"

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace flash
	{

class AccShapeResources : public Object
{
	T_RTTI_CLASS;

public:
	bool create(resource::IResourceManager* resourceManager);

	void destroy();

private:
	friend class AccQuad;
	friend class AccShape;

	resource::Proxy< render::Shader > m_shaderSolid;
	resource::Proxy< render::Shader > m_shaderTextured;
	resource::Proxy< render::Shader > m_shaderSolidCurve;
	resource::Proxy< render::Shader > m_shaderTexturedCurve;
	resource::Proxy< render::Shader > m_shaderSolidMask;
	resource::Proxy< render::Shader > m_shaderTexturedMask;
	resource::Proxy< render::Shader > m_shaderSolidMaskCurve;
	resource::Proxy< render::Shader > m_shaderTexturedMaskCurve;
	resource::Proxy< render::Shader > m_shaderIncrementMask;
	resource::Proxy< render::Shader > m_shaderDecrementMask;

	render::handle_t m_handleTransform;
	render::handle_t m_handleFrameSize;
	render::handle_t m_handleViewSize;
	render::handle_t m_handleViewOffset;
	render::handle_t m_handleScreenOffsetScale;
	render::handle_t m_handleCxFormMul;
	render::handle_t m_handleCxFormAdd;
	render::handle_t m_handleTexture;
	render::handle_t m_handleTextureMatrix0;
	render::handle_t m_handleTextureMatrix1;
	render::handle_t m_handleTextureClamp;
	render::handle_t m_handleCurveSign;
};

	}
}

#endif	// traktor_flash_AccShapeResourcem_H
