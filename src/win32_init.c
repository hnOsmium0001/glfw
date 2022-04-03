//========================================================================
// GLFW 3.4 Win32 - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2019 Camilla Löwy <elmindreda@glfw.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================
// Please use C89 style variable declarations in this file because VS 2010
//========================================================================

#include "internal.h"

#include <stdlib.h>

static const GUID _glfw_GUID_DEVINTERFACE_HID =
    {0x4d1e55b2,0xf16f,0x11cf,{0x88,0xcb,0x00,0x11,0x11,0x00,0x00,0x30}};

#define GUID_DEVINTERFACE_HID _glfw_GUID_DEVINTERFACE_HID

#if defined(_GLFW_USE_HYBRID_HPG) || defined(_GLFW_USE_OPTIMUS_HPG)

#if defined(_GLFW_BUILD_DLL)
 #pragma message("These symbols must be exported by the executable and have no effect in a DLL")
#endif

// Executables (but not DLLs) exporting this symbol with this value will be
// automatically directed to the high-performance GPU on Nvidia Optimus systems
// with up-to-date drivers
//
__declspec(dllexport) DWORD NvOptimusEnablement = 1;

// Executables (but not DLLs) exporting this symbol with this value will be
// automatically directed to the high-performance GPU on AMD PowerXpress systems
// with up-to-date drivers
//
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;

#endif // _GLFW_USE_HYBRID_HPG

#if defined(_GLFW_BUILD_DLL)

// GLFW DLL entry point
//
BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
    return TRUE;
}

#endif // _GLFW_BUILD_DLL

