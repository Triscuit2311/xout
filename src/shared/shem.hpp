/*
* Author: Dante Trisciuzzi (Triscuit2311)
* Updates: 2025-02-06
* License: MIT License (see LICENSE file)
*/

#pragma once

#include <windows.h>
#include <vector>
#include <string>
#include <mutex>
#include <stdexcept>

namespace shem {
    template <typename T>
    class shared_memory_obj {
    private:
        std::wstring name_;
        size_t size_;
        HANDLE h_map_file_;
        T* p_buf_;
        std::mutex mutex_;
    public:
        shared_memory_obj(const std::wstring& name) : name_(name), size_(sizeof(T)){
            h_map_file_ = CreateFileMappingW(
                INVALID_HANDLE_VALUE, nullptr,
                PAGE_READWRITE, 0, 
                size_, name.c_str());
            if (h_map_file_ == nullptr) {
                throw std::runtime_error("could not create file mapping object.");
            }
            p_buf_ = static_cast<T*>(MapViewOfFile(h_map_file_, FILE_MAP_ALL_ACCESS, 0, 0, size_));
            if (p_buf_ == nullptr) {
                CloseHandle(h_map_file_);
                throw std::runtime_error("could not map view of file.");
            }
        }
        ~shared_memory_obj() {
            UnmapViewOfFile(p_buf_);
            CloseHandle(h_map_file_);
        }

        bool write(T* v){
            size_t sz = sizeof(T);
            if (sz > size_)
            {
                throw std::runtime_error("struct size mismatch");
            }
            memcpy(p_buf_, v, sz);
            return true;
        }

        bool read(T* v){
            size_t sz = sizeof(T);
            memcpy(v, p_buf_, sz);
            return true;
        }
    };
}