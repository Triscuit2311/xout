#pragma once
#include <cstdint>
#include <format>
#include <future>
#include <queue>

#include "..\shared\shem.hpp"

namespace xout
{
	namespace _internal
	{
		enum log_level : uint8_t
		{
			log,
			info,
			debug,
			warning,
			error,
			critical,
		};
		namespace model
		{
			struct msg
			{
				char str[255] = "";
				size_t len = 0;
				log_level level;
				bool written = true;
				bool is_first = true;
				bool is_last = true;
				size_t id;
				explicit msg(const std::string& s, const size_t id, const log_level lvl = log_level::log) : level(lvl), id(id)
				{
					len = s.size();
					std::memcpy(str, s.data(), len);
					str[len] = '\0';
					written = false;
				}
				explicit msg(const std::string& s, const size_t id, bool first,
					bool last, const log_level lvl = log_level::log) : level(lvl), id(id), is_first(first), is_last(last)
				{
					len = s.size();
					std::memcpy(str, s.data(), len);
					str[len] = '\0';
					written = false;
				}
				void operator==(const msg& rhs)
				{
					strcpy_s(str, 255, rhs.str);
					level = rhs.level;
					written = rhs.written;
					len = rhs.len;
				}
			} inline g_msg("", 0);

			struct wmsg
			{
				wchar_t str[255] = L"";
				size_t len = 0;
				log_level level;
				bool written = true;
				bool is_first = true;
				bool is_last = true;
				size_t id;
				explicit wmsg(const std::wstring& s, const size_t id, const log_level lvl = log_level::log) : level(lvl), id(id)
				{
					len = s.size();
					wcsncpy_s(str, s.data(), len);
					str[len] = '\0';
					written = false;
				}
				explicit wmsg(const std::wstring& s, const size_t id, bool first,
					bool last, const log_level lvl = log_level::log) : level(lvl), id(id), is_first(first), is_last(last)
				{
					len = s.size();
					wcsncpy_s(str, s.data(), len);
					str[len] = '\0';
					written = false;
				}
				void operator==(const wmsg& rhs)
				{
					wcsncpy_s(str, rhs.str, 255);
					level = rhs.level;
					written = rhs.written;
					len = rhs.len;
				}
			} inline g_wmsg(L"", 0);

			struct shm_mutex
			{
				bool locked = false;
			};
			inline shm_mutex g_client_mutex;
			inline shm_mutex g_server_mutex;
		}

		inline std::shared_ptr<shem::shared_memory_obj<model::msg>> shm_msg;
		inline std::shared_ptr<shem::shared_memory_obj<model::wmsg>> shm_wmsg;
		inline std::shared_ptr<shem::shared_memory_obj<model::shm_mutex>> shm_client_mut;
		inline std::shared_ptr<shem::shared_memory_obj<model::shm_mutex>> shm_server_mut;
		inline bool _init_flag = false;
		inline size_t msgno = 0;
		inline std::shared_ptr<std::queue<model::msg>> msgs{};
		inline std::shared_ptr<std::queue<model::wmsg>> wmsgs{};

		inline std::atomic<bool> stop_requested{ false };
		inline std::jthread worker_thread;

		inline void init()
		{
			shm_msg = std::make_shared<shem::shared_memory_obj<model::msg>>(L"SHM_MSG");
			shm_wmsg = std::make_shared<shem::shared_memory_obj<model::wmsg>>(L"SHM_WMSG");
			shm_client_mut = std::make_shared<shem::shared_memory_obj<model::shm_mutex>>(L"SHM_CLIENT_MUTEX");
			shm_server_mut = std::make_shared<shem::shared_memory_obj<model::shm_mutex>>(L"SHM_SERVER_MUTEX");
			msgs = std::make_shared<std::queue<model::msg>>();
			wmsgs = std::make_shared<std::queue<model::wmsg>>();
			_init_flag = true;
		}

