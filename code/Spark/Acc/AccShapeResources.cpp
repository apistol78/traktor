#include "Spark/SwfTypes.h"
#include "Spark/Acc/AccShapeResources.h"
#include "Resource/IResourceManager.h"

namespace traktor
{
	namespace spark
	{
		namespace
		{

const resource::Id< render::Shader > c_idShaderSolid(Guid(L"{D46877B9-0F90-3A42-AB2D-7346AA607233}"));
const resource::Id< render::Shader > c_idShaderTextured(Guid(L"{5CDDBEC8-1629-0A4E-ACE5-C8186072D694}"));
const resource::Id< render::Shader > c_idShaderLine(Guid(L"{8CAA6CFE-014A-3F4A-80C1-BD8A21B9A8C4}"));
const resource::Id< render::Shader > c_idShaderIncrementMask(Guid(L"{8DCBCF05-4640-884E-95AC-F090510788F4}"));
const resource::Id< render::Shader > c_idShaderDecrementMask(Guid(L"{57F6F4DF-F4EE-6740-907C-027A3A2596D7}"));

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.AccShapeResources", AccShapeResources, Object)

bool AccShapeResources::create(resource::IResourceManager* resourceManager)
{
	if (!resourceManager->bind(c_idShaderSolid, m_shaderSolid))
		return false;
	if (!resourceManager->bind(c_idShaderTextured, m_shaderTextured))
		return false;
	if (!resourceManager->bind(c_idShaderLine, m_shaderLine))
		return false;
	if (!resourceManager->bind(c_idShaderIncrementMask, m_shaderIncrementMask))
		return false;
	if (!resourceManager->bind(c_idShaderDecrementMask, m_shaderDecrementMask))
		return false;

	m_handleTransform = render::getParameterHandle(L"Spark_Transform");
	m_handleClipBounds = render::getParameterHandle(L"Spark_ClipBounds");
	m_handleFrameBounds = render::getParameterHandle(L"Spark_FrameBounds");
	m_handleFrameTransform = render::getParameterHandle(L"Spark_FrameTransform");
	m_handleCxFormMul = render::getParameterHandle(L"Spark_CxFormMul");
	m_handleCxFormAdd = render::getParameterHandle(L"Spark_CxFormAdd");
	m_handleTexture = render::getParameterHandle(L"Spark_Texture");
	m_handleTextureClamp = render::getParameterHandle(L"Spark_TextureClamp");
	m_handleLineData = render::getParameterHandle(L"Spark_LineData");
	m_handleLineWidth = render::getParameterHandle(L"Spark_LineWidth");
	m_handleLineColor = render::getParameterHandle(L"Spark_LineColor");

	m_handleTechniques[SbmDefault] = render::getParameterHandle(L"Spark_Default");
	m_handleTechniques[SbmNormal] = render::getParameterHandle(L"Spark_Default");
	m_handleTechniques[SbmLayer] = render::getParameterHandle(L"Spark_Layer");
	m_handleTechniques[SbmMultiply] = render::getParameterHandle(L"Spark_Multiply");
	m_handleTechniques[SbmScreen] = render::getParameterHandle(L"Spark_Screen");
	m_handleTechniques[SbmLighten] = render::getParameterHandle(L"Spark_Lighten");
	m_handleTechniques[SbmDarken] = render::getParameterHandle(L"Spark_Darken");
	m_handleTechniques[SbmDifference] = render::getParameterHandle(L"Spark_Difference");
	m_handleTechniques[SbmAdd] = render::getParameterHandle(L"Spark_Add");
	m_handleTechniques[SbmSubtract] = render::getParameterHandle(L"Spark_Subtract");
	m_handleTechniques[SbmInvert] = render::getParameterHandle(L"Spark_Invert");
	m_handleTechniques[SbmAlpha] = render::getParameterHandle(L"Spark_Alpha");
	m_handleTechniques[SbmErase] = render::getParameterHandle(L"Spark_Erase");
	m_handleTechniques[SbmOverlay] = render::getParameterHandle(L"Spark_Overlay");
	m_handleTechniques[SbmHardlight] = render::getParameterHandle(L"Spark_Hardlight");
	m_handleTechniques[SbmOpaque] = render::getParameterHandle(L"Spark_Opaque");

	return true;
}

void AccShapeResources::destroy()
{
	m_shaderSolid.clear();
	m_shaderTextured.clear();
	m_shaderLine.clear();
	m_shaderIncrementMask.clear();
	m_shaderDecrementMask.clear();
}

	}
}
