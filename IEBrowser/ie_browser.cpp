#include "stdafx.h"
#include "ie_browser.h"

const wchar_t* kWebBrowserCLSID = L"{8856F961-340A-11D0-A96B-00C04FD705A2}";

IEBrowser::IEBrowser()
{
}

IEBrowser::~IEBrowser()
{
}

bool IEBrowser::Initialize(HWND parent_window_handle)
{
    bool result = false;

    do
    {
        // 创建容器窗口
        if (!CreateBrowserWindow(parent_window_handle))
        {
            break;
        }

        // 创建 WebBrowser 控件
        if (!CreateWebBrowser())
        {
            break;
        }

        result = true;

    } while (false);

    if (!result)
    {
        UnInitialze();
    }

    return result;
}

void IEBrowser::UnInitialze()
{
    if (IsWindow())
    {
        if (web_browser_)
        {
            web_browser_->Stop();
            web_browser_->ExecWB(OLECMDID_CLOSE, OLECMDEXECOPT_DONTPROMPTUSER, 0, 0);
            web_browser_ = nullptr;
        }

        DestroyWindow();
    }
}

bool IEBrowser::Navigate(const wchar_t * url)
{
    bool result = false;

    do
    {
        if (web_browser_ == nullptr)
        {
            break;
        }

        CComVariant url_var(url);

        if (FAILED(web_browser_->Navigate2(&url_var, nullptr, nullptr, nullptr, nullptr)))
        {
            break;
        }

        result = true;

    } while (false);

    return result;
}

LRESULT IEBrowser::OnCreate(unsigned int message, WPARAM w_param, LPARAM l_param, BOOL & is_handled)
{
    LRESULT result = 0;

    result = DefWindowProc(message, w_param, l_param);
    is_handled = FALSE;

    return result;
}

LRESULT IEBrowser::OnDestroy(unsigned int message, WPARAM w_param, LPARAM l_param, BOOL & is_handled)
{
    LRESULT result = 0;

    result = DefWindowProc(message, w_param, l_param);
    is_handled = FALSE;

    return result;
}

bool IEBrowser::CreateBrowserWindow(HWND parent_window_handle)
{
    bool result = false;

    do
    {
        DWORD style = 0;
        DWORD ex_style = 0;

        if (::IsWindow(parent_window_handle))
        {
            // 使用者传入了父窗口, 则创建子窗口
            style = WS_CHILD | WS_CLIPSIBLINGS;
            ex_style = WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE;
        }
        else
        {
            // 使用者没有传入父窗口，则创建顶层窗口
            parent_window_handle = nullptr;
            style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
            ex_style = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
        }

        // 创建容器窗口
        if (Create(
            parent_window_handle,
            CWindow::rcDefault,
            IEBROWSER_WINDOW_NAME,
            style,
            ex_style) == nullptr)
        {
            break;
        }

        UpdateVisible();
        UpdateSize();

        result = true;

    } while (false);

    return result;
}

bool IEBrowser::CreateWebBrowser()
{
    bool result = false;

    do
    {
        if (FAILED(CreateControl(kWebBrowserCLSID)))
        {
            break;
        }

        // 获取 IWebBrowser2 接口
        if (FAILED(QueryControl(IID_IWebBrowser2, (void**)&web_browser_)))
        {
            break;
        }

        result = true;

    } while (false);

    return result;
}

void IEBrowser::UpdateVisible()
{
    CWindow parent_window = GetParent();
    if (parent_window.IsWindow())
    {
        ShowWindow(parent_window.IsWindowVisible() ? SW_SHOW : SW_HIDE);
    }
    else
    {
        ShowWindow(SW_SHOW);
        UpdateWindow();
    }
}

void IEBrowser::UpdateSize()
{
    CWindow parent_window = GetParent();
    if (parent_window.IsWindow())
    {
        RECT parent_window_rect;
        parent_window.GetClientRect(&parent_window_rect);
        
        MoveWindow(&parent_window_rect);
    }
}
