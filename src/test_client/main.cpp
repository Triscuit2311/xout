#include "xout_client.hpp"


int main()
{
    xout::broadcast();

    while (true)
    {
		xout::log("{1:<7} {0:>6} [ {2:+06.2f} {3:x} {4:#08b} {5:.3f} ]", "World!", "Hello,", 3.14159, 255, 5, 0.125);
		xout::info(L"مرحبا عالم!");
		xout::debug(L"Привіт, Світ!");
		xout::warning(L"{1:<7} {0:>6}", L"世界!", L"你好,");
		xout::error("{1:<7} {0:>6}", "An ERROR!", "AH,");

		std::wstring crit = L"CRITICAL ERROR! LIMIT BREAK!!";
		xout::critical(crit);

        Sleep(2000);
    }
}
