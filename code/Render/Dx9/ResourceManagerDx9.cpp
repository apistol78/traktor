/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#include "Core/Thread/Acquire.h"
#include "Render/Dx9/IResourceDx9.h"
#include "Render/Dx9/ResourceManagerDx9.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ResourceManagerDx9", ResourceManagerDx9, Object)

void ResourceManagerDx9::add(IResourceDx9* resource)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	T_ASSERT_M (std::find(m_resources.begin(), m_resources.end(), resource) == m_resources.end(), L"Resource already added");
	m_resources.push_back(resource);
}

void ResourceManagerDx9::remove(IResourceDx9* resource)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	std::list< IResourceDx9* >::iterator i = std::find(m_resources.begin(), m_resources.end(), resource);
	if (i != m_resources.end())
		m_resources.erase(i);
}

HRESULT ResourceManagerDx9::lostDevice()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	for (std::list< IResourceDx9* >::iterator i = m_resources.begin(); i != m_resources.end(); ++i)
	{
		HRESULT hr = (*i)->lostDevice();
		if (FAILED(hr))
			return hr;
	}
	return S_OK;
}

HRESULT ResourceManagerDx9::resetDevice(IDirect3DDevice9* d3dDevice)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	for (std::list< IResourceDx9* >::iterator i = m_resources.begin(); i != m_resources.end(); ++i)
	{
		HRESULT hr = (*i)->resetDevice(d3dDevice);
		if (FAILED(hr))
			return hr;
	}
	return S_OK;
}

	}
}
