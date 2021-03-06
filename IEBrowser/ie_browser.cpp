#include "stdafx.h"
#include "ie_browser.h"

#include "com_utility.h"
#include "ie_browser_setting.h"

// WebBrowser 控件的 CLSID
const wchar_t* kWebBrowserCLSID = L"{8856F961-340A-11D0-A96B-00C04FD705A2}";

// 窗口的 style 取值
enum IEBrowserWindowStyle
{
    // Popup 类型窗口的 style 和 ex_style
    WS_IEBROWSER_POPUP = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
    WS_EX_IEBROWSER_POPUP = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,

    // Child 类型窗口的 style 和 ex_style
    WS_IEBROWSER_CHILD = WS_CHILD | WS_CLIPSIBLINGS,
    WS_EX_IEBROWSER_CHILD = WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
};

IEBrowser::IEBrowser() :
    delegate_(nullptr),
    is_being_destroyed_(false)
{
}

IEBrowser::~IEBrowser()
{
}

bool IEBrowser::Initialize(IEBrowserSetting& setting)
{
    bool result = false;

    do
    {
        // 创建容器窗口
        if (!CreateBrowserWindow(setting.parent_window_handle_))
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
            // 取消连接点
            DispEventUnadvise(web_browser_, &DIID_DWebBrowserEvents2);

            // 停止正在进行的导航
            web_browser_->Stop();

            // 通知 web_browser 控件关闭自己
            web_browser_->ExecWB(OLECMDID_CLOSE, OLECMDEXECOPT_DONTPROMPTUSER, 0, 0);

            // 释放引用计数
            web_browser_ = nullptr;
        }

        // 先解除父子关系，然后销毁窗口
        SetParent(nullptr);
        SetWindowLong(GWL_STYLE, WS_IEBROWSER_POPUP);
        DestroyWindow();
    }
}

void IEBrowser::SetDelegate(Delegate * delegate)
{
    delegate_ = delegate;
}

