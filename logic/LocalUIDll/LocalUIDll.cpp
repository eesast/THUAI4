#include "UI.h"

using namespace THUnity2D;

namespace LocalUIDll
{
	public ref class LocalUI
	{
	public:

		static int Main()
		{
			UI ui;
			return ui.Begin();
		}

	};
}
