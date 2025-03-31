#ifndef UTILITY_H
#define UTILITY_H

#include <wtypes.h>
#include <windows.h>
#include <sstream>

// Inline variables introduced in C++17 to handle definitions directly in header files
inline int SCREEN_WIDTH;
inline int SCREEN_HEIGHT;

inline void GetActualResolution(int& horizontal, int& vertical) {
    DEVMODE devMode;
    ZeroMemory(&devMode, sizeof(DEVMODE));
    devMode.dmSize = sizeof(DEVMODE);

    if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devMode)) {
        horizontal = devMode.dmPelsWidth;
        vertical = devMode.dmPelsHeight;
    }
    else {
        // Fallback if the EnumDisplaySettings fails
        horizontal = GetSystemMetrics(SM_CXSCREEN);
        vertical = GetSystemMetrics(SM_CYSCREEN);
    }
}

inline void InitializeResolution() {
    GetActualResolution(SCREEN_WIDTH, SCREEN_HEIGHT);
}


inline void Output(const char* szFormat, ...)
{
    char szBuff[1024];
    va_list arg;
    va_start(arg, szFormat);

    // Use _vsnprintf_s for secure formatting
    _vsnprintf_s(szBuff, sizeof(szBuff), _TRUNCATE, szFormat, arg);

    va_end(arg);

    OutputDebugStringA(szBuff);
}
#endif // UTILITY_H
