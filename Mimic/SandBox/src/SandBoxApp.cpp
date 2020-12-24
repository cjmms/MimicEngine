
#include <Mimic.h>


class SandBox : public Mimic::Application
{
public:
	SandBox() {};
	~SandBox()
	{
	}
};


Mimic::Application* Mimic::CreateApplication()
{
	return new SandBox();
}