void IEBrowser::RemoveDelegate()
{
    delegate_ = nullptr;
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

        if (url == nullptr)
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

bool IEBrowser::Stop()
{
    bool result = false;

    do
    {
        if (web_browser_ == nullptr)
        {
            break;
        }

        web_browser_->Stop();

        result = true;

    } while (false);

    return result;
}

bool IEBrowser::Refresh()
{
    bool result = false;

    do
    {
        if (web_browser_ == nullptr)
        {
            break;
        }

        web_browser_->Refresh();

        result = true;

    } while (false);

    return result;
}

bool IEBrowser::GoForward()
{
    bool result = false;

    do
    {
        if (web_browser_ == nullptr)
        {
            break;
        }

        web_browser_->GoForward();

        result = true;

    } while (false);

    return result;
}

bool IEBrowser::GoBack()
{
    bool result = false;

    do
    {
        if (web_browser_ == nullptr)
        {
            break;
        }

        web_browser_->GoBack();

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

    do
    {
        // 不知道是什么原因，OnDestroy 方法会进入两次
        // 如果设置了父窗口，而 OnDestroy 中两次调用了 DefWindowProc 的话，线程会卡死
        if (is_being_destroyed_)
        {
            break;
        }

        is_being_destroyed_ = true;

        result = DefWindowProc(message, w_param, l_param);
        is_handled = FALSE;

    } while (false);

    return result;
}

void IEBrowser::OnBeforeNavigate2(
    IDispatch * dispatch, 
    VARIANT * url, 
    VARIANT * flags, 
    VARIANT * target_frame_name, 
    VARIANT * post_data, 
    VARIANT * headers, 
    VARIANT_BOOL * cancel)
{
    if (delegate_)
    {
        delegate_->OnBeforeNavigate(
            IsMainFrame(dispatch),
            ComUtility::VTToString(url),
            ComUtility::VTToString(target_frame_name),
            ComUtility::VTToString(post_data),
            ComUtility::VTToString(headers));
    }
}

void IEBrowser::OnNavigateComplete2(IDispatch * dispatch, VARIANT * url)
{
    if (delegate_)
    {
        delegate_->OnNativeComplete(IsMainFrame(dispatch), ComUtility::VTToString(url));
    }
}

void IEBrowser::OnNavigateError(
    IDispatch * dispatch, 
    VARIANT * url, 
    VARIANT * target_frame_name, 
    VARIANT * status_code, 
    VARIANT_BOOL * cancel)
{
    if (delegate_)
    {
        delegate_->OnNavigateError(
            IsMainFrame(dispatch), 
            ComUtility::VTToString(url), 
            ComUtility::VTToString(target_frame_name), 
            ComUtility::VTToLong(status_code));
    }
}

void IEBrowser::OnDocumentComplete(IDispatch * dispatch, VARIANT * url)
{
    if (delegate_)
    {
        delegate_->OnDocumentComplete(IsMainFrame(dispatch), ComUtility::VTToString(url));
    }
}

void IEBrowser::OnTitleChange(BSTR title)
{
    if (delegate_)
    {
        delegate_->OnTitleChange(title);
    }
}

void IEBrowser::OnStatusTextChange(BSTR status_text)
{
    if (delegate_)
    {
        delegate_->OnStatusTextChange(status_text);
    }
}

void IEBrowser::OnCommandStateChange(long command, VARIANT_BOOL enable)
{
    if (delegate_)
    {
        bool can_go_forward = false;
        bool can_go_back = false;
        bool can_refresh = false;

        if (enable == VARIANT_TRUE)
        {
            if (command == CSC_UPDATECOMMANDS)
            {
                can_refresh = true;
            }
            else if (command == CSC_NAVIGATEBACK)
            {
                can_go_forward = true;
            }
            else if (command == CSC_NAVIGATEFORWARD)
            {
                can_go_forward = true;
            }
        }

        delegate_->OnCommandStateChange(can_refresh, can_go_forward, can_go_back);
    }
}

void IEBrowser::OnProgressChange(long progress, long max_progress)
{
    if (delegate_)
    {
        delegate_->OnProgressChange(progress, max_progress);
    }
}

bool IEBrowser::CreateBrowserWindow(HWND parent_window_handle)
{
    bool result = false;

    do
    {
        // 创建容器窗口
        if (Create(
            nullptr,
            CWindow::rcDefault,
            IEBROWSER_WINDOW_NAME,
            WS_IEBROWSER_POPUP,
            WS_EX_IEBROWSER_POPUP) == nullptr)
        {
            break;
        }

        if (::IsWindow(parent_window_handle))
        {
            SetWindowLong(GWL_STYLE, WS_IEBROWSER_CHILD);
            SetWindowLong(GWL_EXSTYLE, WS_EX_IEBROWSER_CHILD);
            SetParent(parent_window_handle);
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
        // 创建 WebBrowser 控件
        if (FAILED(CreateControl(kWebBrowserCLSID)))
        {
            break;
        }

        // 获取 IWebBrowser2 接口
        if (FAILED(QueryControl(IID_IWebBrowser2, (void**)&web_browser_)))
        {
            break;
        }

        // 建立连接点
        if (FAILED(DispEventAdvise(web_browser_, &DIID_DWebBrowserEvents2)))
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

bool IEBrowser::IsMainFrame(IDispatch * dispatch)
{
    bool result = false;

    for (;;)
    {
        if (dispatch == nullptr)
        {
            break;
        }

        if (web_browser_ == nullptr)
        {
            break;
        }

        CComPtr<IDispatch> top_dispatch;
        if (FAILED(web_browser_->QueryInterface(IID_IDispatch, (void**)&top_dispatch)))
        {
            break;
        }

        if (!top_dispatch.IsEqualObject(dispatch))
        {
            break;
        }

        result = true;

        break;
    }

    return result;
}
