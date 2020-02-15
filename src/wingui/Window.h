
/* Copyright 2020 the SumatraPDF project authors (see AUTHORS file).
   License: Simplified BSD (see COPYING.BSD) */

struct WindowBase;
struct Window;

struct WndEvent {
    // args sent to WndProc
    HWND hwnd = nullptr;
    UINT msg = 0;
    WPARAM wparam = 0;
    LPARAM lparam = 0;

    // indicate if we handled the message and the result (if handled)
    bool didHandle = false;
    LRESULT result = 0;

    // window that logically received the message
    // (we reflect messages sent to parent windows back to real window)
    WindowBase* w = nullptr;
};

#define SetWndEvent(n) \
    {                  \
        n.w = w;       \
        n.hwnd = hwnd; \
        n.msg = msg;   \
        n.wparam = wp; \
        n.lparam = lp; \
    }

struct CopyWndEvent {
    WndEvent* dst = nullptr;
    WndEvent* src = nullptr;

    CopyWndEvent() = delete;

    CopyWndEvent(CopyWndEvent&) = delete;
    CopyWndEvent(CopyWndEvent&&) = delete;
    CopyWndEvent& operator=(CopyWndEvent&) = delete;

    CopyWndEvent(WndEvent* dst, WndEvent* src);
    ~CopyWndEvent();
};

typedef std::function<void(WndEvent*)> MsgFilter;

struct SizeEvent : WndEvent {
    int dx = 0;
    int dy = 0;
};

typedef std::function<void(SizeEvent*)> SizeHandler;

struct ContextMenuEvent : WndEvent {
    // mouse x,y position relative to the window
    PointI mouseWindow{};
    // global (screen) mouse x,y position
    PointI mouseGlobal{};
};

typedef std::function<void(ContextMenuEvent*)> ContextMenuHandler;

struct WindowCloseEvent : WndEvent {
    bool cancel = false;
};

struct WmCommandEvent : WndEvent {
    int id = 0;
    int ev = 0;
};

typedef std::function<void(WmCommandEvent*)> WmCommandHandler;

typedef std::function<void(WindowCloseEvent*)> CloseHandler;

struct WindowDestroyEvent : WndEvent {
    Window* window = nullptr;
};

typedef std::function<void(WindowDestroyEvent*)> DestroyHandler;

struct CharEvent : WndEvent {
    int keyCode = 0;
};

typedef std::function<void(CharEvent*)> CharHandler;

// TODO: extract data from LPARAM
struct KeyEvent : WndEvent {
    int keyVirtCode = 0;
};

typedef std::function<void(KeyEvent*)> KeyHandler;

struct MouseWheelEvent : WndEvent {
    bool isVertical = false;
    int delta = 0;
    u32 keys = 0;
    int x = 0;
    int y = 0;
};

typedef std::function<void(MouseWheelEvent*)> MouseWheelHandler;

extern Kind kindWindowBase;

struct WindowBase {
    Kind kind = nullptr;

    // data that can be set before calling Create()

    // either a custom class that we registered or
    // a win32 control class. Assumed static so not freed
    const WCHAR* winClass = nullptr;

    HWND parent = nullptr;
    Point initialPos = {-1, -1};
    Size initialSize = {0, 0};
    DWORD dwStyle = 0;
    DWORD dwExStyle = 0;
    HFONT hfont = nullptr; // TODO: this should be abstract Font description
    int menuId = 0;

    // those tweak WNDCLASSEX for RegisterClass() class
    HICON hIcon = nullptr;
    HICON hIconSm = nullptr;
    LPCWSTR lpszMenuName = nullptr;

    // called at start of windows proc to allow intercepting messages
    MsgFilter msgFilter;

    // allow handling WM_CONTEXTMENU
    ContextMenuHandler onContextMenu = nullptr;
    // allow handling WM_SIZE
    SizeHandler onSize = nullptr;
    // for WM_COMMAND
    WmCommandHandler onWmCommand = nullptr;
    // for WM_NCDESTROY
    DestroyHandler onDestroy = nullptr;
    // for WM_CLOSE
    CloseHandler onClose = nullptr;
    // for WM_KEYDOWN
    KeyHandler onKeyDown = nullptr;
    // for WM_KEYUP
    KeyHandler onKeyUp = nullptr;
    // for WM_CHAR
    CharHandler onChar = nullptr;
    // for WM_MOUSEWHEEL and WM_MOUSEHWHEEL
    MouseWheelHandler onMouseWheel = nullptr;

    COLORREF textColor = ColorUnset;
    COLORREF backgroundColor = ColorUnset;
    HBRUSH backgroundColorBrush = nullptr;

    str::Str text;

    HWND hwnd = nullptr;
    UINT_PTR subclassId = 0;
    UINT_PTR subclassParentId = 0;

    WindowBase() = default;
    WindowBase(HWND p);
    virtual ~WindowBase();

    virtual bool Create();
    virtual SIZE GetIdealSize();

    virtual void WndProc(WndEvent*);
    virtual void WndProcParent(WndEvent*);

    void Destroy();
    void Subclass();
    void SubclassParent();
    void Unsubclass();

    void SetIsEnabled(bool);
    bool IsEnabled();

    void SetIsVisible(bool);
    bool IsVisible();

    void SuspendRedraw();
    void ResumeRedraw();

    void SetFocus();
    bool IsFocused();

    void SetFont(HFONT f);

    void SetText(const WCHAR* s);
    void SetText(std::string_view);
    std::string_view GetText();

    void SetPos(RECT* r);
    void SetBounds(const RECT& r);
    void SetTextColor(COLORREF);
    void SetBackgroundColor(COLORREF);
    void SetColors(COLORREF bg, COLORREF txt);
    void SetRtl(bool);
};

extern Kind kindWindow;

// a top-level window. Must set winClass before
// calling Create()
struct Window : public WindowBase {
    Window();
    ~Window() override;

    bool Create() override;

    void SetTitle(std::string_view);

    void Close();
};

struct WindowBaseLayout : public ILayout {
    WindowBase* wb = nullptr;

    WindowBaseLayout(WindowBase*, Kind);
    ~WindowBaseLayout() override;

    Size Layout(const Constraints bc) override;
    i32 MinIntrinsicHeight(i32) override;
    i32 MinIntrinsicWidth(i32) override;
    void SetBounds(const Rect bounds) override;
};

void HwndSetText(HWND hwnd, std::string_view s);
UINT_PTR NextSubclassId();
int RunMessageLoop(HACCEL accelTable);
void PositionCloseTo(WindowBase* w, HWND hwnd);
