#ifndef traktor_model_LwSurface_H
#define traktor_model_LwSurface_H

#include <list>
#include "Core/Object.h"
#include "Core/Math/Color4f.h"

namespace traktor
{

class IStream;

	namespace model
	{

struct LwChunk;

//struct LwTexture
//{
//   char *ord;
//   uint32_t type;
//   uint32_t chan;
//   LwEParam opacity;
//   short opac_type;
//   short enabled;
//   short negative;
//   short axis;
//   union
//   {
//      lwImageMap     imap;
//      lwProcedural   proc;
//      lwGradient     grad;
//   }
//   param;
//   lwTMap tmap;
//};

struct LwEParam
{
   float val;
   uint32_t envelope;
};

struct LwVParam
{
   float val[3];
   uint32_t envelope;
};

struct LwTParam
{
   float val;
   uint32_t envelope;
//   std::list< LwTexture > textures;
};

struct LwCParam
{
   Color4f val;
   uint32_t envelope;
//   std::list< LwTexture > textures;
};

struct LwRMap
{
   LwTParam val;
   int options;
   int cindex;
   float seamAngle;
};

struct LwLine
{
   short enabled;
   uint16_t flags;
   LwEParam size;
};

class LwSurface : public Object
{
	T_RTTI_CLASS;

public:
	bool readLWO2(const LwChunk& chunk, IStream* stream);

	bool readLWOB(const LwChunk& chunk, IStream* stream);

	const std::wstring& getName() const { return m_name; }

	const std::wstring& getSource() const { return m_source; }

	const LwCParam& getColor() const { return m_color; }

private:
	std::wstring m_name;
	std::wstring m_source;
	LwCParam m_color;
	LwTParam m_luminosity;
	LwTParam m_diffuse;
	LwTParam m_specularity;
	LwTParam m_glossiness;
	LwRMap m_reflection;
	LwRMap m_transparency;
	LwTParam m_eta;
	LwTParam m_translucency;
	LwTParam m_bump;
	float m_smooth;
	int m_sideflags;
	float m_alpha;
	int m_alphaMode;
	LwEParam m_colorHilite;
	LwEParam m_colorFilter;
	LwEParam m_addTrans;
	LwEParam m_difSharp;
	LwEParam m_glow;
	LwLine m_line;
	std::wstring m_comment;
};

	}
}

#endif	// traktor_model_LwSurface_H
