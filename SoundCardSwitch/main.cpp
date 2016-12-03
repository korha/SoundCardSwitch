//SoundCardSwitch
#define _WIN32_WINNT _WIN32_IE_WINBLUE
#include <mmdeviceapi.h>

static constexpr const wchar_t *const g_wGuidClass = L"App::15423203-ff9d-2e13-1788-e087bcd8dbcb";

//-------------------------------------------------------------------------------------------------
static inline bool FCompareMemoryW(const wchar_t *pBuf1, const wchar_t *pBuf2)
{
    while (*pBuf1 == *pBuf2 && *pBuf2)
        ++pBuf1, ++pBuf2;
    return *pBuf1 == *pBuf2;
}

static inline void FCopyMemory(unsigned char *pDst)
{
    const char *pSrc = "\x80\x20\x67\xD1\x46\xA8\x50\xE0";
    DWORD dwSize = 8;
    while (dwSize--)
        *pDst++ = *pSrc++;
}

static inline void PropVariantInitFix(void *const pDst__)
{
    BYTE *pDst = static_cast<BYTE*>(pDst__);
    DWORD dwSize = sizeof(PROPVARIANT);
    while (dwSize--)
        *pDst++ = '\0';
}

//-------------------------------------------------------------------------------------------------
class IPolicyConfig : public IUnknown
{
public:
    virtual HRESULT GetMixFormat(PCWSTR, WAVEFORMATEX **);
    virtual HRESULT STDMETHODCALLTYPE GetDeviceFormat(PCWSTR, INT, WAVEFORMATEX **);
    virtual HRESULT STDMETHODCALLTYPE ResetDeviceFormat(PCWSTR);
    virtual HRESULT STDMETHODCALLTYPE SetDeviceFormat(PCWSTR, WAVEFORMATEX *, WAVEFORMATEX *);
    virtual HRESULT STDMETHODCALLTYPE GetProcessingPeriod(PCWSTR, INT, PINT64, PINT64);
    virtual HRESULT STDMETHODCALLTYPE SetProcessingPeriod(PCWSTR, PINT64);
    virtual HRESULT STDMETHODCALLTYPE GetShareMode(PCWSTR, struct DeviceShareMode *);
    virtual HRESULT STDMETHODCALLTYPE SetShareMode(PCWSTR, struct DeviceShareMode *);
    virtual HRESULT STDMETHODCALLTYPE GetPropertyValue(PCWSTR, const PROPERTYKEY &, PROPVARIANT *);
    virtual HRESULT STDMETHODCALLTYPE SetPropertyValue(PCWSTR, const PROPERTYKEY &, PROPVARIANT *);
    virtual HRESULT STDMETHODCALLTYPE SetDefaultEndpoint(IN PCWSTR wszDeviceId, IN ERole eRole);
    virtual HRESULT STDMETHODCALLTYPE SetEndpointVisibility(PCWSTR, INT);
};

class IPolicyConfigWin10 : public IPolicyConfig
{};

__CRT_UUID_DECL(IPolicyConfig, 0xF8679F50, 0x850A, 0x41CF, 0x9C, 0x72, 0x43, 0x0F, 0x29, 0x02, 0x90, 0xC8)
__CRT_UUID_DECL(IPolicyConfigWin10, 0xCA286FC3, 0x91FD, 0x42C3, 0x8E, 0x9B, 0xCA, 0xAF, 0xA6, 0x62, 0x42, 0xE3)

class DECLSPEC_UUID("870af99c-171d-4f9e-af0d-e63df40c2bc9") CPolicyConfigClient;
__CRT_UUID_DECL(CPolicyConfigClient, 0x870AF99C, 0x171D, 0x4F9E, 0xAF, 0x0D, 0xE6, 0x3D, 0xF4, 0x0C, 0x2B, 0xC9)

