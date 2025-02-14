#pragma once
#include <cstdint>
#include <Windows.h>
#include <format>
#include <stdio.h>

#include "output_window.hpp"


#if !_HAS_CXX20
#error error: xout only supports [>= C++20] (for std::vformat)
#endif

namespace xout
{
	namespace _internal
	{
		inline HANDLE _con_handle = nullptr;
		inline bool _init_flag = false;
		inline WORD _saved_attrs = 0;
		inline std::shared_ptr<output_window> out = nullptr;

		enum log_level : uint8_t
		{
			log,
			info,
			debug,
			warning,
			error,
			critical,
		};


		inline void print_prefix(const log_level lvl)
		{
			char prefix_buff[] = { ' ','-', ' ', '|', ' ', 0x00 };
			COLORREF text_bg = 0x00000000;
			COLORREF text_fg = 0x00FFFFFF;

			switch (lvl)
			{
			case log:
				prefix_buff[1] = '+';
				break;
			case info:
				text_fg = 0x00FF0000;
				prefix_buff[1] = 'i';
				break;
			case debug:
				text_fg = 0x00FFFF00;
				prefix_buff[1] = '?';
				break;
			case warning:
				text_fg = 0x00FF00FF;
				prefix_buff[1] = '~';
				break;
			case error:
				text_fg = 0x000000FF;
				prefix_buff[1] = 'x';
				break;
			case critical:
				text_fg = 0x00000000;
				text_bg = 0x000000FF;
				prefix_buff[1] = '!';
				break;
			}


			_internal::out->set_text_bg(text_bg);
			_internal::out->set_text_fg(text_fg);

			_internal::out->cout() << prefix_buff;
			_internal::out->cout().flush();

			_internal::out->set_text_bg(0x00000000);
			_internal::out->set_text_fg(0x00FFFFFF);
		}

	}


	template <class... _Types>
	void log(const std::format_string<_Types...> _Fmt, _Types&&... _Args) {
		if (_internal::out == nullptr) { return; }
		_internal::print_prefix(_internal::log);
		std::string str = ::std::vformat(_Fmt.get(), ::std::make_format_args(_Args...));
		str.push_back('\n');
		_internal::out->cout() << str;
	}

	template <class... _Types>
	void info(const std::format_string<_Types...> _Fmt, _Types&&... _Args) {
		if (_internal::out == nullptr) { return; }
		_internal::print_prefix(_internal::info);
		std::string str = ::std::vformat(_Fmt.get(), ::std::make_format_args(_Args...));
		str.push_back('\n');
		_internal::out->cout() << str;
	}

	template <class... _Types>
	void debug(const std::format_string<_Types...> _Fmt, _Types&&... _Args) {
		if (_internal::out == nullptr) { return; }

		_internal::print_prefix(_internal::debug);
		std::string str = ::std::vformat(_Fmt.get(), ::std::make_format_args(_Args...));
		str.push_back('\n');
		_internal::out->cout() << str;
	}

	template <class... _Types>
	void warning(const std::format_string<_Types...> _Fmt, _Types&&... _Args) {
		if (_internal::out == nullptr) { return; }

		_internal::print_prefix(_internal::warning);
		std::string str = ::std::vformat(_Fmt.get(), ::std::make_format_args(_Args...));
		str.push_back('\n');
		_internal::out->cout() << str;
	}

	template <class... _Types>
	void error(const std::format_string<_Types...> _Fmt, _Types&&... _Args) {
		if (_internal::out == nullptr) { return; }

		_internal::print_prefix(_internal::error);
		std::string str = ::std::vformat(_Fmt.get(), ::std::make_format_args(_Args...));
		str.push_back('\n');
		_internal::out->cout() << str;
	}

	template <class... _Types>
	void critical(const std::format_string<_Types...> _Fmt, _Types&&... _Args) {
		if (_internal::out == nullptr) { return; }

		_internal::print_prefix(_internal::critical);
		std::string str = ::std::vformat(_Fmt.get(), ::std::make_format_args(_Args...));
		str.push_back('\n');

		_internal::out->set_text_fg(0x00000000);
		_internal::out->set_text_bg(0x000000FF);

		_internal::out->cout() << str;

		_internal::out->set_text_bg(0x00000000);
		_internal::out->set_text_fg(0x00FFFFFF);
	}

	inline bool init(HINSTANCE hInstance) {
		_internal::out = output_window::create_async(hInstance);
		_internal::out->set_control_background_color(0x00000000);
		_internal::out->set_window_title_bar_color(0x00000000);
		_internal::_init_flag = true;
		return true;
	}

	inline bool should_exit()
	{
		return output_window::ready_to_exit;
	}

}
