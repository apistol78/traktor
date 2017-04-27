/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_ResourceManagerDx9_H
#define traktor_render_ResourceManagerDx9_H

#include <list>
#include "Core/Object.h"
#include "Core/Thread/Semaphore.h"

namespace traktor
{
	namespace render
	{

class IResourceDx9;

class ResourceManagerDx9 : public Object
{
	T_RTTI_CLASS;

public:
	void add(IResourceDx9* resource);

	void remove(IResourceDx9* resource);

	HRESULT lostDevice();

	HRESULT resetDevice(IDirect3DDevice9* d3dDevice);

private:
	Semaphore m_lock;
	std::list< IResourceDx9* > m_resources;
};

	}
}

#endif	// traktor_render_ResourceManagerDx9_H
