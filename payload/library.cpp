#include <windows.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {

    MessageBoxA(NULL, "Hello there!", "WIS Project", MB_OK);

    return TRUE;
}