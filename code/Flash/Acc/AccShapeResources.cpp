#include "Flash/Acc/AccShapeResources.h"
#include "Resource/IResourceManager.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

const Guid c_guidShaderSolid(L"{4F6F6CCE-97EC-624D-96B7-842F1D99D060}");
const Guid c_guidShaderTextured(L"{049F4B08-1A54-DB4C-86CC-B533BCFFC65D}");

const Guid c_guidShaderSolidCurve(L"{E942960D-81C2-FD4C-B005-009902CBD91E}");
const Guid c_guidShaderTexturedCurve(L"{209E791F-C8E8-E646-973B-2910CC99C244}");

const Guid c_guidShaderSolidMask(L"{D46877B9-0F90-3A42-AB2D-7346AA607233}");
const Guid c_guidShaderTexturedMask(L"{5CDDBEC8-1629-0A4E-ACE5-C8186072D694}");

const Guid c_guidShaderSolidMaskCurve(L"{BDC662CF-8A6B-BE42-BAEE-B12313EC3DDC}");
const Guid c_guidShaderTexturedMaskCurve(L"{4FCA84E5-A055-BD48-8EAF-EB118B8C9BF7}");

const Guid c_guidShaderIncrementMask(L"{8DCBCF05-4640-884E-95AC-F090510788F4}");
const Guid c_guidShaderDecrementMask(L"{57F6F4DF-F4EE-6740-907C-027A3A2596D7}");

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AccShapeResources", AccShapeResources, Object)

bool AccShapeResources::create(resource::IResourceManager* resourceManager)
{
	m_shaderSolid = c_guidShaderSolid;
	if (!resourceManager->bind(m_shaderSolid))
		return false;
	m_shaderTextured = c_guidShaderTextured;
	if (!resourceManager->bind(m_shaderTextured))
		return false;
	m_shaderSolidCurve = c_guidShaderSolidCurve;
	if (!resourceManager->bind(m_shaderSolidCurve))
		return false;
	m_shaderTexturedCurve = c_guidShaderTexturedCurve;
	if (!resourceManager->bind(m_shaderTexturedCurve))
		return false;
	m_shaderSolidMask = c_guidShaderSolidMask;
	if (!resourceManager->bind(m_shaderSolidMask))
		return false;
	m_shaderTexturedMask = c_guidShaderTexturedMask;
	if (!resourceManager->bind(m_shaderTexturedMask))
		return false;

	m_shaderSolidMaskCurve = c_guidShaderSolidMaskCurve;
	if (!resourceManager->bind(m_shaderSolidMaskCurve))
		return false;
	m_shaderTexturedMaskCurve = c_guidShaderTexturedMaskCurve;
	if (!resourceManager->bind(m_shaderTexturedMaskCurve))
		return false;

	m_shaderIncrementMask = c_guidShaderIncrementMask;
	if (!resourceManager->bind(m_shaderIncrementMask))
		return false;
	m_shaderDecrementMask = c_guidShaderDecrementMask;
	if (!resourceManager->bind(m_shaderDecrementMask))
		return false;

	m_handleTransform = render::getParameterHandle(L"Flash_Transform");
	m_handleFrameSize = render::getParameterHandle(L"Flash_FrameSize");
	m_handleViewSize = render::getParameterHandle(L"Flash_ViewSize");
	m_handleViewOffset = render::getParameterHandle(L"Flash_ViewOffset");
	m_handleScreenOffsetScale = render::getParameterHandle(L"Flash_ScreenOffsetScale");
	m_handleCxFormMul = render::getParameterHandle(L"Flash_CxFormMul");
	m_handleCxFormAdd = render::getParameterHandle(L"Flash_CxFormAdd");
	m_handleTexture = render::getParameterHandle(L"Flash_Texture");
	m_handleTextureMatrix = render::getParameterHandle(L"Flash_TextureMatrix");
	m_handleCurveSign = render::getParameterHandle(L"Flash_CurveSign");

	return true;
}

void AccShapeResources::destroy()
{
	m_shaderSolid.clear();
	m_shaderTextured.clear();
	m_shaderSolidCurve.clear();
	m_shaderTexturedCurve.clear();
	m_shaderSolidMask.clear();
	m_shaderTexturedMask.clear();
	m_shaderIncrementMask.clear();
	m_shaderDecrementMask.clear();
}

void AccShapeResources::validate()
{
	m_shaderSolid.validate();
	m_shaderTextured.validate();
	m_shaderSolidCurve.validate();
	m_shaderTexturedCurve.validate();
	m_shaderSolidMask.validate();
	m_shaderTexturedMask.validate();
	m_shaderIncrementMask.validate();
	m_shaderDecrementMask.validate();
}

	}
}
