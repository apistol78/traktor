#if defined(_WIN32)
#include <windows.h>
#endif
#include "UnitTest/CaseMeta.h"
#include "UnitTest/CaseProcess.h"
#include "UnitTest/CaseThread.h"
#include "UnitTest/CaseJob.h"
#include "UnitTest/CaseHeap.h"
#include "UnitTest/CaseMath.h"
#include "UnitTest/CaseMatrix44.h"
#include "UnitTest/CaseWinding.h"
#include "UnitTest/CasePath.h"
#include "UnitTest/CaseXmlPullParser.h"
#include "UnitTest/CaseXmlDocument.h"
#include "UnitTest/CaseZip.h"
#include "UnitTest/CaseSpatialHashArray.h"
//#include "UnitTest/CaseRenderSw.h"
#include "Core/Log/Log.h"

using namespace traktor;

#if !defined(_WIN32) || defined(_CONSOLE)
int main(int /*argc*/, const char** /*argv*/)
#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
#endif
{
	//log::info << L"Case meta" << Endl;
	//traktor::CaseMeta().run();

	//log::info << L"Case process" << Endl;
	//traktor::CaseProcess().run();

	//log::info << L"Case thread" << Endl;
	//traktor::CaseThread().run();

	//log::info << L"Case job" << Endl;
	//traktor::CaseJob().run();

	//log::info << L"Case heap" << Endl;
	//traktor::CaseHeap().run();

	//log::info << L"Case math" << Endl;
	//traktor::CaseMath().run();

	log::info << L"Case matrix 4x4" << Endl;
	traktor::CaseMatrix44().run();

	//log::info << L"Case winding" << Endl;
	//traktor::CaseWinding().run();

	//log::info << L"Case path" << Endl;
	//traktor::CasePath().run();

	//log::info << L"Case xml pull parser" << Endl;
	//traktor::CaseXmlPullParser().run();

	//log::info << L"Case xml document" << Endl;
	//traktor::CaseXmlDocument().run();

	//log::info << L"Case zip" << Endl;
	//traktor::CaseZip().run();

	//log::info << L"Case spatial hash array" << Endl;
	//traktor::CaseSpatialHashArray().run();

	////log::info << L"Case software renderer" << Endl;
	////traktor::CaseRenderSw().run();

	log::info << L"Finished" << Endl;
}
