// ==WindhawkMod==
// @id              acrylic-effect-radius-changer
// @name            Acrylic Effect Radius Changer
// @description     Allows the user to change the Acrylic effect blur radius
// @version         1.0.0
// @author          Dulappy
// @github          https://github.com/Dulappy
// @include         dwm.exe
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Acrylic blur radius changer
By default, the Acrylic effect comes with a blur radius of 30, both horizontally and vertically, while not allowing developers or users to modify it. This mod allows the user to modify the default blur radius across **all** apps that utilize the Acrylic effect, by injecting into DWM and hooking all functions necessary to set the aforementioned blur radius.

## ⚠ Important usage note ⚠

In order to use this mod, you must enable Windhawk to inject into system processes in its advanced settings.
If you do not do this, it will silently fail to inject. **Changing the Windhawk advanced settings will also
affect any other mod you have installed, and may cause instability as any other mod that injects into all
processes will now inject into system processes too.**

This mod will not work on portable versions of Windhawk because DWM is a protected process and can only be
modified by a system account. Since the portable version of Windhawk only runs as administrator under your
own user account, it will not have the privilege required to inject into DWM.

## Examples
You can scroll down to see some examples of different custom blur radii in action:

![Default radius](https://i.imgur.com/AZM5rzq.png) \
*Default blur radius (30x30)*


![Custom radius lite](https://i.imgur.com/kDj5G4l.png) \
*Custom blur radius (4x4)*


![Custom radius customized](https://i.imgur.com/GA1meqE.png) \
*Custom blur radius (24x2)*

*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- radius:
  - width: 30
  - height: 30
  $name: Blur radius
  $description: Set the radius of the Acrylic blur effect to any number
*/
// ==/WindhawkModSettings==

#include <d2d1.h>
#include <d2d1effects.h>
#include <dcommon.h>

struct {
    int width;
    int height;
} settings;

void LoadSettings() {
    settings.width = Wh_GetIntSetting(L"radius.width");
    settings.height = Wh_GetIntSetting(L"radius.height");
}

long (*CCustomBlur_BuildEffect_orig)(void* pThis, ID2D1Image* image, D2D_RECT_F*, D2D_SIZE_F*, DWORD, D2D_VECTOR_2F*, D2D_VECTOR_2F*);

long CCustomBlur_BuildEffect_Hook(void* pThis, ID2D1Image* image, D2D_RECT_F* blurrect, D2D_SIZE_F* blurradius, DWORD optimization, D2D_VECTOR_2F* vector1, D2D_VECTOR_2F* vector2) {
    blurradius->width = (float)settings.width;
    blurradius->height = (float)settings.height;
    Wh_Log(L"%i", settings.width);
    return CCustomBlur_BuildEffect_orig(pThis, image, blurrect, blurradius, optimization, vector1, vector2);
}

float (*CCustomBlur_DetermineOutputScale_orig)(float, float, DWORD);

float CCustomBlur_DetermineOutputScale_Hook(float f1, float f2, DWORD optimization) {
    f1 = 0; // Makes the effect slightly less optimized, but makes it look significantly better in some cases
    f2 = 0; // In a future update, I might try to reimplement the lost optimization.

    return CCustomBlur_DetermineOutputScale_orig(f1, f2, optimization);
}

// The mod is being initialized, load settings, hook functions, and do other
// initialization stuff if required.
BOOL Wh_ModInit() {	
    HMODULE dwmcore = GetModuleHandle(L"dwmcore.dll");
    if (!dwmcore) return FALSE;

    WH_FIND_SYMBOL findSymbol;
    HANDLE findSymbolHandle = Wh_FindFirstSymbol(dwmcore, nullptr, &findSymbol);
    if (!findSymbolHandle) {
        Wh_Log(L"Wh_FindFirstSymbol failed");
        return FALSE;
    }
    
    void* DetermineOutputScaleAddr = nullptr;
    void* BuildEffectAddr = nullptr;

    do {
        if (_wcsicmp(findSymbol.symbol, L"public: static float __cdecl CCustomBlur::DetermineOutputScale(float,float,enum D2D1_GAUSSIANBLUR_OPTIMIZATION)") == 0) {
            DetermineOutputScaleAddr = findSymbol.address;
            Wh_Log(L"symbol: %s, Addr: %i", findSymbol.symbol, findSymbol.address);
        }
        else if (_wcsicmp(findSymbol.symbol, L"public: long __cdecl CCustomBlur::BuildEffect(struct ID2D1Image *,struct D2D_RECT_F const &,struct D2D_SIZE_F const &,enum D2D1_GAUSSIANBLUR_OPTIMIZATION,struct D2D_VECTOR_2F const &,struct D2D_VECTOR_2F *)") == 0) {
            BuildEffectAddr = findSymbol.address;
            Wh_Log(L"symbol: %s, Addr: %i", findSymbol.symbol, findSymbol.address);
        }
        /*else {
            Wh_Log(L"symbol: %s", findSymbol.symbol);
        }*/
    } while (Wh_FindNextSymbol(findSymbolHandle, &findSymbol));

    Wh_SetFunctionHook(DetermineOutputScaleAddr, (void*)CCustomBlur_DetermineOutputScale_Hook, (void**)&CCustomBlur_DetermineOutputScale_orig);

    Wh_SetFunctionHook(BuildEffectAddr, (void*)CCustomBlur_BuildEffect_Hook, (void**)&CCustomBlur_BuildEffect_orig);

    Wh_FindCloseSymbol(findSymbolHandle);

    LoadSettings();

    return TRUE;
}

// The mod is being unloaded, free all allocated resources.
void Wh_ModUninit() {
    Wh_Log(L"Uninit");
}

// The mod setting were changed, reload them.
void Wh_ModSettingsChanged() {
    Wh_Log(L"SettingsChanged");

    LoadSettings();
}
