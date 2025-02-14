#include "output_window.hpp"
#include "jbmono.h"
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

bool output_window::ready_to_exit = false;

std::ostream& output_window::cout() const
{
	return *cstream_;
}

std::wostream& output_window::wcout() const
{
	return *wcstream_;
}

output_window::output_window(HINSTANCE h_instance, const std::wstring& title, int width, int height) : h_instance_(h_instance), h_window_(nullptr), h_edit_(nullptr),
text_foreground_color_(RGB(255, 255, 255)), text_background_color_(RGB(0, 0, 0))
{
	LoadLibrary("Msftedit.dll");
	register_window_class();
	create_window(title, width, height + 25);
	create_edit_control();
	buffer_a_ = std::make_unique<edit_control_buffer_a>(h_edit_, &text_foreground_color_, &text_background_color_);
	buffer_w_ = std::make_unique<edit_control_buffer_w>(h_edit_, &text_foreground_color_, &text_background_color_);

	cstream_ = std::make_unique<std::ostream>(buffer_a_.get());
	wcstream_ = std::make_unique<std::wostream>(buffer_w_.get());
}

output_window::~output_window()
{
	if (h_window_) DestroyWindow(h_window_);
}

void output_window::show(int n_cmd_show) const
{
	ShowWindow(h_window_, n_cmd_show);
}

void output_window::run_message_loop()
{
	MSG msg;
	while (GetMessageW(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
}

void output_window::set_text_bg(COLORREF color)
{
	text_background_color_ = color;
}

void output_window::set_text_fg(COLORREF color)
{
	text_foreground_color_ = color;
}

void output_window::set_title(const std::wstring& title) const
{
	if (h_window_) { SetWindowTextW(h_window_, title.c_str()); }
}

void output_window::set_control_background_color(COLORREF color) const
{
	if (h_edit_)
	{
		SendMessageW(h_edit_, EM_SETBKGNDCOLOR, 0, (LPARAM)color);
		InvalidateRect(h_edit_, nullptr, TRUE);
	}
}

void output_window::set_window_title_bar_color(COLORREF color) const
{
	DwmSetWindowAttribute(h_window_, DWMWA_CAPTION_COLOR, &color, sizeof(color));
}

void output_window::set_window_title_text_color(COLORREF color) const
{
	DwmSetWindowAttribute(h_window_, DWMWA_TEXT_COLOR, &color, sizeof(color));
}

std::shared_ptr<output_window> output_window::create_async(HINSTANCE h_instance, const std::wstring& title,
	int width, int height, int n_cmd_show)
{
	std::promise<std::shared_ptr<output_window>> promise;
	auto future = promise.get_future();
	std::thread([h_instance, title, width, height, n_cmd_show, p = std::move(promise)]() mutable
		{
			auto console = std::make_shared<output_window>(h_instance, title, width, height);
			p.set_value(console);
			console->show(n_cmd_show);
			console->run_message_loop();
		}).detach();
	return future.get();
}

HFONT output_window::set_font(const HWND h_edit)
{
	DWORD num_fonts = 0;
	HANDLE h_font_res = AddFontMemResourceEx((void*)jbmono_font::jbmono_data, jbmono_font::jbmono_sz, 0,
		&num_fonts);

	LOGFONTW lf = { 0 };
	HDC hdc = GetDC(h_edit);
	lf.lfHeight = -MulDiv(10, GetDeviceCaps(hdc, LOGPIXELSY), 72);
	ReleaseDC(h_edit, hdc);
	wcscpy_s(lf.lfFaceName, jbmono_font::jbmono_name);

	HFONT h_font = CreateFontIndirectW(&lf);
	if (!h_font)
	{
		wcscpy_s(lf.lfFaceName, L"Segoe UI");
		h_font = CreateFontIndirectW(&lf);
	}
	SendMessageW(h_edit, WM_SETFONT, (WPARAM)h_font, TRUE);
	return h_font;
}

LRESULT output_window::window_proc(HWND h_wnd, UINT message, WPARAM w_param, LPARAM l_param)
{
	switch (message)
	{
	case WM_NCCREATE:
	{
		auto* cs = reinterpret_cast<CREATESTRUCTW*>(l_param);
		output_window* console = reinterpret_cast<output_window*>(cs->lpCreateParams);
		SetWindowLongPtrW(h_wnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(console));
		console->h_window_ = h_wnd;
		break;
	}
	case WM_SIZE:
	{
		output_window* console = reinterpret_cast<output_window*>(GetWindowLongPtrW(h_wnd, GWLP_USERDATA));
		if (console && console->h_edit_)
		{
			RECT rc;
			GetClientRect(h_wnd, &rc);
			MoveWindow(console->h_edit_, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
		}
		break;
	}
	case WM_DESTROY:
	{
		output_window* ptr = reinterpret_cast<output_window*>(GetWindowLongPtrW(h_wnd, GWLP_USERDATA));
		if (ptr)
		{
			output_window::ready_to_exit = true;
			PostQuitMessage(0);
		}
	}
	}

	return DefWindowProcW(h_wnd, message, w_param, l_param);
}

void output_window::register_window_class()
{
	WNDCLASSW wc = {};
	wc.lpfnWndProc = output_window::window_proc;
	wc.hInstance = h_instance_;
	wc.lpszClassName = k_window_class_name;
	RegisterClassW(&wc);
}

void output_window::create_window(const std::wstring& title, int width, int height)
{
	h_window_ = CreateWindowExW(0, k_window_class_name, title.c_str(), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, width, height,
		nullptr, nullptr, h_instance_, this);
}

void output_window::create_edit_control()
{
	h_edit_ = CreateWindowExW(0, L"RichEdit50W", L"",
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL,
		0, 0, xout::width, xout::height, h_window_, nullptr, h_instance_, nullptr);
	set_font(h_edit_);
}