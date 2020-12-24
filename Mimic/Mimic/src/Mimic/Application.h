#pragma once
#include "Core.h"

namespace Mimic
{


	class MIMIC_API Application
	{
	public:
		Application();
		virtual ~Application();

		void run();
	};

	Application* CreateApplication();
}
