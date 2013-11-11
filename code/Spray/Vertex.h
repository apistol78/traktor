#ifndef traktor_spray_Vertex_H
#define traktor_spray_Vertex_H

namespace traktor
{
	namespace spray
	{

#pragma pack(1)
struct EmitterVertex
{
	struct PositionAndOrientation
	{
		float position[3];
		float orientation;
	}
	positionAndOrientation;
	
	struct VelocityAndRandom
	{
		float velocity[3];
		float random;
	}
	velocityAndRandom;

	struct Attributes1
	{
		float extent[2];
		float alpha;
		float size;
	}
	attrib1;

	struct Attributes2
	{
		float color[3];
		float age;
	}
	attrib2;
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
