#include <CCubeMapProcessor.h>
#include <ErrorMsg.h>
#include "Core/Log/Log.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Drawing/Image.h"
#include "Illuminate/Editor/CubeMap.h"
#include "Illuminate/Editor/CubeProbe.h"

namespace traktor
{
	namespace illuminate
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.illuminate.CubeProbe", CubeProbe, IProbe)

CubeProbe::CubeProbe(const drawing::Image* cubeMap)
{
	Ref< drawing::Image > cubeMap4f = cubeMap->clone();
	cubeMap4f->convert(drawing::PixelFormat::getARGBF32());

	m_cubeMap = new CubeMap(cubeMap4f);
	int32_t size = m_cubeMap->getSize();

	CCubeMapProcessor cubeMapProcessor;
	cubeMapProcessor.Init(size, size, 1, 4);

	for (int32_t i = 0; i < 6; ++i)
	{
		cubeMapProcessor.SetInputFaceData(
			i,
			CP_VAL_FLOAT32,
			4,
			size * 4 * sizeof(float),
			(void*)m_cubeMap->getSide(i)->getData(),
			1000000.0f,
			1.0f,
			1.0f
		);
	}

	cubeMapProcessor.InitiateFiltering(
		20.0f,
		1.0f,
		2.0f,
		CP_FILTER_TYPE_ANGULAR_GAUSSIAN,
		CP_FIXUP_PULL_LINEAR,
		3,
		TRUE
	);

	while (cubeMapProcessor.GetStatus() == CP_STATUS_PROCESSING)
	{
		log::info << cubeMapProcessor.GetFilterProgressString() << Endl;
		ThreadManager::getInstance().getCurrentThread()->sleep(200);
	}

	for (int32_t i = 0; i < 6; ++i)
	{
		std::memcpy(
			(void*)m_cubeMap->getSide(i)->getData(),
			cubeMapProcessor.m_OutputSurface[0][i].m_ImgData,
			size * size * 4 * sizeof(float)
		);
	}

}

Color4f CubeProbe::sample(const Vector4& direction) const
{
	return m_cubeMap->get(direction);
}

	}
}