// Load necessary libraries (DLLs)
//
static GLFWbool loadLibraries(void)
{
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                                GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                            (const WCHAR*) &_glfw,
                            (HMODULE*) &_glfw.win32.instance))
    {
        _glfwInputErrorWin32(GLFW_PLATFORM_ERROR,
                             "Win32: Failed to retrieve own module handle");
        return GLFW_FALSE;
    }

    _glfw.win32.user32.instance = _glfwPlatformLoadModule("user32.dll");
    if (!_glfw.win32.user32.instance)
    {
        _glfwInputErrorWin32(GLFW_PLATFORM_ERROR,
                             "Win32: Failed to load user32.dll");
        return GLFW_FALSE;
    }

    _glfw.win32.user32.SetProcessDPIAware_ = (PFN_SetProcessDPIAware)
        _glfwPlatformGetModuleSymbol(_glfw.win32.user32.instance, "SetProcessDPIAware");
    _glfw.win32.user32.ChangeWindowMessageFilterEx_ = (PFN_ChangeWindowMessageFilterEx)
        _glfwPlatformGetModuleSymbol(_glfw.win32.user32.instance, "ChangeWindowMessageFilterEx");
    _glfw.win32.user32.EnableNonClientDpiScaling_ = (PFN_EnableNonClientDpiScaling)
        _glfwPlatformGetModuleSymbol(_glfw.win32.user32.instance, "EnableNonClientDpiScaling");
    _glfw.win32.user32.SetProcessDpiAwarenessContext_ = (PFN_SetProcessDpiAwarenessContext)
        _glfwPlatformGetModuleSymbol(_glfw.win32.user32.instance, "SetProcessDpiAwarenessContext");
    _glfw.win32.user32.GetDpiForWindow_ = (PFN_GetDpiForWindow)
        _glfwPlatformGetModuleSymbol(_glfw.win32.user32.instance, "GetDpiForWindow");
    _glfw.win32.user32.AdjustWindowRectExForDpi_ = (PFN_AdjustWindowRectExForDpi)
        _glfwPlatformGetModuleSymbol(_glfw.win32.user32.instance, "AdjustWindowRectExForDpi");
    _glfw.win32.user32.GetSystemMetricsForDpi_ = (PFN_GetSystemMetricsForDpi)
        _glfwPlatformGetModuleSymbol(_glfw.win32.user32.instance, "GetSystemMetricsForDpi");

    _glfw.win32.dinput8.instance = _glfwPlatformLoadModule("dinput8.dll");
    if (_glfw.win32.dinput8.instance)
    {
        _glfw.win32.dinput8.Create = (PFN_DirectInput8Create)
            _glfwPlatformGetModuleSymbol(_glfw.win32.dinput8.instance, "DirectInput8Create");
    }

    {
        int i;
        const char* names[] =
        {
            "xinput1_4.dll",
            "xinput1_3.dll",
            "xinput9_1_0.dll",
            "xinput1_2.dll",
            "xinput1_1.dll",
            NULL
        };

        for (i = 0;  names[i];  i++)
        {
            _glfw.win32.xinput.instance = _glfwPlatformLoadModule(names[i]);
            if (_glfw.win32.xinput.instance)
            {
                _glfw.win32.xinput.GetCapabilities = (PFN_XInputGetCapabilities)
                    _glfwPlatformGetModuleSymbol(_glfw.win32.xinput.instance, "XInputGetCapabilities");
                _glfw.win32.xinput.GetState = (PFN_XInputGetState)
                    _glfwPlatformGetModuleSymbol(_glfw.win32.xinput.instance, "XInputGetState");

                break;
            }
        }
    }

    _glfw.win32.dwmapi.instance = _glfwPlatformLoadModule("dwmapi.dll");
    if (_glfw.win32.dwmapi.instance)
    {
        _glfw.win32.dwmapi.IsCompositionEnabled = (PFN_DwmIsCompositionEnabled)
            _glfwPlatformGetModuleSymbol(_glfw.win32.dwmapi.instance, "DwmIsCompositionEnabled");
        _glfw.win32.dwmapi.Flush = (PFN_DwmFlush)
            _glfwPlatformGetModuleSymbol(_glfw.win32.dwmapi.instance, "DwmFlush");
        _glfw.win32.dwmapi.EnableBlurBehindWindow = (PFN_DwmEnableBlurBehindWindow)
            _glfwPlatformGetModuleSymbol(_glfw.win32.dwmapi.instance, "DwmEnableBlurBehindWindow");
        _glfw.win32.dwmapi.GetColorizationColor = (PFN_DwmGetColorizationColor)
            _glfwPlatformGetModuleSymbol(_glfw.win32.dwmapi.instance, "DwmGetColorizationColor");
    }

    _glfw.win32.shcore.instance = _glfwPlatformLoadModule("shcore.dll");
    if (_glfw.win32.shcore.instance)
    {
        _glfw.win32.shcore.SetProcessDpiAwareness_ = (PFN_SetProcessDpiAwareness)
            _glfwPlatformGetModuleSymbol(_glfw.win32.shcore.instance, "SetProcessDpiAwareness");
        _glfw.win32.shcore.GetDpiForMonitor_ = (PFN_GetDpiForMonitor)
            _glfwPlatformGetModuleSymbol(_glfw.win32.shcore.instance, "GetDpiForMonitor");
    }

    _glfw.win32.ntdll.instance = _glfwPlatformLoadModule("ntdll.dll");
    if (_glfw.win32.ntdll.instance)
    {
        _glfw.win32.ntdll.RtlVerifyVersionInfo_ = (PFN_RtlVerifyVersionInfo)
            _glfwPlatformGetModuleSymbol(_glfw.win32.ntdll.instance, "RtlVerifyVersionInfo");
    }

    return GLFW_TRUE;
}

// Unload used libraries (DLLs)
//
static void freeLibraries(void)
{
    if (_glfw.win32.xinput.instance)
        _glfwPlatformFreeModule(_glfw.win32.xinput.instance);

    if (_glfw.win32.dinput8.instance)
        _glfwPlatformFreeModule(_glfw.win32.dinput8.instance);

    if (_glfw.win32.user32.instance)
        _glfwPlatformFreeModule(_glfw.win32.user32.instance);

    if (_glfw.win32.dwmapi.instance)
        _glfwPlatformFreeModule(_glfw.win32.dwmapi.instance);

    if (_glfw.win32.shcore.instance)
        _glfwPlatformFreeModule(_glfw.win32.shcore.instance);

    if (_glfw.win32.ntdll.instance)
        _glfwPlatformFreeModule(_glfw.win32.ntdll.instance);
}

