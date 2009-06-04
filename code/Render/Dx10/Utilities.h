#ifndef traktor_render_Utilities_H
#define traktor_render_Utilities_H

#include "Core/Config.h"

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
bool setupSampleDesc(ID3D10Device* d3dDevice, uint32_t sampleCount, DXGI_FORMAT colorFormat, DXGI_SAMPLE_DESC& outSampleDesc);

/*! \brief Setup MSAA sample descriptor.
 *
 * \param d3dDevice DirectX device.
 * \param sampleCount Number of samples.
 * \param colorFormat Color buffer format.
 * \param depthFormat Depth buffer format.
 * \param outSampleDesc DXGI Sample descriptor.
 * \return True if successful.
 */
bool setupSampleDesc(ID3D10Device* d3dDevice, uint32_t sampleCount, DXGI_FORMAT colorFormat, DXGI_FORMAT depthFormat, DXGI_SAMPLE_DESC& outSampleDesc);

	}
}

#endif	// traktor_render_Utilities_H
