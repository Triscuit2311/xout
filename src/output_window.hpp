#pragma once
#include <windows.h>
#include <richedit.h>
#include <iostream>
#include <streambuf>
#include <string>
#include <memory>
#include <future>




namespace xout
{
	constexpr int height = 300;
	constexpr int width = 500;
}

class edit_control_buffer_w final : public std::wstreambuf
{
public:
	edit_control_buffer_w(HWND h_edit, COLORREF* fg, COLORREF* bg)
		: h_edit_(h_edit), foreground_color_ptr_(fg), background_color_ptr_(bg)
	{
	}

protected:
	wint_t overflow(wint_t c) override
	{
		if (c != WEOF)
		{
			buffer_.push_back(static_cast<wchar_t>(c));
			if (c == L'\n') { flush_buffer(); }
		}
		return c;
	}

	int sync() override
	{
		flush_buffer();
		return 0;
	}

private:
	void flush_buffer()
	{
		if (!buffer_.empty())
		{
			int length = GetWindowTextLengthW(h_edit_);
			SendMessageW(h_edit_, EM_SETSEL, length, length);
			CHARFORMAT2W cf = {0};
			cf.cbSize = sizeof(CHARFORMAT2W);
			cf.dwMask = CFM_COLOR | CFM_BACKCOLOR;
			cf.crTextColor = *foreground_color_ptr_;
			cf.crBackColor = *background_color_ptr_;
			SendMessageW(h_edit_, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
			SendMessageW(h_edit_, EM_REPLACESEL, 0, (LPARAM)buffer_.c_str());
			SendMessageW(h_edit_, WM_VSCROLL, SB_BOTTOM, 0);
			buffer_.clear();
		}
	}

	HWND h_edit_;
	std::wstring buffer_;
	COLORREF* foreground_color_ptr_;
	COLORREF* background_color_ptr_;
};

class edit_control_buffer_a final : public std::streambuf
{
public:
	edit_control_buffer_a(HWND h_edit, COLORREF* fg, COLORREF* bg)
		: h_edit_(h_edit), foreground_color_ptr_(fg), background_color_ptr_(bg)
	{
	}

protected:
	int overflow(int c) override
	{
		if (c != EOF)
		{
			buffer_.push_back(static_cast<char>(c));
			if (c == '\n')
			{
				flush_buffer();
			}
		}
		return c;
	}

	int sync() override
	{
		flush_buffer();
		return 0;
	}

private:
	void flush_buffer()
	{
		if (!buffer_.empty())
		{
			int len = MultiByteToWideChar(CP_ACP, 0, buffer_.c_str(), -1, nullptr, 0);
			if (len > 0)
			{
				std::wstring wstr;
				wstr.resize(len - 1);
				MultiByteToWideChar(CP_ACP, 0, buffer_.c_str(), -1, &wstr[0], len);
				int length = GetWindowTextLengthW(h_edit_);
				SendMessageW(h_edit_, EM_SETSEL, length, length);
				CHARFORMAT2W cf = {0};
				cf.cbSize = sizeof(CHARFORMAT2W);
				cf.dwMask = CFM_COLOR | CFM_BACKCOLOR;
				cf.crTextColor = *foreground_color_ptr_;
				cf.crBackColor = *background_color_ptr_;
				SendMessageW(h_edit_, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
				SendMessageW(h_edit_, EM_REPLACESEL, 0, (LPARAM)wstr.c_str());
				SendMessageW(h_edit_, WM_VSCROLL, SB_BOTTOM, 0);
			}
			buffer_.clear();
		}
	}

	HWND h_edit_;
	std::string buffer_;
	COLORREF* foreground_color_ptr_;
	COLORREF* background_color_ptr_;
};

class output_window
{
	static constexpr const wchar_t* k_window_class_name = L"xout_output_cls";
	HINSTANCE h_instance_;
	HWND h_window_;
	HWND h_edit_;
	std::unique_ptr<edit_control_buffer_a> buffer_a_;
	std::unique_ptr<edit_control_buffer_w> buffer_w_;

	COLORREF text_foreground_color_;
	COLORREF text_background_color_;
	std::unique_ptr<std::ostream> cstream_;
	std::unique_ptr<std::wostream> wcstream_;

	static HFONT set_font(const HWND h_edit);
	static LRESULT CALLBACK window_proc(HWND h_wnd, UINT message, WPARAM w_param, LPARAM l_param);
	void register_window_class();
	void create_window(const std::wstring& title, int width, int height);
	void create_edit_control();

public:
	static bool ready_to_exit;

	std::ostream& cout() const;
	std::wostream& wcout() const;

	explicit output_window(HINSTANCE h_instance, const std::wstring& title = L"xout", int width = xout::width,
	                       int height = xout::height);

	~output_window();

	void show(int n_cmd_show) const;

	static void run_message_loop();

	void set_text_bg(COLORREF color);

	void set_text_fg(COLORREF color);

	void set_title(const std::wstring& title) const;

	void set_control_background_color(COLORREF color) const;

	void set_window_title_bar_color(COLORREF color) const;

	void set_window_title_text_color(COLORREF color) const;


	static std::shared_ptr<output_window> create_async(
		HINSTANCE h_instance,
		const std::wstring& title = L"xout",
		int width = xout::width,
		int height = xout::height,
		int n_cmd_show = SW_SHOW);
};
