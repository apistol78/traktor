/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_Utilities_H
#define traktor_render_Utilities_H

#include "Core/Config.h"
#include "Render/Types.h"

namespace traktor
{
	namespace render
	{

/*! \brief Setup MSAA sample descriptor.
 *
 * \param d3dDevice DirectX device.
 * \param sampleCount Number of samples.
 * \param colorFormat Color buffer format.
 * \param outSampleDesc DXGI Sample descriptor.
 * \return True if successful.
 */
bool setupSampleDesc(ID3D11Device* d3dDevice, uint32_t sampleCount, DXGI_FORMAT colorFormat, DXGI_SAMPLE_DESC& outSampleDesc);

/*! \brief Setup MSAA sample descriptor.
 *
 * \param d3dDevice DirectX device.
 * \param sampleCount Number of samples.
 * \param colorFormat Color buffer format.
 * \param depthFormat Depth buffer format.
 * \param outSampleDesc DXGI Sample descriptor.
 * \return True if successful.
 */
bool setupSampleDesc(ID3D11Device* d3dDevice, uint32_t sampleCount, DXGI_FORMAT colorFormat, DXGI_FORMAT depthFormat, DXGI_SAMPLE_DESC& outSampleDesc);

/*! \brief
 */
uint32_t getDisplayModeCount(IDXGIOutput* dxgiOutput);

/*! \brief
*/
bool getDisplayMode(IDXGIOutput* dxgiOutput, uint32_t index, DisplayMode& outDisplayMode);

/*! \brief Find matching DXGI display mode descriptor.
 *
 * \param dxgiOutput Output device.
 * \param dm Display mode.
 * \param outDxgiDisplayMode DXGI display mode descriptor.
 * \return True if display mode found.
 */
bool findDxgiDisplayMode(IDXGIOutput* dxgiOutput, const DisplayMode& dm, DXGI_MODE_DESC& outDxgiDisplayMode);

	}
}

#endif	// traktor_render_Utilities_H
