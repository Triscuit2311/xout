#pragma once
#include "xout.hpp"
#include "../test_client/xout_client.hpp"

namespace xout
{
	namespace server
	{
		namespace _internal {

			namespace model
			{
				struct msg
				{
					char str[255] = "";
					size_t len = 0;
					xout::_internal::log_level level;
					bool written = true;
					bool is_first = true;
					bool is_last = true;
					size_t id;
					explicit msg(const std::string& s, const size_t id, const xout::_internal::log_level lvl = xout::_internal::log_level::log) : level(lvl), id(id)
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
					xout::_internal::log_level level;
					bool written = true;
					bool is_first = true;
					bool is_last = true;
					size_t id;
					explicit wmsg(const std::wstring& s, const size_t id, const xout::_internal::log_level lvl = xout::_internal::log_level::log) : level(lvl), id(id)
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
			inline bool _init_flag = false;
			inline std::shared_ptr<shem::shared_memory_obj<model::msg>> shm_msg;
			inline std::shared_ptr<shem::shared_memory_obj<model::wmsg>> shm_wmsg;
			inline std::shared_ptr<shem::shared_memory_obj<model::shm_mutex>> shm_client_mut;
			inline std::shared_ptr<shem::shared_memory_obj<model::shm_mutex>> shm_server_mut;
			inline std::shared_ptr<std::queue<model::msg>> msgs{};
			inline std::shared_ptr<std::queue<model::wmsg>> wmsgs{};
			inline void init_shm()
			{
				shm_msg = std::make_shared<shem::shared_memory_obj<model::msg>>(L"SHM_MSG");
				shm_wmsg = std::make_shared<shem::shared_memory_obj<model::wmsg>>(L"SHM_WMSG");
				shm_client_mut = std::make_shared<shem::shared_memory_obj<model::shm_mutex>>(L"SHM_CLIENT_MUTEX");
				shm_server_mut = std::make_shared<shem::shared_memory_obj<model::shm_mutex>>(L"SHM_SERVER_MUTEX");
				msgs = std::make_shared<std::queue<model::msg>>();
				wmsgs = std::make_shared<std::queue<model::wmsg>>();
				server::_internal::_init_flag = true;
			}
		}

		inline void listen(const size_t interval_in_ms)
		{
			if (!xout::_internal::_init_flag) { xout::_internal::init(); }
			if (!server::_internal::_init_flag) { _internal::init_shm(); }

			while (!xout::should_exit()) {
				std::this_thread::sleep_for(std::chrono::milliseconds(interval_in_ms));

				while (true)
				{
					_internal::shm_client_mut->read(&_internal::model::g_client_mutex);
					if (!_internal::model::g_client_mutex.locked) { break; }
				}

				_internal::model::g_server_mutex.locked = true;
				_internal::shm_server_mut->write(&_internal::model::g_server_mutex);

				static auto send_msg = []()
					{
						if (_internal::model::g_msg.len == 0) { return; }
						xout::_internal::output->server_marshall(
							std::string(_internal::model::g_msg.str), _internal::model::g_msg.level,
							_internal::model::g_msg.is_first, _internal::model::g_msg.is_last);
						_internal::model::g_msg.written = true;
						_internal::model::g_msg.len = 0;
						_internal::shm_msg->write(&_internal::model::g_msg);
					};

				static auto send_wmsg = []()
					{
						if (_internal::model::g_wmsg.len == 0) { return; }
						xout::_internal::output->server_marshall(std::wstring(_internal::model::g_wmsg.str), _internal::model::g_wmsg.level,
							_internal::model::g_wmsg.is_first, _internal::model::g_wmsg.is_last);
						_internal::model::g_wmsg.written = true;
						_internal::model::g_wmsg.len = 0;
						_internal::shm_wmsg->write(&_internal::model::g_wmsg);
					};


				_internal::shm_msg->read(&_internal::model::g_msg);
				_internal::shm_wmsg->read(&_internal::model::g_wmsg);

				if (!_internal::model::g_msg.written && !_internal::model::g_wmsg.written) {

					if (_internal::model::g_msg.id < _internal::model::g_wmsg.id) {
						send_msg();
						send_wmsg();
					}
					else {
						send_wmsg();
						send_msg();
					}
				}
				else if (!_internal::model::g_msg.written) {
					send_msg();
				}
				else if (!_internal::model::g_wmsg.written) {
					send_wmsg();
				}

				_internal::model::g_server_mutex.locked = false;
				_internal::shm_server_mut->write(&_internal::model::g_server_mutex);

			}
		}
	}
}
