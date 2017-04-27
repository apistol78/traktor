/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Core/Thread/Acquire.h"
#include "Render/Dx9/VertexDeclCache.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const D3DDECLTYPE c_d3dType[] =
{
	D3DDECLTYPE_FLOAT1,
	D3DDECLTYPE_FLOAT2,
	D3DDECLTYPE_FLOAT3,
	D3DDECLTYPE_FLOAT4,
	D3DDECLTYPE_UBYTE4,
	D3DDECLTYPE_D3DCOLOR,
	D3DDECLTYPE_SHORT2,
	D3DDECLTYPE_SHORT4,
	D3DDECLTYPE_SHORT2N,
	D3DDECLTYPE_SHORT4N,
	D3DDECLTYPE_FLOAT16_2,
	D3DDECLTYPE_FLOAT16_4
};

const D3DDECLUSAGE c_d3dUsage[] =
{
	D3DDECLUSAGE_POSITION,
	D3DDECLUSAGE_NORMAL,
	D3DDECLUSAGE_TANGENT,
	D3DDECLUSAGE_BINORMAL,
	D3DDECLUSAGE_COLOR,
	D3DDECLUSAGE_TEXCOORD
}; 

struct VertexElementPred
{
	bool operator () (const VertexElement& lh, const VertexElement& rh) const
	{
		return
			lh.getDataUsage() == rh.getDataUsage() &&
			lh.getDataType() == rh.getDataType() &&
			lh.getOffset() == rh.getOffset() &&
			lh.getIndex() == rh.getIndex();
	}
};

		}

VertexDeclCache::VertexDeclCache(IDirect3DDevice9* d3dDevice)
:	m_d3dDevice(d3dDevice)
,	m_currentVertexDeclaration(0)
{
}

bool VertexDeclCache::createDeclaration(const std::vector< VertexElement >& vertexElements, ComRef< IDirect3DVertexDeclaration9 >& outVertexDeclaration, DWORD& outVertexStride)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	D3DVERTEXELEMENT9 d3dDeclaration[32];
	D3DVERTEXELEMENT9* d3dDeclarationLast = d3dDeclaration;
	HRESULT hr;

	// Check cache if declaration is already created.
	for (std::vector< Declaration >::iterator i = m_cache.begin(); i != m_cache.end(); ++i)
	{
		if (i->vertexElements.size() != vertexElements.size())
			continue;

		if (std::equal(i->vertexElements.begin(), i->vertexElements.end(), vertexElements.begin(), VertexElementPred()))
		{
			outVertexDeclaration = i->d3dVertexDeclaration;
			outVertexStride = i->d3dVertexStride;
			return true;
		}
	}

	// Create new declaration.
	for (std::vector< VertexElement >::const_iterator i = vertexElements.begin(); i != vertexElements.end(); ++i)
	{
		const VertexElement& vertexElement = *i;
		std::memset(d3dDeclarationLast, 0, sizeof(D3DVERTEXELEMENT9));
		d3dDeclarationLast->Stream = 0;
		d3dDeclarationLast->Offset = vertexElement.getOffset();
		d3dDeclarationLast->Type = c_d3dType[vertexElement.getDataType()];
		d3dDeclarationLast->Method = D3DDECLMETHOD_DEFAULT;
		d3dDeclarationLast->Usage = c_d3dUsage[vertexElement.getDataUsage()];
		d3dDeclarationLast->UsageIndex = vertexElement.getIndex();
		d3dDeclarationLast++;
	}

	std::memset(d3dDeclarationLast, 0, sizeof(D3DVERTEXELEMENT9));
	d3dDeclarationLast->Stream = 0xFF;
	d3dDeclarationLast->Offset = 0;
	d3dDeclarationLast->Type = D3DDECLTYPE_UNUSED;
	d3dDeclarationLast->Method = 0;
	d3dDeclarationLast->Usage = 0;
	d3dDeclarationLast->UsageIndex = 0;

	Declaration decl;

	decl.vertexElements = vertexElements;

	hr = m_d3dDevice->CreateVertexDeclaration(
		d3dDeclaration,
		&decl.d3dVertexDeclaration.getAssign()
	);
	if (FAILED(hr))
	{
		log::error << L"Unable to create vertex declaration, HRESULT = " << int32_t(hr) << Endl;
		return false;
	}

	decl.d3dVertexStride = D3DXGetDeclVertexSize(
		d3dDeclaration,
		0
	);

	m_cache.push_back(decl);

	outVertexDeclaration = decl.d3dVertexDeclaration;
	outVertexStride = decl.d3dVertexStride;

	return true;
}

void VertexDeclCache::setDeclaration(IDirect3DVertexDeclaration9* d3dVertexDeclaration)
{
	if (d3dVertexDeclaration != m_currentVertexDeclaration)
	{
		m_d3dDevice->SetVertexDeclaration(d3dVertexDeclaration);
		m_currentVertexDeclaration = d3dVertexDeclaration;
	}
}

HRESULT VertexDeclCache::lostDevice()
{
	m_currentVertexDeclaration = 0;
	m_d3dDevice.release();
	return S_OK;
}

HRESULT VertexDeclCache::resetDevice(IDirect3DDevice9* d3dDevice)
{
	m_d3dDevice = d3dDevice;
	return S_OK;
}

	}
}
