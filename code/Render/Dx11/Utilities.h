/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Config.h"
#include "Render/Types.h"

namespace traktor
{
	namespace render
	{

/*! Setup MSAA sample descriptor.
 *
 * \param d3dDevice DirectX device.
 * \param sampleCount Number of samples.
 * \param colorFormat Color buffer format.
 * \param outSampleDesc DXGI Sample descriptor.
 * \return True if successful.
 */
bool setupSampleDesc(ID3D11Device* d3dDevice, uint32_t sampleCount, DXGI_FORMAT colorFormat, DXGI_SAMPLE_DESC& outSampleDesc);

/*! Setup MSAA sample descriptor.
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

/*! Find matching DXGI display mode descriptor.
 *
 * \param dxgiOutput Output device.
 * \param dm Display mode.
 * \param outDxgiDisplayMode DXGI display mode descriptor.
 * \return True if display mode found.
 */
bool findDxgiDisplayMode(IDXGIOutput* dxgiOutput, const DisplayMode& dm, DXGI_MODE_DESC& outDxgiDisplayMode);

	}
}