//-------------------------------------------------------------------------------------------------
static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HFONT hFont;
    static bool bTimerActive = false;
    static const wchar_t *wSoundCardName;

    switch (uMsg)
    {
    case WM_CREATE:
    {
        if ((hFont = CreateFontW(48, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, L"Tahoma")) &&
                SetLayeredWindowAttributes(hWnd, 0, 0, LWA_COLORKEY) &&
                SetTimer(hWnd, 1, 1500, nullptr))
        {
            bTimerActive = true;
            wSoundCardName = static_cast<wchar_t*>(reinterpret_cast<const CREATESTRUCT*>(lParam)->lpCreateParams);
            return 0;
        }
        return -1;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        if (const HDC hDc = BeginPaint(hWnd, &ps))
        {
            RECT rect;
            if (GetClientRect(hWnd, &rect))
            {
                rect.top = 40;
                SelectObject(hDc, hFont);
                SetBkColor(hDc, RGB(1, 1, 1));
                SetTextColor(hDc, RGB(255, 127, 0));
                DrawTextW(hDc, wSoundCardName, -1, &rect, DT_CENTER);
            }
            EndPaint(hWnd, &ps);
        }
        return 0;
    }
    case WM_TIMER:
    {
        KillTimer(hWnd, 1);
        bTimerActive = false;
        PostMessageW(hWnd, WM_CLOSE, 0, 0);
        return 0;
    }
    case WM_DESTROY:
    {
        if (bTimerActive)
            KillTimer(hWnd, 1);
        if (hFont)
            DeleteObject(hFont);
        PostQuitMessage(0);
        return 0;
    }
    }
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

