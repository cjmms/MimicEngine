#pragma once
#include "Application.h"

#ifdef MC_PLATFORM_WINDOWS




int main(int argc, char** argv)
{
	auto app = Mimic::CreateApplication();
	app->run();
	delete app;
}

#endif