		inline void msg_queue_loop()
		{

			while (true)
			{
				// do nothing while waiting for server
				while (true) {
					_internal::shm_server_mut->read(&model::g_server_mutex);
					if (!model::g_server_mutex.locked) {
						break;
					}
				}
			
				model::g_client_mutex.locked = true;
				_internal::shm_client_mut->write(&model::g_client_mutex);

				// Send messages in order
				if (!msgs->empty() && !wmsgs->empty())
				{
					if (msgs->front().id < wmsgs->front().id)
					{
						model::g_msg = msgs->front();
						msgs->pop();
						_internal::shm_msg->write(&xout::_internal::model::g_msg);
					}else
					{
						model::g_wmsg = wmsgs->front();
						wmsgs->pop();
						_internal::shm_wmsg->write(&xout::_internal::model::g_wmsg);
					}
				}else if (!msgs->empty())
				{
					model::g_msg = msgs->front();
					msgs->pop();
					_internal::shm_msg->write(&xout::_internal::model::g_msg);
				}else if (!wmsgs->empty())
				{
					model::g_wmsg = wmsgs->front();
					wmsgs->pop();
					_internal::shm_wmsg->write(&xout::_internal::model::g_wmsg);
				}
				

				model::g_client_mutex.locked = false;
				_internal::shm_client_mut->write(&model::g_client_mutex);

				// Wait for server to read last message
				while (true) {
					_internal::shm_server_mut->read(&model::g_server_mutex);
					if (model::g_server_mutex.locked) {
						break;
					}
				}
			}

		}

		inline void send_msg(const std::string& s, log_level lvl)
		{
			if (s.length() > 255)
			{
				size_t its = (s.length() + 254) / 255;
				size_t last_sz = (s.length() + 1) % 255;
				for (size_t i = 0; i < its; ++i)
				{
					size_t ct = (i == its - 1 && last_sz > 0) ? last_sz : 255;
					_internal::msgs->emplace(s.substr(i * 255, ct), _internal::msgno,
						i == 0,
						i == (its - 1),
						lvl);
				}
			}
			else
			{
				_internal::msgs->emplace(s, _internal::msgno, lvl);
			}
			_internal::msgno++;
		}

		inline void send_msg(std::wstring ws, log_level lvl)
		{
			if (ws.length() > 255)
			{
				size_t its = (ws.length() + 254) / 255;
				size_t last_sz = (ws.length() + 1) % 255;
				for (size_t i = 0; i < its; ++i)
				{
					size_t ct = (i == its - 1 && last_sz > 0) ? last_sz : 255;
					_internal::wmsgs->emplace(ws.substr(i * 255, ct), _internal::msgno,
						i == 0,
						i == (its - 1),
						lvl);
				}
			}
			else
			{
				_internal::wmsgs->emplace(ws, _internal::msgno, lvl);
			}
			_internal::msgno++;
		}


	}


	inline void broadcast()
	{
		if (!_internal::_init_flag) { _internal::init(); }
		_internal::stop_requested.store(false);
		_internal::worker_thread = std::jthread(_internal::msg_queue_loop);
	}


	template <class... Ts>
	void log(const std::format_string<Ts...> fmt, Ts&&... args){
		if (!_internal::_init_flag) { _internal::init(); }

		std::string str = ::std::vformat(fmt.get(), ::std::make_format_args(args...));
		str.push_back('\n');
		_internal::send_msg(str, _internal::log);
	}

	template <class... Ts>
	void log(const std::wformat_string<Ts...> fmt, Ts&&... args){
		if (!_internal::_init_flag) { _internal::init(); }

		std::wstring str = ::std::vformat(fmt.get(), ::std::make_wformat_args(args...));
		str.push_back('\n');
		_internal::send_msg(str, _internal::log);
	}

	inline void log(const std::string& str){
		if (!_internal::_init_flag) { _internal::init(); }
		_internal::send_msg(str + ( str.ends_with("\n") ? "" : "\n"), _internal::log);
	}

	inline void log(const std::wstring& str){
		if (!_internal::_init_flag) { _internal::init(); }
		_internal::send_msg(str + (str.ends_with(L"\n") ? L"" : L"\n"), _internal::log);
	}

	template <class... Ts>
	void info(const std::format_string<Ts...> fmt, Ts&&... args) {
		if (!_internal::_init_flag) { _internal::init(); }

		std::string str = ::std::vformat(fmt.get(), ::std::make_format_args(args...));
		str.push_back('\n');
		_internal::send_msg(str, _internal::info);
	}

	template <class... Ts>
	void info(const std::wformat_string<Ts...> fmt, Ts&&... args) {
		if (!_internal::_init_flag) { _internal::init(); }

		std::wstring str = ::std::vformat(fmt.get(), ::std::make_wformat_args(args...));
		str.push_back('\n');
		_internal::send_msg(str, _internal::info);
	}

	inline void info(const std::string& str) {
		if (!_internal::_init_flag) { _internal::init(); }
		_internal::send_msg(str + (str.ends_with("\n") ? "" : "\n"), _internal::info);
	}

	inline void info(const std::wstring& str) {
		if (!_internal::_init_flag) { _internal::init(); }
		_internal::send_msg(str + (str.ends_with(L"\n") ? L"" : L"\n"), _internal::info);
	}


