#include <Windows.h>
#include "xout.hpp"
#include "xout_server.hpp"


constexpr bool local_example = false;

// Example of using xout for remote applications
// Requires xout.hpp & xout_server.hpp
void run_server_example()
{
	// That's it!
	xout::server::listen(100);
}

// Example of using xout for local application
// Only requires xout.hpp
void run_local_example()
{
	// Not explicitly needed, will init on first print
	xout::init();Sleep(1000);

	// Uses std::vformat
	xout::log("{1:<7} {0:>6} [ {2:+06.2f} {3:x} {4:#08b} {5:.3f} ]", "World!", "Hello,", 3.14159, 255, 5, 0.125);
	Sleep(250);

	// Also supports UTF-16 strings
	xout::info(L"مرحبا عالم!");
	Sleep(250);

	xout::debug(L"Привіт, Світ!");
	Sleep(250);

	xout::warning(L"{1:<7} {0:>6}", L"世界!", L"你好,");
	Sleep(250);

	xout::error("{1:<7} {0:>6}", "An ERROR!", "AH,");
	Sleep(250);

	// Supports std::string and std::wstring as well
	std::wstring crit = L"CRITICAL ERROR! LIMIT BREAK!!";
	xout::critical(crit);
	Sleep(250);

	// Send custom messages as well!
	std::wstring custom_msg = L"DANGER TO MANIFOLD!";
	xout::custom(custom_msg, L"!", 0x000000, 0xDC6626);

	// Returns true once xout window is closed
	while (!xout::should_exit()) { Sleep(100); }
}

int APIENTRY wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
	if (local_example){
		run_local_example();
	}else
	{
		run_server_example();
	}
	return 0;
}
