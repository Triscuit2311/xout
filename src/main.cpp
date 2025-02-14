#include <memory>
#include <Windows.h>

#include "output_window.hpp"
#include "xout.hpp"


int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int)
{
    xout::init(hInstance);

	for (int i = 0; i < 100; ++i) {
        xout::log("HONK WORLD, 0x{1:08X} {0}", "Wait this is index 0", 255);
        xout::info("HONK WORLD, 0x{1:08X} {0}", "Wait this is index 0", 255);
        xout::debug("HONK WORLD, 0x{1:08X} {0}", "Wait this is index 0", 255);
        xout::warning("HONK WORLD, 0x{1:08X} {0}", "Wait this is index 0", 255);
        xout::error("HONK WORLD, 0x{1:08X} {0}", "Wait this is index 0", 255);
        xout::critical("HONK WORLD, 0x{1:08X} {0}", "Wait this is index 0", 255);

        Sleep(3000);
        if (xout::should_exit()) { break; }
	}

}