//-------------------------------------------------------------------------------------------------
void FMain()
{
    if (const HWND hWnd = FindWindowW(g_wGuidClass, nullptr))
        PostMessageW(hWnd, WM_CLOSE, 0, 0);

    if (SUCCEEDED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED)))
    {
        IMMDeviceEnumerator *immDeviceEnumerator;
        if (CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), reinterpret_cast<LPVOID*>(&immDeviceEnumerator)) == S_OK)
        {
            IMMDevice *immDeviceDefault;
            if (immDeviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &immDeviceDefault) == S_OK)
            {
                wchar_t *wIdDefaultOld;
                HRESULT hr = immDeviceDefault->GetId(&wIdDefaultOld);
                immDeviceDefault->Release();
                if (hr == S_OK)
                {
                    IMMDeviceCollection *immDeviceCollection;
                    hr = immDeviceEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &immDeviceCollection);
                    immDeviceEnumerator->Release();
                    if (hr == S_OK)
                    {
                        UINT iCount;
                        if (immDeviceCollection->GetCount(&iCount) == S_OK)
                        {
                            bool bFail = true;
                            for (UINT i = 0; i < iCount; ++i)
                            {
                                IMMDevice *immDevice;
                                if (immDeviceCollection->Item(i, &immDevice) == S_OK)
                                {
                                    wchar_t *wIdEnum;
                                    hr = immDevice->GetId(&wIdEnum);
                                    immDevice->Release();
                                    if (hr == S_OK)
                                    {
                                        if (FCompareMemoryW(wIdDefaultOld, wIdEnum))
                                        {
                                            bFail = false;
                                            if (++i >= iCount)
                                                i = 0;
                                            hr = immDeviceCollection->Item(i, &immDevice);
                                            immDeviceCollection->Release();
                                            if (hr == S_OK)
                                            {
                                                wchar_t *wIdDefaultNew;
                                                if (immDevice->GetId(&wIdDefaultNew) == S_OK)
                                                {
                                                    IPropertyStore *ipStore;
                                                    hr = immDevice->OpenPropertyStore(STGM_READ, &ipStore);
                                                    immDevice->Release();
                                                    if (hr == S_OK)
                                                    {
                                                        PROPVARIANT propFriendlyName;
                                                        PropVariantInitFix(&propFriendlyName);
                                                        PROPERTYKEY propKeyFriendlyName;
                                                        propKeyFriendlyName.fmtid.Data1 = 0xA45C254E;
                                                        propKeyFriendlyName.fmtid.Data2 = 0xDF1C;
                                                        propKeyFriendlyName.fmtid.Data3 = 0x4EFD;
                                                        FCopyMemory(propKeyFriendlyName.fmtid.Data4);
                                                        propKeyFriendlyName.pid = 14;
                                                        hr = ipStore->GetValue(propKeyFriendlyName, &propFriendlyName);
                                                        ipStore->Release();
                                                        if (SUCCEEDED(hr))
                                                        {
                                                            IPolicyConfig *pPolicyConfig;
                                                            if (CoCreateInstance(__uuidof(CPolicyConfigClient), nullptr, CLSCTX_ALL, (GetVersion() & 0xFF) >= 10 ? __uuidof(IPolicyConfigWin10) : __uuidof(IPolicyConfig), reinterpret_cast<LPVOID*>(&pPolicyConfig)) == S_OK)
                                                            {
                                                                hr = pPolicyConfig->SetDefaultEndpoint(wIdDefaultNew, eConsole);
                                                                if (hr == S_OK)
                                                                {
                                                                    pPolicyConfig->SetDefaultEndpoint(wIdDefaultNew, eMultimedia);
                                                                    pPolicyConfig->SetDefaultEndpoint(wIdDefaultNew, eCommunications);
                                                                }
                                                                pPolicyConfig->Release();
                                                                if (hr == S_OK)
                                                                {
                                                                    WNDCLASSEX wndCl;
                                                                    wndCl.cbSize = sizeof(WNDCLASSEX);
                                                                    wndCl.style = 0;
                                                                    wndCl.lpfnWndProc = WindowProc;
                                                                    wndCl.cbClsExtra = 0;
                                                                    wndCl.cbWndExtra = 0;
                                                                    wndCl.hInstance = GetModuleHandleW(nullptr);
                                                                    wndCl.hIcon = nullptr;
                                                                    wndCl.hCursor = nullptr;
                                                                    wndCl.hbrBackground = nullptr;
                                                                    wndCl.lpszMenuName = nullptr;
                                                                    wndCl.lpszClassName = g_wGuidClass;
                                                                    wndCl.hIconSm = nullptr;

                                                                    if (RegisterClassExW(&wndCl))
                                                                    {
                                                                        if (CreateWindowExW(WS_EX_NOACTIVATE | WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT | WS_EX_TOPMOST, g_wGuidClass, nullptr, WS_POPUP | WS_VISIBLE | WS_MAXIMIZE, 0, 0, 0, 0, nullptr, nullptr, wndCl.hInstance, propFriendlyName.pwszVal))
                                                                        {
                                                                            MSG msg;
                                                                            while (GetMessageW(&msg, nullptr, 0, 0) > 0)
                                                                                DispatchMessageW(&msg);
                                                                        }
                                                                        UnregisterClassW(g_wGuidClass, wndCl.hInstance);
                                                                    }
                                                                }
                                                            }
                                                        }
                                                        PropVariantClear(&propFriendlyName);
                                                    }
                                                    CoTaskMemFree(wIdDefaultNew);
                                                }
                                                else
                                                    immDevice->Release();
                                            }
                                            break;
                                        }
                                        CoTaskMemFree(wIdEnum);
                                    }
                                }
                            }
                            if (bFail)
                                immDeviceCollection->Release();
                        }
                        else
                            immDeviceCollection->Release();
                    }
                    CoTaskMemFree(wIdDefaultOld);
                }
                else
                    immDeviceEnumerator->Release();
            }
            else
                immDeviceEnumerator->Release();
        }
        CoUninitialize();
    }
}

//-------------------------------------------------------------------------------------------------
extern "C" int start()
{
    FMain();
    ExitProcess(0);
    return 0;
}