// Create key code translation tables
//
static void createKeyTables(void)
{
    int scancode;

    memset(_glfw.win32.keycodes, -1, sizeof(_glfw.win32.keycodes));
    memset(_glfw.win32.scancodes, -1, sizeof(_glfw.win32.scancodes));

    // See https://download.microsoft.com/download/1/6/1/161ba512-40e2-4cc9-843a-923143f3456c/translate.pdf
    // PS/2 set 1 make
#pragma push_macro("INDEX")
#undef INDEX
#define INDEX(makeCode, e0, e1) GLFW_WIN32_CALC_KEYMAP(makeCode, e0, e1)

    short int* keycodes = _glfw.win32.keycodes;
    
    //            E0 bit-v  v-E1 bit
    keycodes[INDEX(0x0B, 0, 0)] = GLFW_KEY_0;
    keycodes[INDEX(0x02, 0, 0)] = GLFW_KEY_1;
    keycodes[INDEX(0x03, 0, 0)] = GLFW_KEY_2;
    keycodes[INDEX(0x04, 0, 0)] = GLFW_KEY_3;
    keycodes[INDEX(0x05, 0, 0)] = GLFW_KEY_4;
    keycodes[INDEX(0x06, 0, 0)] = GLFW_KEY_5;
    keycodes[INDEX(0x07, 0, 0)] = GLFW_KEY_6;
    keycodes[INDEX(0x08, 0, 0)] = GLFW_KEY_7;
    keycodes[INDEX(0x09, 0, 0)] = GLFW_KEY_8;
    keycodes[INDEX(0x0A, 0, 0)] = GLFW_KEY_9;
    keycodes[INDEX(0x1E, 0, 0)] = GLFW_KEY_A;
    keycodes[INDEX(0x30, 0, 0)] = GLFW_KEY_B;
    keycodes[INDEX(0x2E, 0, 0)] = GLFW_KEY_C;
    keycodes[INDEX(0x20, 0, 0)] = GLFW_KEY_D;
    keycodes[INDEX(0x12, 0, 0)] = GLFW_KEY_E;
    keycodes[INDEX(0x21, 0, 0)] = GLFW_KEY_F;
    keycodes[INDEX(0x22, 0, 0)] = GLFW_KEY_G;
    keycodes[INDEX(0x23, 0, 0)] = GLFW_KEY_H;
    keycodes[INDEX(0x17, 0, 0)] = GLFW_KEY_I;
    keycodes[INDEX(0x24, 0, 0)] = GLFW_KEY_J;
    keycodes[INDEX(0x25, 0, 0)] = GLFW_KEY_K;
    keycodes[INDEX(0x26, 0, 0)] = GLFW_KEY_L;
    keycodes[INDEX(0x32, 0, 0)] = GLFW_KEY_M;
    keycodes[INDEX(0x31, 0, 0)] = GLFW_KEY_N;
    keycodes[INDEX(0x18, 0, 0)] = GLFW_KEY_O;
    keycodes[INDEX(0x19, 0, 0)] = GLFW_KEY_P;
    keycodes[INDEX(0x10, 0, 0)] = GLFW_KEY_Q;
    keycodes[INDEX(0x13, 0, 0)] = GLFW_KEY_R;
    keycodes[INDEX(0x1F, 0, 0)] = GLFW_KEY_S;
    keycodes[INDEX(0x14, 0, 0)] = GLFW_KEY_T;
    keycodes[INDEX(0x16, 0, 0)] = GLFW_KEY_U;
    keycodes[INDEX(0x2F, 0, 0)] = GLFW_KEY_V;
    keycodes[INDEX(0x11, 0, 0)] = GLFW_KEY_W;
    keycodes[INDEX(0x2D, 0, 0)] = GLFW_KEY_X;
    keycodes[INDEX(0x15, 0, 0)] = GLFW_KEY_Y;
    keycodes[INDEX(0x2C, 0, 0)] = GLFW_KEY_Z;

    keycodes[INDEX(0x28, 0, 0)] = GLFW_KEY_APOSTROPHE;
    keycodes[INDEX(0x2B, 0, 0)] = GLFW_KEY_BACKSLASH;
    keycodes[INDEX(0x33, 0, 0)] = GLFW_KEY_COMMA;
    keycodes[INDEX(0x0D, 0, 0)] = GLFW_KEY_EQUAL;
    keycodes[INDEX(0x29, 0, 0)] = GLFW_KEY_GRAVE_ACCENT;
    keycodes[INDEX(0x1A, 0, 0)] = GLFW_KEY_LEFT_BRACKET;
    keycodes[INDEX(0x0C, 0, 0)] = GLFW_KEY_MINUS;
    keycodes[INDEX(0x34, 0, 0)] = GLFW_KEY_PERIOD;
    keycodes[INDEX(0x1B, 0, 0)] = GLFW_KEY_RIGHT_BRACKET;
    keycodes[INDEX(0x27, 0, 0)] = GLFW_KEY_SEMICOLON;
    keycodes[INDEX(0x35, 0, 0)] = GLFW_KEY_SLASH;
    keycodes[INDEX(0x56, 0, 0)] = GLFW_KEY_WORLD_2;

    keycodes[INDEX(0x0E, 0, 0)] = GLFW_KEY_BACKSPACE;
    keycodes[INDEX(0x53, 1, 0)] = GLFW_KEY_DELETE;
    keycodes[INDEX(0x4F, 1, 0)] = GLFW_KEY_END;
    keycodes[INDEX(0x1C, 0, 0)] = GLFW_KEY_ENTER;
    keycodes[INDEX(0x01, 0, 0)] = GLFW_KEY_ESCAPE;
    keycodes[INDEX(0x47, 1, 0)] = GLFW_KEY_HOME;
    keycodes[INDEX(0x52, 1, 0)] = GLFW_KEY_INSERT;
    keycodes[INDEX(0x5D, 1, 0)] = GLFW_KEY_MENU;
    keycodes[INDEX(0x51, 1, 0)] = GLFW_KEY_PAGE_DOWN;
    keycodes[INDEX(0x49, 1, 0)] = GLFW_KEY_PAGE_UP;
    keycodes[INDEX(0x46, 1, 0)] = GLFW_KEY_PAUSE; // Ctrl+Pause
    keycodes[INDEX(0x1D, 0, 1)] = GLFW_KEY_PAUSE; // Pause
    keycodes[INDEX(0x39, 0, 0)] = GLFW_KEY_SPACE;
    keycodes[INDEX(0x0F, 0, 0)] = GLFW_KEY_TAB;
    keycodes[INDEX(0x3A, 0, 0)] = GLFW_KEY_CAPS_LOCK;
    keycodes[INDEX(0x45, 1, 0)] = GLFW_KEY_NUM_LOCK;
    keycodes[INDEX(0x46, 0, 0)] = GLFW_KEY_SCROLL_LOCK;
    keycodes[INDEX(0x3B, 0, 0)] = GLFW_KEY_F1;
    keycodes[INDEX(0x3C, 0, 0)] = GLFW_KEY_F2;
    keycodes[INDEX(0x3D, 0, 0)] = GLFW_KEY_F3;
    keycodes[INDEX(0x3E, 0, 0)] = GLFW_KEY_F4;
    keycodes[INDEX(0x3F, 0, 0)] = GLFW_KEY_F5;
    keycodes[INDEX(0x40, 0, 0)] = GLFW_KEY_F6;
    keycodes[INDEX(0x41, 0, 0)] = GLFW_KEY_F7;
    keycodes[INDEX(0x42, 0, 0)] = GLFW_KEY_F8;
    keycodes[INDEX(0x43, 0, 0)] = GLFW_KEY_F9;
    keycodes[INDEX(0x44, 0, 0)] = GLFW_KEY_F10;
    keycodes[INDEX(0x57, 0, 0)] = GLFW_KEY_F11;
    keycodes[INDEX(0x58, 0, 0)] = GLFW_KEY_F12;
    keycodes[INDEX(0x64, 0, 0)] = GLFW_KEY_F13;
    keycodes[INDEX(0x65, 0, 0)] = GLFW_KEY_F14;
    keycodes[INDEX(0x66, 0, 0)] = GLFW_KEY_F15;
    keycodes[INDEX(0x67, 0, 0)] = GLFW_KEY_F16;
    keycodes[INDEX(0x68, 0, 0)] = GLFW_KEY_F17;
    keycodes[INDEX(0x69, 0, 0)] = GLFW_KEY_F18;
    keycodes[INDEX(0x6A, 0, 0)] = GLFW_KEY_F19;
    keycodes[INDEX(0x6B, 0, 0)] = GLFW_KEY_F20;
    keycodes[INDEX(0x6C, 0, 0)] = GLFW_KEY_F21;
    keycodes[INDEX(0x6D, 0, 0)] = GLFW_KEY_F22;
    keycodes[INDEX(0x6E, 0, 0)] = GLFW_KEY_F23;
    keycodes[INDEX(0x76, 0, 0)] = GLFW_KEY_F24;
    keycodes[INDEX(0x38, 0, 0)] = GLFW_KEY_LEFT_ALT;
    keycodes[INDEX(0x1D, 0, 0)] = GLFW_KEY_LEFT_CONTROL;
    keycodes[INDEX(0x2A, 0, 0)] = GLFW_KEY_LEFT_SHIFT;
    keycodes[INDEX(0x5B, 1, 0)] = GLFW_KEY_LEFT_SUPER;
    keycodes[INDEX(0x37, 1, 0)] = GLFW_KEY_PRINT_SCREEN;
    keycodes[INDEX(0x38, 1, 0)] = GLFW_KEY_RIGHT_ALT;
    keycodes[INDEX(0x1D, 1, 0)] = GLFW_KEY_RIGHT_CONTROL;
    keycodes[INDEX(0x36, 0, 0)] = GLFW_KEY_RIGHT_SHIFT;
    keycodes[INDEX(0x5C, 1, 0)] = GLFW_KEY_RIGHT_SUPER;
    keycodes[INDEX(0x50, 1, 0)] = GLFW_KEY_DOWN;
    keycodes[INDEX(0x4B, 1, 0)] = GLFW_KEY_LEFT;
    keycodes[INDEX(0x4D, 1, 0)] = GLFW_KEY_RIGHT;
    keycodes[INDEX(0x48, 1, 0)] = GLFW_KEY_UP;

    keycodes[INDEX(0x52, 0, 0)] = GLFW_KEY_KP_0;
    keycodes[INDEX(0x4F, 0, 0)] = GLFW_KEY_KP_1;
    keycodes[INDEX(0x50, 0, 0)] = GLFW_KEY_KP_2;
    keycodes[INDEX(0x51, 0, 0)] = GLFW_KEY_KP_3;
    keycodes[INDEX(0x4B, 0, 0)] = GLFW_KEY_KP_4;
    keycodes[INDEX(0x4C, 0, 0)] = GLFW_KEY_KP_5;
    keycodes[INDEX(0x4D, 0, 0)] = GLFW_KEY_KP_6;
    keycodes[INDEX(0x47, 0, 0)] = GLFW_KEY_KP_7;
    keycodes[INDEX(0x48, 0, 0)] = GLFW_KEY_KP_8;
    keycodes[INDEX(0x49, 0, 0)] = GLFW_KEY_KP_9;
    keycodes[INDEX(0x4E, 0, 0)] = GLFW_KEY_KP_ADD;
    keycodes[INDEX(0x53, 0, 0)] = GLFW_KEY_KP_DECIMAL;
    keycodes[INDEX(0x35, 1, 0)] = GLFW_KEY_KP_DIVIDE;
    keycodes[INDEX(0x1C, 1, 0)] = GLFW_KEY_KP_ENTER;
    keycodes[INDEX(0x59, 0, 0)] = GLFW_KEY_KP_EQUAL;
    keycodes[INDEX(0x37, 0, 0)] = GLFW_KEY_KP_MULTIPLY;
    keycodes[INDEX(0x4A, 0, 0)] = GLFW_KEY_KP_SUBTRACT;
#pragma pop_macro("INDEX")

    for (scancode = 0;  scancode < (sizeof(_glfw.win32.keycodes) / sizeof(_glfw.win32.keycodes[0]));  scancode++)
    {
        int keycode = _glfw.win32.keycodes[scancode];
        if (keycode > 0)
        {
            int e0 = scancode & (1 << GLFW_WIN32_KEYMAP_E0_BIT);
            int e1 = scancode & (1 << GLFW_WIN32_KEYMAP_E1_BIT);
            // NOTE: operator || outputs 0 or 1 regardless the value of the operands
            GLFWbool extended = e0 || e1;
            // Old scancode format:
            // Xkkkkkkkk <- 8 bit index
            // ^-- Extended bit
            int compatScancode = (scancode & 0xFF) | (extended & 0x01) << 8;

            _glfw.win32.scancodes[keycode] = compatScancode;
        }
    }
}

