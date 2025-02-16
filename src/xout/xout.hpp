#pragma once
#include <cstdint>
#include <Windows.h>
#include <format>
#include <queue>

#include "output_window.hpp"
#include "..\shared\shem.hpp"


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
		inline std::shared_ptr<output_window> out_wnd = nullptr;

		namespace colors
		{
			const COLORREF rgb2colorref(uint8_t r, uint8_t g, uint8_t b)
			{
				return static_cast<COLORREF>(r | (g << 8) | (b << 16));
			}

			const COLORREF hex2colorref(uint32_t hex)
			{
				return RGB((hex >> 16) & 0xFF, (hex >> 8) & 0xFF, hex & 0xFF);
			}

			const COLORREF log_fg = hex2colorref(0xcafe48);
			const COLORREF info_fg = hex2colorref(0x4dffb2);
			const COLORREF debug_fg = hex2colorref(0x00bfff);
			const COLORREF warning_fg = hex2colorref(0xffd900);
			const COLORREF error_fg = hex2colorref(0xff3c00);
			const COLORREF critical_fg = hex2colorref(0x000000);
			const COLORREF critical_bg = hex2colorref(0xff3c00);

			
		}

		enum log_level : uint8_t
		{
			log,
			info,
			debug,
			warning,
			error,
			critical,
		};

		class xout_output
		{

		public:
			xout_output()
			{
			}

			void prefix_custom(const std::string& prefix, COLORREF text_bg, COLORREF text_fg)
			{
				_internal::out_wnd->push_colors();

				_internal::out_wnd->set_text_bg(text_bg);
				_internal::out_wnd->set_text_fg(text_fg);

				_internal::out_wnd->cout() << " " << prefix;
				_internal::out_wnd->cout().flush();

				_internal::out_wnd->wcout() << L" ▪ ";
				_internal::out_wnd->wcout().flush();

				_internal::out_wnd->pop_colors();
			}

			void prefix_custom(const std::wstring& prefix, COLORREF text_bg, COLORREF text_fg)
			{
				_internal::out_wnd->push_colors();

				_internal::out_wnd->set_text_bg(text_bg);
				_internal::out_wnd->set_text_fg(text_fg);

				_internal::out_wnd->wcout() << L" " << prefix << L" ▪ ";
				_internal::out_wnd->wcout().flush();

				_internal::out_wnd->pop_colors();
			}

			void prefix(log_level lvl,  bool no_flush = false)
			{
				if (_internal::out_wnd == nullptr) { return; }

				std::wstring prefix = L" ";
				COLORREF text_bg = 0x00000000;
				COLORREF text_fg = 0x00FFFFFF;

				switch (lvl)
				{
				case log:
					text_fg = colors::log_fg;
					prefix += L"+";
					break;
				case info:
					text_fg = colors::info_fg;
					prefix += L"i";
					break;
				case debug:
					text_fg = colors::debug_fg;
					prefix += L"D";
					break;
				case warning:
					text_fg = colors::warning_fg;
					prefix += L"W";
					break;
				case error:
					text_fg = colors::error_fg;
					prefix += L"x";
					break;
				case critical:
					text_fg = colors::critical_fg;
					text_bg = colors::critical_bg;
					prefix += L"X";
					break;
				}


				_internal::out_wnd->push_colors();

				_internal::out_wnd->set_text_bg(text_bg);
				_internal::out_wnd->set_text_fg(text_fg);


				_internal::out_wnd->wcout() << prefix << L" ▪ ";
				if (!no_flush) {
					_internal::out_wnd->wcout().flush();
				}
				
				_internal::out_wnd->pop_colors();

			}

			void put(const std::string& str)
			{
				if (_internal::out_wnd == nullptr) { return; }
				_internal::out_wnd->cout() << str;
			}

			void put(const std::wstring& wstr)
			{
				if (_internal::out_wnd == nullptr) { return; }
				_internal::out_wnd->wcout() << wstr;
			}

			void put(const std::string& str, COLORREF bg, COLORREF fg)
			{
				if (_internal::out_wnd == nullptr) { return; }
				_internal::out_wnd->push_colors();

				_internal::out_wnd->set_text_bg(bg);
				_internal::out_wnd->set_text_fg(fg);

				_internal::out_wnd->cout() << str;
				_internal::out_wnd->cout().flush();
				_internal::out_wnd->pop_colors();
			}

			void put(const std::wstring& wstr, COLORREF bg, COLORREF fg)
			{
				if (_internal::out_wnd == nullptr) { return; }
				_internal::out_wnd->push_colors();
				_internal::out_wnd->set_text_bg(bg);
				_internal::out_wnd->set_text_fg(fg);
				_internal::out_wnd->wcout() << wstr;
				_internal::out_wnd->wcout().flush();
				_internal::out_wnd->pop_colors();
			}



			void server_marshall(const std::string& str, log_level lvl, bool first, bool last)
			{
				if (_internal::out_wnd == nullptr) { return; }

				if (first) {
					prefix(lvl);
				}

				if (lvl == critical){
					_internal::out_wnd->push_colors();
					_internal::out_wnd->set_text_bg(colors::critical_bg);
					_internal::out_wnd->set_text_fg(colors::critical_fg);
					put(str);
					_internal::out_wnd->pop_colors();
				}else
				{
					put(str);
				}

				if (last)
				{
					if (!str.ends_with('\n')) {
						put("\n");
					}
				}
			}

			void server_marshall(const std::wstring& str, log_level lvl, bool first, bool last)
			{
				if (_internal::out_wnd == nullptr) { return; }

				if (first) {
					prefix(lvl);
				}

				if (lvl == critical) {
					_internal::out_wnd->push_colors();
					_internal::out_wnd->set_text_bg(colors::critical_bg);
					_internal::out_wnd->set_text_fg(colors::critical_fg);
					put(str);
					_internal::out_wnd->pop_colors();
				}
				else
				{
					put(str);
				}

				if (last)
				{
					if (!str.ends_with(L'\n')) {
						put(L"\n");
					}
				}
			}


		};

		inline std::shared_ptr<_internal::xout_output> output = nullptr;
	}


	inline bool init()
	{
		_internal::out_wnd = output_window::create_async();
		_internal::output = std::make_shared<_internal::xout_output>();
		_internal::_init_flag = true;
		return true;
	}

	template <class... Ts>
	void log(const std::format_string<Ts...> fmt, Ts&&... args)
	{
		if (!_internal::_init_flag) { init(); }

		std::string str = ::std::vformat(fmt.get(), ::std::make_format_args(args...));
		str.push_back('\n');
		_internal::output->prefix(_internal::log);
		_internal::output->put(str);
	}

	template <class... Ts>
	void log(const std::wformat_string<Ts...> fmt, Ts&&... args)
	{
		if (!_internal::_init_flag) { init(); }

		std::wstring str = ::std::vformat(fmt.get(), ::std::make_wformat_args(args...));
		str.push_back('\n');
		_internal::output->prefix(_internal::log);
		_internal::output->put(str);
	}

	inline void log(const std::string& str)
	{
		if (!_internal::_init_flag) { init(); }
		_internal::output->prefix(_internal::log);
		_internal::output->put(str);
		if (!str.ends_with("\n"))
		{
			_internal::output->put("\n");
		}
	}

	inline void log(const std::wstring& str)
	{
		if (!_internal::_init_flag) { init(); }
		_internal::output->prefix(_internal::log);
		_internal::output->put(str);
		if (!str.ends_with(L"\n"))
		{
			_internal::output->put(L"\n");
		}
	}

	template <class... Ts>
	void info(const std::format_string<Ts...> fmt, Ts&&... args)
	{
		if (!_internal::_init_flag) { init(); }

		std::string str = ::std::vformat(fmt.get(), ::std::make_format_args(args...));
		str.push_back('\n');
		_internal::output->prefix(_internal::info);
		_internal::output->put(str);
	}

	template <class... Ts>
	void info(const std::wformat_string<Ts...> fmt, Ts&&... args)
	{
		if (!_internal::_init_flag) { init(); }

		std::wstring str = ::std::vformat(fmt.get(), ::std::make_wformat_args(args...));
		str.push_back('\n');
		_internal::output->prefix(_internal::info);
		_internal::output->put(str);
	}

	inline void info(const std::string& str)
	{
		if (!_internal::_init_flag) { init(); }
		_internal::output->prefix(_internal::info);
		_internal::output->put(str);
		if (!str.ends_with("\n"))
		{
			_internal::output->put("\n");
		}
	}

	inline void info(const std::wstring& str)
	{
		if (!_internal::_init_flag) { init(); }
		_internal::output->prefix(_internal::info);
		_internal::output->put(str);
		if (!str.ends_with(L"\n"))
		{
			_internal::output->put(L"\n");
		}
	}

	template <class... Ts>
	void debug(const std::format_string<Ts...> fmt, Ts&&... args)
	{
		if (!_internal::_init_flag) { init(); }

		std::string str = ::std::vformat(fmt.get(), ::std::make_format_args(args...));
		str.push_back('\n');
		_internal::output->prefix(_internal::debug);
		_internal::output->put(str);
	}

	template <class... Ts>
	void debug(const std::wformat_string<Ts...> fmt, Ts&&... args)
	{
		if (!_internal::_init_flag) { init(); }

		std::wstring str = ::std::vformat(fmt.get(), ::std::make_wformat_args(args...));
		str.push_back('\n');
		_internal::output->prefix(_internal::debug);
		_internal::output->put(str);
	}

	inline void debug(const std::string& str)
	{
		if (!_internal::_init_flag) { init(); }
		_internal::output->prefix(_internal::debug);
		_internal::output->put(str);
		if (!str.ends_with("\n"))
		{
			_internal::output->put("\n");
		}
	}

	inline void debug(const std::wstring& str)
	{
		if (!_internal::_init_flag) { init(); }
		_internal::output->prefix(_internal::debug);
		_internal::output->put(str);
		if (!str.ends_with(L"\n"))
		{
			_internal::output->put(L"\n");
		}
	}


	template <class... Ts>
	void warning(const std::format_string<Ts...> fmt, Ts&&... args)
	{
		if (!_internal::_init_flag) { init(); }

		std::string str = ::std::vformat(fmt.get(), ::std::make_format_args(args...));
		str.push_back('\n');
		_internal::output->prefix(_internal::warning);
		_internal::output->put(str);
	}

	template <class... Ts>
	void warning(const std::wformat_string<Ts...> fmt, Ts&&... args)
	{
		if (!_internal::_init_flag) { init(); }

		std::wstring str = ::std::vformat(fmt.get(), ::std::make_wformat_args(args...));
		str.push_back('\n');
		_internal::output->prefix(_internal::warning);
		_internal::output->put(str);
	}

	inline void warning(const std::string& str)
	{
		if (!_internal::_init_flag) { init(); }
		_internal::output->prefix(_internal::warning);
		_internal::output->put(str);
		if (!str.ends_with("\n"))
		{
			_internal::output->put("\n");
		}
	}

	inline void warning(const std::wstring& str)
	{
		if (!_internal::_init_flag) { init(); }
		_internal::output->prefix(_internal::warning);
		_internal::output->put(str);
		if (!str.ends_with(L"\n"))
		{
			_internal::output->put(L"\n");
		}
	}

	template <class... Ts>
	void error(const std::format_string<Ts...> fmt, Ts&&... args)
	{
		if (!_internal::_init_flag) { init(); }

		std::string str = ::std::vformat(fmt.get(), ::std::make_format_args(args...));
		str.push_back('\n');
		_internal::output->prefix(_internal::error);
		_internal::output->put(str);
	}

	template <class... Ts>
	void error(const std::wformat_string<Ts...> fmt, Ts&&... args)
	{
		if (!_internal::_init_flag) { init(); }

		std::wstring str = ::std::vformat(fmt.get(), ::std::make_wformat_args(args...));
		str.push_back('\n');
		_internal::output->prefix(_internal::error);
		_internal::output->put(str);
	}

	inline void error(const std::string& str)
	{
		if (!_internal::_init_flag) { init(); }
		_internal::output->prefix(_internal::error);
		_internal::output->put(str);
		if (!str.ends_with("\n"))
		{
			_internal::output->put("\n");
		}
	}

	inline void error(const std::wstring& str)
	{
		if (!_internal::_init_flag) { init(); }
		_internal::output->prefix(_internal::error);
		_internal::output->put(str);
		if (!str.ends_with(L"\n"))
		{
			_internal::output->put(L"\n");
		}
	}


	template <class... Ts>
	void critical(const std::format_string<Ts...> fmt, Ts&&... args)
	{
		if (!_internal::_init_flag) { init(); }

		std::string str = ::std::vformat(fmt.get(), ::std::make_format_args(args...));
		str.push_back('\n');
		_internal::output->prefix(_internal::critical);
		_internal::output->put(str, _internal::colors::critical_bg, _internal::colors::critical_fg);
	}

	template <class... Ts>
	void critical(const std::wformat_string<Ts...> fmt, Ts&&... args)
	{
		if (!_internal::_init_flag) { init(); }
		std::wstring str = ::std::vformat(fmt.get(), ::std::make_wformat_args(args...));
		str.push_back('\n');
		_internal::output->prefix(_internal::critical);
		_internal::output->put(str, _internal::colors::critical_bg, _internal::colors::critical_fg);
	}

	inline void critical(const std::string& str)
	{
		if (!_internal::_init_flag) { init(); }
		_internal::output->prefix(_internal::critical);
		_internal::output->put(str, _internal::colors::critical_bg, _internal::colors::critical_fg);
		if (!str.ends_with("\n"))
		{
			_internal::output->put("\n", _internal::colors::critical_bg, _internal::colors::critical_fg);
		}
	}

	inline void critical(const std::wstring& str)
	{
		if (!_internal::_init_flag) { init(); }
		_internal::output->prefix(_internal::critical);
		_internal::output->put(str, _internal::colors::critical_bg, _internal::colors::critical_fg);
		if (!str.ends_with(L"\n"))
		{
			_internal::output->put(L"\n", _internal::colors::critical_bg, _internal::colors::critical_fg);
		}
	}

	inline void custom(const std::string& str, const std::string& prefix, const uint32_t fg, const uint32_t bg)
	{
		if (!_internal::_init_flag) { init(); }
		_internal::output->prefix_custom(prefix, _internal::colors::hex2colorref(bg), _internal::colors::hex2colorref(fg));
		_internal::output->put(str, _internal::colors::hex2colorref(bg), _internal::colors::hex2colorref(fg));
		if (!str.ends_with("\n"))
		{
			_internal::output->put("\n");
		}
	}

	inline void custom(const std::wstring& str, const std::wstring& prefix, const uint32_t fg, const uint32_t bg)
	{
		if (!_internal::_init_flag) { init(); }
		_internal::output->prefix_custom(prefix, _internal::colors::hex2colorref(bg), _internal::colors::hex2colorref(fg));
		_internal::output->put(str, _internal::colors::hex2colorref(bg), _internal::colors::hex2colorref(fg));
		if (!str.ends_with(L"\n"))
		{
			_internal::output->put(L"\n");
		}
	}


	inline bool should_exit()
	{
		return output_window::ready_to_exit;
	}
	}
