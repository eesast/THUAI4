#include "UI.h"

namespace PlayBackPlayerDll
{
	public ref class PlayBackPlayerDll abstract sealed
	{
	public:
		static int Main(System::String^ fileName)
		{
			ShowWindow(GetConsoleWindow(), SW_HIDE);
			UI ui;
			return ui.Begin(fileName);
		}
	};
}
