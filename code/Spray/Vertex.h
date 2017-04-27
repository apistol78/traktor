/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_spray_Vertex_H
#define traktor_spray_Vertex_H

namespace traktor
{
	namespace spray
	{

#pragma pack(1)
struct EmitterVertex
{
	float positionAndOrientation[4];
	float velocityAndRandom[4];
	float extentAlphaAndSize[4];
	float colorAndAge[4];
};
#pragma pack()

#pragma pack(1)
struct TrailVertex
{
	float position[4];
	float direction[4];
	float uv[4];
};
#pragma pack()

	}
}

#endif	// traktor_spray_Vertex_H