// Creates a dummy window for behind-the-scenes work
//
static GLFWbool createHelperWindow(void)
{
    MSG msg;

    _glfw.win32.helperWindowHandle =
        CreateWindowExW(WS_EX_OVERLAPPEDWINDOW,
                        _GLFW_WNDCLASSNAME,
                        L"GLFW message window",
                        WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                        0, 0, 1, 1,
                        NULL, NULL,
                        _glfw.win32.instance,
                        NULL);

    if (!_glfw.win32.helperWindowHandle)
    {
        _glfwInputErrorWin32(GLFW_PLATFORM_ERROR,
                             "Win32: Failed to create helper window");
        return GLFW_FALSE;
    }

    // HACK: The command to the first ShowWindow call is ignored if the parent
    //       process passed along a STARTUPINFO, so clear that with a no-op call
    ShowWindow(_glfw.win32.helperWindowHandle, SW_HIDE);

    // Register for HID device notifications
    {
        DEV_BROADCAST_DEVICEINTERFACE_W dbi;
        ZeroMemory(&dbi, sizeof(dbi));
        dbi.dbcc_size = sizeof(dbi);
        dbi.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
        dbi.dbcc_classguid = GUID_DEVINTERFACE_HID;

        _glfw.win32.deviceNotificationHandle =
            RegisterDeviceNotificationW(_glfw.win32.helperWindowHandle,
                                        (DEV_BROADCAST_HDR*) &dbi,
                                        DEVICE_NOTIFY_WINDOW_HANDLE);
    }

    while (PeekMessageW(&msg, _glfw.win32.helperWindowHandle, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

   return GLFW_TRUE;
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Returns a wide string version of the specified UTF-8 string
//
WCHAR* _glfwCreateWideStringFromUTF8Win32(const char* source)
{
    WCHAR* target;
    int count;

    count = MultiByteToWideChar(CP_UTF8, 0, source, -1, NULL, 0);
    if (!count)
    {
        _glfwInputErrorWin32(GLFW_PLATFORM_ERROR,
                             "Win32: Failed to convert string from UTF-8");
        return NULL;
    }

    target = _glfw_calloc(count, sizeof(WCHAR));

    if (!MultiByteToWideChar(CP_UTF8, 0, source, -1, target, count))
    {
        _glfwInputErrorWin32(GLFW_PLATFORM_ERROR,
                             "Win32: Failed to convert string from UTF-8");
        _glfw_free(target);
        return NULL;
    }

    return target;
}

// Returns a UTF-8 string version of the specified wide string
//
char* _glfwCreateUTF8FromWideStringWin32(const WCHAR* source)
{
    char* target;
    int size;

    size = WideCharToMultiByte(CP_UTF8, 0, source, -1, NULL, 0, NULL, NULL);
    if (!size)
    {
        _glfwInputErrorWin32(GLFW_PLATFORM_ERROR,
                             "Win32: Failed to convert string to UTF-8");
        return NULL;
    }

    target = _glfw_calloc(size, 1);

    if (!WideCharToMultiByte(CP_UTF8, 0, source, -1, target, size, NULL, NULL))
    {
        _glfwInputErrorWin32(GLFW_PLATFORM_ERROR,
                             "Win32: Failed to convert string to UTF-8");
        _glfw_free(target);
        return NULL;
    }

    return target;
}

// Reports the specified error, appending information about the last Win32 error
//
void _glfwInputErrorWin32(int error, const char* description)
{
    WCHAR buffer[_GLFW_MESSAGE_SIZE] = L"";
    char message[_GLFW_MESSAGE_SIZE] = "";

    FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM |
                       FORMAT_MESSAGE_IGNORE_INSERTS |
                       FORMAT_MESSAGE_MAX_WIDTH_MASK,
                   NULL,
                   GetLastError() & 0xffff,
                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                   buffer,
                   sizeof(buffer) / sizeof(WCHAR),
                   NULL);
    WideCharToMultiByte(CP_UTF8, 0, buffer, -1, message, sizeof(message), NULL, NULL);

    _glfwInputError(error, "%s: %s", description, message);
}

// Updates key names according to the current keyboard layout
//
void _glfwUpdateKeyNamesWin32(void)
{
    int key;
    BYTE state[256] = {0};

    memset(_glfw.win32.keynames, 0, sizeof(_glfw.win32.keynames));

    for (key = GLFW_KEY_SPACE;  key <= GLFW_KEY_LAST;  key++)
    {
        UINT vk;
        int scancode, length;
        WCHAR chars[16];

        scancode = _glfw.win32.scancodes[key];
        if (scancode == -1)
            continue;

        if (key >= GLFW_KEY_KP_0 && key <= GLFW_KEY_KP_ADD)
        {
            const UINT vks[] = {
                VK_NUMPAD0,  VK_NUMPAD1,  VK_NUMPAD2, VK_NUMPAD3,
                VK_NUMPAD4,  VK_NUMPAD5,  VK_NUMPAD6, VK_NUMPAD7,
                VK_NUMPAD8,  VK_NUMPAD9,  VK_DECIMAL, VK_DIVIDE,
                VK_MULTIPLY, VK_SUBTRACT, VK_ADD
            };

            vk = vks[key - GLFW_KEY_KP_0];
        }
        else
            vk = MapVirtualKeyW(scancode, MAPVK_VSC_TO_VK);

        length = ToUnicode(vk, scancode, state,
                           chars, sizeof(chars) / sizeof(WCHAR),
                           0);

        if (length == -1)
        {
            length = ToUnicode(vk, scancode, state,
                               chars, sizeof(chars) / sizeof(WCHAR),
                               0);
        }

        if (length < 1)
            continue;

        WideCharToMultiByte(CP_UTF8, 0, chars, 1,
                            _glfw.win32.keynames[key],
                            sizeof(_glfw.win32.keynames[key]),
                            NULL, NULL);
    }
}

// Replacement for IsWindowsVersionOrGreater, as we cannot rely on the
// application having a correct embedded manifest
//
BOOL _glfwIsWindowsVersionOrGreaterWin32(WORD major, WORD minor, WORD sp)
{
    OSVERSIONINFOEXW osvi = { sizeof(osvi), major, minor, 0, 0, {0}, sp };
    DWORD mask = VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR;
    ULONGLONG cond = VerSetConditionMask(0, VER_MAJORVERSION, VER_GREATER_EQUAL);
    cond = VerSetConditionMask(cond, VER_MINORVERSION, VER_GREATER_EQUAL);
    cond = VerSetConditionMask(cond, VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);
    // HACK: Use RtlVerifyVersionInfo instead of VerifyVersionInfoW as the
    //       latter lies unless the user knew to embed a non-default manifest
    //       announcing support for Windows 10 via supportedOS GUID
    return RtlVerifyVersionInfo(&osvi, mask, cond) == 0;
}

// Checks whether we are on at least the specified build of Windows 10
//
BOOL _glfwIsWindows10BuildOrGreaterWin32(WORD build)
{
    OSVERSIONINFOEXW osvi = { sizeof(osvi), 10, 0, build };
    DWORD mask = VER_MAJORVERSION | VER_MINORVERSION | VER_BUILDNUMBER;
    ULONGLONG cond = VerSetConditionMask(0, VER_MAJORVERSION, VER_GREATER_EQUAL);
    cond = VerSetConditionMask(cond, VER_MINORVERSION, VER_GREATER_EQUAL);
    cond = VerSetConditionMask(cond, VER_BUILDNUMBER, VER_GREATER_EQUAL);
    // HACK: Use RtlVerifyVersionInfo instead of VerifyVersionInfoW as the
    //       latter lies unless the user knew to embed a non-default manifest
    //       announcing support for Windows 10 via supportedOS GUID
    return RtlVerifyVersionInfo(&osvi, mask, cond) == 0;
}

GLFWbool _glfwConnectWin32(int platformID, _GLFWplatform* platform)
{
    const _GLFWplatform win32 =
    {
        GLFW_PLATFORM_WIN32,
        _glfwInitWin32,
        _glfwTerminateWin32,
        _glfwGetCursorPosWin32,
        _glfwSetCursorPosWin32,
        _glfwSetCursorModeWin32,
        _glfwSetRawMouseMotionWin32,
        _glfwRawMouseMotionSupportedWin32,
        _glfwKeyboardsSupportedWin32,
        _glfwCreateCursorWin32,
        _glfwCreateStandardCursorWin32,
        _glfwDestroyCursorWin32,
        _glfwSetCursorWin32,
        _glfwGetScancodeNameWin32,
        _glfwGetKeyScancodeWin32,
        _glfwSetClipboardStringWin32,
        _glfwGetClipboardStringWin32,
        _glfwInitJoysticksWin32,
        _glfwTerminateJoysticksWin32,
        _glfwPollJoystickWin32,
        _glfwGetMappingNameWin32,
        _glfwUpdateGamepadGUIDWin32,
        _glfwFreeMonitorWin32,
        _glfwGetMonitorPosWin32,
        _glfwGetMonitorContentScaleWin32,
        _glfwGetMonitorWorkareaWin32,
        _glfwGetVideoModesWin32,
        _glfwGetVideoModeWin32,
        _glfwGetGammaRampWin32,
        _glfwSetGammaRampWin32,
        _glfwCreateWindowWin32,
        _glfwDestroyWindowWin32,
        _glfwSetWindowTitleWin32,
        _glfwSetWindowIconWin32,
        _glfwGetWindowPosWin32,
        _glfwSetWindowPosWin32,
        _glfwGetWindowSizeWin32,
        _glfwSetWindowSizeWin32,
        _glfwSetWindowSizeLimitsWin32,
        _glfwSetWindowAspectRatioWin32,
        _glfwGetFramebufferSizeWin32,
        _glfwGetWindowFrameSizeWin32,
        _glfwGetWindowContentScaleWin32,
        _glfwIconifyWindowWin32,
        _glfwRestoreWindowWin32,
        _glfwMaximizeWindowWin32,
        _glfwShowWindowWin32,
        _glfwHideWindowWin32,
        _glfwRequestWindowAttentionWin32,
        _glfwFocusWindowWin32,
        _glfwSetWindowMonitorWin32,
        _glfwWindowFocusedWin32,
        _glfwWindowIconifiedWin32,
        _glfwWindowVisibleWin32,
        _glfwWindowMaximizedWin32,
        _glfwWindowHoveredWin32,
        _glfwFramebufferTransparentWin32,
        _glfwGetWindowOpacityWin32,
        _glfwSetWindowResizableWin32,
        _glfwSetWindowDecoratedWin32,
        _glfwSetWindowFloatingWin32,
        _glfwSetWindowOpacityWin32,
        _glfwSetWindowMousePassthroughWin32,
        _glfwPollEventsWin32,
        _glfwWaitEventsWin32,
        _glfwWaitEventsTimeoutWin32,
        _glfwPostEmptyEventWin32,
        _glfwGetEGLPlatformWin32,
        _glfwGetEGLNativeDisplayWin32,
        _glfwGetEGLNativeWindowWin32,
        _glfwGetRequiredInstanceExtensionsWin32,
        _glfwGetPhysicalDevicePresentationSupportWin32,
        _glfwCreateWindowSurfaceWin32,
    };

    *platform = win32;
    return GLFW_TRUE;
}

int _glfwInitWin32(void)
{
    if (!loadLibraries())
        return GLFW_FALSE;

    createKeyTables();
    _glfwUpdateKeyNamesWin32();

    if (_glfwIsWindows10Version1703OrGreaterWin32())
        SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    else if (IsWindows8Point1OrGreater())
        SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
    else if (IsWindowsVistaOrGreater())
        SetProcessDPIAware();

    if (!_glfwRegisterWindowClassWin32())
        return GLFW_FALSE;

    if (!createHelperWindow())
        return GLFW_FALSE;

    // Register that we want to receive WM_INPUT for keyboards
    {
        RAWINPUTDEVICE rid;
        rid.usUsagePage = 0x01;         // HID_USAGE_PAGE_GENERIC
        rid.usUsage = 0x06;             // HID_USAGE_GENERIC_KEYBOARD
        rid.dwFlags = 0;
        rid.hwndTarget = 0;             // Message delivered to focused window

        if (RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE)) == FALSE)
            return GLFW_FALSE;
    }

    _glfwPollMonitorsWin32();
    _glfwPollKeyboardsWin32();
    return GLFW_TRUE;
}

void _glfwTerminateWin32(void)
{
    if (_glfw.win32.deviceNotificationHandle)
        UnregisterDeviceNotification(_glfw.win32.deviceNotificationHandle);

    if (_glfw.win32.helperWindowHandle)
        DestroyWindow(_glfw.win32.helperWindowHandle);

    _glfwUnregisterWindowClassWin32();

    _glfw_free(_glfw.win32.clipboardString);
    _glfw_free(_glfw.win32.rawInput);

    _glfwTerminateWGL();
    _glfwTerminateEGL();

    freeLibraries();
}

