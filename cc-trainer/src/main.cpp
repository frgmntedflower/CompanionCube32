#include <cstdio>
#include <windows.h>
#include <d3d9.h>
#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx9.h"
#include "MinHook.h"

HWND g_hWnd = nullptr;
LPDIRECT3DDEVICE9 g_pDevice = nullptr;
bool g_showMenu = true;
bool g_running = true;

typedef HRESULT(__stdcall* tEndScene)(LPDIRECT3DDEVICE9 pDevice);
tEndScene oEndScene = nullptr;

int createLogFile() {
    FILE* file;
    errno_t err = fopen_s(&file, R"(C:\Users\aki\Desktop\CompanionCube32\cc_trainer_log.txt)", "w");
    if (err != 0) {
        MessageBox(nullptr, "Failed to create log file!", "Error", MB_ICONERROR);
        return 1;
    }
    fprintf(file, "CC-Trainer log initialized.\n");
    MessageBox(nullptr, "Log file created!", "Created", MB_ICONINFORMATION);
    fclose(file);
    return 0;
}

void writeLog(const char* message) {
    FILE* file;
    fopen_s(&file, "cc_trainer_log.txt", "a");
    if (file) {
        fprintf(file, "%s\n", message);
        fclose(file);
    }
}

HRESULT __stdcall hkEndScene(LPDIRECT3DDEVICE9 pDevice) {
    MessageBox(g_hWnd, "Hello Hook!", "Hello Hook!", 0);
    if (!g_pDevice) g_pDevice = pDevice;

    if (!g_hWnd) {
        g_hWnd = GetForegroundWindow();
        ImGui::CreateContext();
        ImGui_ImplWin32_Init(g_hWnd);
        ImGui_ImplDX9_Init(g_pDevice);
    }

    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    if (g_showMenu) {
        ImGui::Begin("CC-Trainer", &g_showMenu);
        ImGui::Text("Hello from CC-Trainer!");
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

    return oEndScene(pDevice);
}

typedef HRESULT(__stdcall* tCreateDevice)(
    IDirect3D9*,
    UINT, D3DDEVTYPE, HWND,
    DWORD, D3DPRESENT_PARAMETERS*,
    LPDIRECT3DDEVICE9*);
tCreateDevice oCreateDevice = nullptr;

HRESULT __stdcall hkCreateDevice(
    IDirect3D9* pThis, UINT Adapter,
    D3DDEVTYPE DeviceType, HWND hFocusWindow,
    DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS* pPresentationParameters,
    LPDIRECT3DDEVICE9* ppReturnedDeviceInterface)
{
    HRESULT hr = oCreateDevice(pThis, Adapter, DeviceType, hFocusWindow,
                               BehaviorFlags, pPresentationParameters,
                               ppReturnedDeviceInterface);

    if (SUCCEEDED(hr) && ppReturnedDeviceInterface && !g_pDevice) {
        g_pDevice = *ppReturnedDeviceInterface;

        uintptr_t* vtable = *reinterpret_cast<uintptr_t**>(g_pDevice);
        uintptr_t endSceneAddr = vtable[42];

        MH_CreateHook(reinterpret_cast<LPVOID>(endSceneAddr),
                      reinterpret_cast<LPVOID>(hkEndScene),
                      reinterpret_cast<LPVOID*>(&oEndScene));
        MH_EnableHook(reinterpret_cast<LPVOID>(endSceneAddr));
    }

    return hr;
}

DWORD WINAPI MainThread(LPVOID lpParam)
{
    createLogFile();
    if (MH_Initialize() != MH_OK)
    {
        writeLog("Failed to initialize MinHook.");
        return 1;
    };

    // Hook IDirect3D9::CreateDevice
    HMODULE d3d9 = GetModuleHandleA("d3d9.dll");
    if (!d3d9)
    {
        writeLog("Failed to get d3d9.dll handle.");
        return 1;
    }

    uintptr_t* pVTable = *reinterpret_cast<uintptr_t**>(Direct3DCreate9(D3D_SDK_VERSION));
    MH_CreateHook(
        reinterpret_cast<LPVOID>(pVTable[16]),
        reinterpret_cast<LPVOID>(hkCreateDevice),
        reinterpret_cast<LPVOID*>(&oCreateDevice)
        );
    MH_EnableHook(MH_ALL_HOOKS);
    writeLog("Hooks initialized successfully.");

    while (g_running) Sleep(100);

    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();

    if (g_pDevice) {
        writeLog("Releasing Direct3D device.");
        ImGui_ImplDX9_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }


    writeLog("Exiting MainThread.");
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, MainThread, hModule, 0, nullptr);
    }
    else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
        g_running = false;
    }

    return TRUE;
}
