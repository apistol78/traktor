/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Math/MathUtils.h"
#include "Core/Misc/AutoPtr.h"
#include "Render/Dx11/Platform.h"
#include "Render/Dx11/Utilities.h"

namespace traktor
{
	namespace render
	{

bool setupSampleDesc(ID3D11Device* d3dDevice, uint32_t sampleCount, DXGI_FORMAT colorFormat, DXGI_SAMPLE_DESC& outSampleDesc)
{
	HRESULT hr;

	if (sampleCount > 0)
	{
		outSampleDesc.Count = 0;
		outSampleDesc.Quality = 0;

		for (int32_t i = int32_t(sampleCount); i >= 1; --i)
		{
			UINT msQuality = 0;

			hr = d3dDevice->CheckMultisampleQualityLevels(
				colorFormat,
				i,
				&msQuality
			);

			if (SUCCEEDED(hr) && msQuality > 0)
			{
				outSampleDesc.Count = i;
				outSampleDesc.Quality = msQuality - 1;
				break;
			}
		}

		if (!outSampleDesc.Count)
			return false;
	}
	else
	{
		outSampleDesc.Count = 1;
		outSampleDesc.Quality = 0;
	}

	return true;
}

bool setupSampleDesc(ID3D11Device* d3dDevice, uint32_t sampleCount, DXGI_FORMAT colorFormat, DXGI_FORMAT depthFormat, DXGI_SAMPLE_DESC& outSampleDesc)
{
	HRESULT hr;

	if (sampleCount > 0)
	{
		outSampleDesc.Count = 0;
		for (uint32_t i = 1; i <= D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT; ++i)
		{
			UINT msQuality1 = 0;
			UINT msQuality2 = 0;

			hr = d3dDevice->CheckMultisampleQualityLevels(
				colorFormat,
				i,
				&msQuality1
			);

			if (SUCCEEDED(hr) && msQuality1 > 0)
			{
				hr = d3dDevice->CheckMultisampleQualityLevels(
					depthFormat,
					i,
					&msQuality2
				);

				if (SUCCEEDED(hr) && msQuality2 > 0)
				{
					outSampleDesc.Count = i;
					outSampleDesc.Quality = min< UINT >(msQuality1 - 1, msQuality2 - 1);
				}
			}
		}
		if (!outSampleDesc.Count)
			return false;
	}
	else
	{
		outSampleDesc.Count = 1;
		outSampleDesc.Quality = 0;
	}

	return true;
}

uint32_t getDisplayModeCount(IDXGIOutput* dxgiOutput)
{
	HRESULT hr;
	UINT count = 0;

	if (!dxgiOutput)
		return 0;

	hr = dxgiOutput->GetDisplayModeList(
		DXGI_FORMAT_R8G8B8A8_UNORM,
		0,
		&count,
		0
	);
	if (FAILED(hr) || !count)
		return 0;

	return uint32_t(count);
}

bool getDisplayMode(IDXGIOutput* dxgiOutput, uint32_t index, DisplayMode& outDisplayMode)
{
	HRESULT hr;
	UINT count = 0;

	if (!dxgiOutput)
		return false;

	hr = dxgiOutput->GetDisplayModeList(
		DXGI_FORMAT_R8G8B8A8_UNORM,
		0,
		&count,
		0
	);
	if (FAILED(hr) || !count)
		return false;
	
	if (index >= count)
		return false;

	AutoArrayPtr< DXGI_MODE_DESC > dxgiDisplayModes(new DXGI_MODE_DESC [count]);

	hr = dxgiOutput->GetDisplayModeList(
		DXGI_FORMAT_R8G8B8A8_UNORM,
		0,
		&count,
		dxgiDisplayModes.ptr()
	);
	if (FAILED(hr) || !count)
		return false;

	outDisplayMode.width = dxgiDisplayModes[index].Width;
	outDisplayMode.height = dxgiDisplayModes[index].Height;
	outDisplayMode.refreshRate = dxgiDisplayModes[index].RefreshRate.Numerator / dxgiDisplayModes[index].RefreshRate.Denominator;
	outDisplayMode.colorBits = 32;
	outDisplayMode.stereoscopic = false;

	return true;
}

bool findDxgiDisplayMode(IDXGIOutput* dxgiOutput, const DisplayMode& dm, DXGI_MODE_DESC& outDxgiDisplayMode)
{
	HRESULT hr;
	UINT count = 0;

	if (!dxgiOutput)
		return false;

	hr = dxgiOutput->GetDisplayModeList(
		DXGI_FORMAT_R8G8B8A8_UNORM,
		0,
		&count,
		0
	);
	if (FAILED(hr) || !count)
		return false;
	
	AutoArrayPtr< DXGI_MODE_DESC > dxgiDisplayModes(new DXGI_MODE_DESC [count]);

	hr = dxgiOutput->GetDisplayModeList(
		DXGI_FORMAT_R8G8B8A8_UNORM,
		0,
		&count,
		dxgiDisplayModes.ptr()
	);
	if (FAILED(hr) || !count)
		return false;

	for (UINT i = 0; i < count; ++i)
	{
		if (dm.width != dxgiDisplayModes[i].Width)
			continue;
		
		if (dm.height != dxgiDisplayModes[i].Height)
			continue;
		
		if (dm.refreshRate)
		{
			if (dm.refreshRate != dxgiDisplayModes[i].RefreshRate.Numerator / dxgiDisplayModes[i].RefreshRate.Denominator)
				continue;
		}
		
		outDxgiDisplayMode = dxgiDisplayModes[i];
		return true;
	}
	
	return false;
}

	}
}
