#include <mmdeviceapi.h>

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
__CRT_UUID_DECL(IPolicyConfig, 0xf8679f50, 0x850a, 0x41cf, 0x9c, 0x72, 0x43, 0x0f, 0x29, 0x02, 0x90, 0xc8)

class DECLSPEC_UUID("870af99c-171d-4f9e-af0d-e63df40c2bc9") CPolicyConfigClient;
__CRT_UUID_DECL(CPolicyConfigClient, 0x870af99c, 0x171d, 0x4f9e, 0xaf, 0x0d, 0xe6, 0x3d, 0xf4, 0x0c, 0x2b, 0xc9)

//-------------------------------------------------------------------------------------------------
const wchar_t *const g_wGuidClass = L"15423203-ff9d-2e13-1788-e087bcd8dbcb";
wchar_t *g_wSoundCardName;

enum
{
    eFontHeight = 48
};

//-------------------------------------------------------------------------------------------------
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HFONT hFont;
    switch (uMsg)
    {
    case WM_CREATE:
        return (hFont = CreateFont(eFontHeight, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, L"Tahoma")) ? 0 : -1;
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
                DrawText(hDc, g_wSoundCardName, -1, &rect, DT_CENTER);
            }
            EndPaint(hWnd, &ps);
        }
        return 0;
    }
    case WM_TIMER:
        KillTimer(hWnd, 1);
        PostMessage(hWnd, WM_CLOSE, 0, 0);
        return 0;
    case WM_DESTROY:
        if (hFont)
            DeleteObject(hFont);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

//-------------------------------------------------------------------------------------------------
int main()
{
    if (const HWND hWnd = FindWindow(g_wGuidClass, 0))
        PostMessage(hWnd, WM_CLOSE, 0, 0);

    if (SUCCEEDED(CoInitializeEx(0, COINIT_APARTMENTTHREADED)))
    {
        IMMDeviceEnumerator *immDeviceEnumerator;
        if (CoCreateInstance(__uuidof(MMDeviceEnumerator), 0, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), reinterpret_cast<LPVOID*>(&immDeviceEnumerator)) == S_OK)
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
                                        if (wcscmp(wIdDefaultOld, wIdEnum) == 0)
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
                                                        PropVariantInit(&propFriendlyName);
                                                        PROPERTYKEY propKeyFriendlyName;
                                                        propKeyFriendlyName.fmtid.Data1 = 0xA45C254E;
                                                        propKeyFriendlyName.fmtid.Data2 = 0xDF1C;
                                                        propKeyFriendlyName.fmtid.Data3 = 0x4EFD;
                                                        propKeyFriendlyName.fmtid.Data4[0] = '\x80';
                                                        propKeyFriendlyName.fmtid.Data4[1] = '\x20';
                                                        propKeyFriendlyName.fmtid.Data4[2] = '\x67';
                                                        propKeyFriendlyName.fmtid.Data4[3] = '\xD1';
                                                        propKeyFriendlyName.fmtid.Data4[4] = '\x46';
                                                        propKeyFriendlyName.fmtid.Data4[5] = '\xA8';
                                                        propKeyFriendlyName.fmtid.Data4[6] = '\x50';
                                                        propKeyFriendlyName.fmtid.Data4[7] = '\xE0';
                                                        propKeyFriendlyName.pid = 14;
                                                        hr = ipStore->GetValue(propKeyFriendlyName, &propFriendlyName);
                                                        ipStore->Release();
                                                        if (SUCCEEDED(hr))
                                                        {
                                                            IPolicyConfig *pPolicyConfig;
                                                            if (CoCreateInstance(__uuidof(CPolicyConfigClient), 0, CLSCTX_ALL, __uuidof(IPolicyConfig), reinterpret_cast<LPVOID*>(&pPolicyConfig)) == S_OK)
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
                                                                    WNDCLASS wndCl;
                                                                    wndCl.style = 0;
                                                                    wndCl.lpfnWndProc = WindowProc;
                                                                    wndCl.cbClsExtra = 0;
                                                                    wndCl.cbWndExtra = 0;
                                                                    wndCl.hInstance = GetModuleHandle(0);
                                                                    wndCl.hIcon = 0;
                                                                    wndCl.hCursor = 0;
                                                                    wndCl.hbrBackground = 0;
                                                                    wndCl.lpszMenuName = 0;
                                                                    wndCl.lpszClassName = g_wGuidClass;

                                                                    if (RegisterClass(&wndCl))
                                                                    {
                                                                        g_wSoundCardName = propFriendlyName.pwszVal;
                                                                        if (const HWND hWnd = CreateWindowEx(WS_EX_NOACTIVATE | WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT | WS_EX_TOPMOST, g_wGuidClass, 0, WS_POPUP | WS_VISIBLE | WS_MAXIMIZE, 0, 0, 0, 0, 0, 0, wndCl.hInstance, 0))
                                                                        {
                                                                            SetLayeredWindowAttributes(hWnd, 0, 0, LWA_COLORKEY);
                                                                            SetTimer(hWnd, 1, 1500, 0);
                                                                            MSG msg;
                                                                            while (GetMessage(&msg, 0, 0, 0) > 0)
                                                                                DispatchMessage(&msg);
                                                                        }
                                                                        UnregisterClass(g_wGuidClass, wndCl.hInstance);
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
    return 0;
}
