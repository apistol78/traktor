/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_PsslContext_H
#define traktor_render_PsslContext_H

#include <map>
#include "Render/Ps4/Pssl/PsslEmitter.h"
#include "Render/Ps4/Pssl/PsslShader.h"

namespace traktor
{
	namespace render
	{

class PsslVariable;
class InputPin;
class OutputPin;
class ShaderGraph;

/*!
 * \ingroup GNM
 */
class PsslContext
{
public:
	PsslContext(const ShaderGraph* shaderGraph);

	Node* getInputNode(const InputPin* inputPin);

	Node* getInputNode(Node* node, const std::wstring& inputPinName);

	void emit(Node* node);

	PsslVariable* emitInput(const InputPin* inputPin);

	PsslVariable* emitInput(Node* node, const std::wstring& inputPinName);

	PsslVariable* emitOutput(Node* node, const std::wstring& outputPinName, PsslType type);

	void emitOutput(Node* node, const std::wstring& outputPinName, PsslVariable* variable);

	void findNonDependentOutputs(Node* node, const std::wstring& inputPinName, const std::vector< const OutputPin* >& dependentOutputPins, std::vector< const OutputPin* >& outOutputPins) const;

	void findCommonOutputs(Node* node, const std::wstring& inputPin1, const std::wstring& inputPin2, std::vector< const OutputPin* >& outOutputPins) const;

	void enterVertex();

	void enterPixel();

	bool inVertex() const;

	bool inPixel() const;

	bool allocateInterpolator(int32_t width, int32_t& outId, int32_t& outOffset);

	PsslShader& getVertexShader();

	PsslShader& getPixelShader();

	PsslShader& getShader();

	PsslEmitter& getEmitter();

	//D3D11_RASTERIZER_DESC& getD3DRasterizerDesc();

	//D3D11_DEPTH_STENCIL_DESC& getD3DDepthStencilDesc();

	//D3D11_BLEND_DESC& getD3DBlendDesc();

	//void setStencilReference(uint32_t stencilReference);

	//uint32_t getStencilReference() const;

private:
	Ref< const ShaderGraph > m_shaderGraph;
	PsslShader m_vertexShader;
	PsslShader m_pixelShader;
	PsslShader* m_currentShader;
	PsslEmitter m_emitter;
	std::vector< uint8_t > m_interpolatorMap;
	//D3D11_RASTERIZER_DESC m_d3dRasterizerDesc;
	//D3D11_DEPTH_STENCIL_DESC m_d3dDepthStencilDesc;
	//D3D11_BLEND_DESC m_d3dBlendDesc;
	//uint32_t m_stencilReference;
};

	}
}

#endif	// traktor_render_PsslContext_H