	template <class... Ts>
	void debug(const std::format_string<Ts...> fmt, Ts&&... args) {
		if (!_internal::_init_flag) { _internal::init(); }

		std::string str = ::std::vformat(fmt.get(), ::std::make_format_args(args...));
		str.push_back('\n');
		_internal::send_msg(str, _internal::debug);
	}

	template <class... Ts>
	void debug(const std::wformat_string<Ts...> fmt, Ts&&... args) {
		if (!_internal::_init_flag) { _internal::init(); }

		std::wstring str = ::std::vformat(fmt.get(), ::std::make_wformat_args(args...));
		str.push_back('\n');
		_internal::send_msg(str, _internal::debug);
	}

	inline void debug(const std::string& str) {
		if (!_internal::_init_flag) { _internal::init(); }
		_internal::send_msg(str + (str.ends_with("\n") ? "" : "\n"), _internal::debug);
	}

	inline void debug(const std::wstring& str) {
		if (!_internal::_init_flag) { _internal::init(); }
		_internal::send_msg(str + (str.ends_with(L"\n") ? L"" : L"\n"), _internal::debug);
	}



	template <class... Ts>
	void warning(const std::format_string<Ts...> fmt, Ts&&... args) {
		if (!_internal::_init_flag) { _internal::init(); }

		std::string str = ::std::vformat(fmt.get(), ::std::make_format_args(args...));
		str.push_back('\n');
		_internal::send_msg(str, _internal::warning);
	}

	template <class... Ts>
	void warning(const std::wformat_string<Ts...> fmt, Ts&&... args) {
		if (!_internal::_init_flag) { _internal::init(); }

		std::wstring str = ::std::vformat(fmt.get(), ::std::make_wformat_args(args...));
		str.push_back('\n');
		_internal::send_msg(str, _internal::warning);
	}

	inline void warning(const std::string& str) {
		if (!_internal::_init_flag) { _internal::init(); }
		_internal::send_msg(str + (str.ends_with("\n") ? "" : "\n"), _internal::warning);
	}

	inline void warning(const std::wstring& str) {
		if (!_internal::_init_flag) { _internal::init(); }
		_internal::send_msg(str + (str.ends_with(L"\n") ? L"" : L"\n"), _internal::warning);
	}

	template <class... Ts>
	void error(const std::format_string<Ts...> fmt, Ts&&... args) {
		if (!_internal::_init_flag) { _internal::init(); }

		std::string str = ::std::vformat(fmt.get(), ::std::make_format_args(args...));
		str.push_back('\n');
		_internal::send_msg(str, _internal::error);
	}

	template <class... Ts>
	void error(const std::wformat_string<Ts...> fmt, Ts&&... args) {
		if (!_internal::_init_flag) { _internal::init(); }

		std::wstring str = ::std::vformat(fmt.get(), ::std::make_wformat_args(args...));
		str.push_back('\n');
		_internal::send_msg(str, _internal::error);
	}

	inline void error(const std::string& str) {
		if (!_internal::_init_flag) { _internal::init(); }
		_internal::send_msg(str + (str.ends_with("\n") ? "" : "\n"), _internal::error);
	}

	inline void error(const std::wstring& str) {
		if (!_internal::_init_flag) { _internal::init(); }
		_internal::send_msg(str + (str.ends_with(L"\n") ? L"" : L"\n"), _internal::error);
	}


	template <class... Ts>
	void critical(const std::format_string<Ts...> fmt, Ts&&... args) {
		if (!_internal::_init_flag) { _internal::init(); }

		std::string str = ::std::vformat(fmt.get(), ::std::make_format_args(args...));
		str.push_back('\n');
		_internal::send_msg(str, _internal::critical);
	}

	template <class... Ts>
	void critical(const std::wformat_string<Ts...> fmt, Ts&&... args) {
		if (!_internal::_init_flag) { _internal::init(); }

		std::wstring str = ::std::vformat(fmt.get(), ::std::make_wformat_args(args...));
		str.push_back('\n');
		_internal::send_msg(str, _internal::critical);
	}

	inline void critical(const std::string& str) {
		if (!_internal::_init_flag) { _internal::init(); }
		_internal::send_msg(str + (str.ends_with("\n") ? "" : "\n"), _internal::critical);
	}

	inline void critical(const std::wstring& str) {
		if (!_internal::_init_flag) { _internal::init(); }
		_internal::send_msg(str + (str.ends_with(L"\n") ? L"" : L"\n"), _internal::critical);
	}
}
