//
// Copyright RIME Developers
// Distributed under the BSD License
//
// register components
//
// 2011-06-30 GONG Chen <chen.sst@gmail.com>
//
#include <fstream>
#include <filesystem>
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif
#include <rime/dict/mapped_file.h>

namespace rime {

// Minimal boost::interprocess::{file_mapping, mapped_region} replacement.
// Provides the subset mapped_file.cc needs: open a file, map it read-only or
// read-write, expose address/size, flush, and remove — POSIX mmap below the
// fold, CreateFileMapping/MapViewOfFile on Windows.
class MappedFileImpl {
 public:
  enum OpenMode {
    kOpenReadOnly,
    kOpenReadWrite,
  };

  MappedFileImpl(const path& file_path, OpenMode mode) {
#ifdef _WIN32
    HANDLE file = ::CreateFileW(
        file_path.c_str(),
        (mode == kOpenReadOnly) ? GENERIC_READ : GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
        nullptr);
    if (file == INVALID_HANDLE_VALUE)
      return;
    file_ = file;
    LARGE_INTEGER file_size{};
    if (!::GetFileSizeEx(file, &file_size)) {
      ::CloseHandle(file);
      file_ = nullptr;
      return;
    }
    size_ = static_cast<size_t>(file_size.QuadPart);
    if (size_ > 0) {
      // A mapping of zero bytes is invalid on Windows; skip it, as the POSIX
      // path does for empty files.
      HANDLE mapping = ::CreateFileMappingW(
          file, nullptr,
          (mode == kOpenReadOnly) ? PAGE_READONLY : PAGE_READWRITE, 0, 0,
          nullptr);
      if (!mapping) {
        ::CloseHandle(file);
        file_ = nullptr;
        size_ = 0;
        return;
      }
      mapping_ = mapping;
      addr_ = ::MapViewOfFile(mapping,
                              (mode == kOpenReadOnly) ? FILE_MAP_READ
                                                      : FILE_MAP_WRITE,
                              0, 0, 0);
      if (!addr_) {
        ::CloseHandle(mapping);
        ::CloseHandle(file);
        mapping_ = nullptr;
        file_ = nullptr;
        size_ = 0;
      }
    }
#else
    int flags = (mode == kOpenReadOnly) ? O_RDONLY : O_RDWR;
    fd_ = ::open(file_path.c_str(), flags);
    if (fd_ < 0) return;
    struct stat st;
    if (::fstat(fd_, &st) != 0) {
      ::close(fd_);
      fd_ = -1;
      return;
    }
    size_ = static_cast<size_t>(st.st_size);
    int prot = (mode == kOpenReadOnly) ? PROT_READ : (PROT_READ | PROT_WRITE);
    void* addr = nullptr;
    if (size_ > 0) {
      addr = ::mmap(nullptr, size_, prot, MAP_SHARED, fd_, 0);
      if (addr == MAP_FAILED) {
        addr = nullptr;
        size_ = 0;
      }
    }
    addr_ = addr;
#endif
  }
  ~MappedFileImpl() {
#ifdef _WIN32
    if (addr_) ::UnmapViewOfFile(addr_);
    if (mapping_) ::CloseHandle(mapping_);
    if (file_) ::CloseHandle(file_);
#else
    if (addr_) ::munmap(addr_, size_);
    if (fd_ >= 0) ::close(fd_);
#endif
  }
  bool Flush() {
    if (!addr_) return false;
#ifdef _WIN32
    // FlushViewOfFile writes the dirty pages of the mapped view back to the
    // file — the analog of msync(MS_SYNC). (FlushFileBuffers is deliberately
    // not used: it fails on a read-only handle.)
    return ::FlushViewOfFile(addr_, size_) != 0;
#else
    return ::msync(addr_, size_, MS_SYNC) == 0;
#endif
  }
  void* get_address() const { return addr_; }
  size_t get_size() const { return size_; }

  // Replaces boost::interprocess::file_mapping::remove.
  static bool remove(const path& file_path) {
#ifdef _WIN32
    return ::DeleteFileW(file_path.c_str()) != 0;
#else
    return ::unlink(file_path.c_str()) == 0;
#endif
  }

 private:
#ifdef _WIN32
  void* file_ = nullptr;     // HANDLE
  void* mapping_ = nullptr;  // HANDLE
#else
  int fd_ = -1;
#endif
  void* addr_ = nullptr;
  size_t size_ = 0;
};

MappedFile::MappedFile(const path& file_path) : file_path_(file_path) {}

MappedFile::~MappedFile() {
  if (file_) {
    file_.reset();
  }
}

bool MappedFile::Create(size_t capacity) {
  if (Exists()) {
    LOG(INFO) << "overwriting file '" << file_path_ << "'.";
    Resize(capacity);
  } else {
    LOG(INFO) << "creating file '" << file_path_ << "'.";
    std::filebuf fbuf;
    fbuf.open(file_path_.c_str(), std::ios_base::in | std::ios_base::out |
                                      std::ios_base::trunc |
                                      std::ios_base::binary);
    if (capacity > 0) {
      fbuf.pubseekoff(capacity - 1, std::ios_base::beg);
      fbuf.sputc(0);
    }
    fbuf.close();
  }
  LOG(INFO) << "opening file for read/write access.";
  file_.reset(new MappedFileImpl(file_path_, MappedFileImpl::kOpenReadWrite));
  size_ = 0;
  return bool(file_);
}

bool MappedFile::OpenReadOnly() {
  if (!Exists()) {
    LOG(ERROR) << "attempt to open non-existent file '" << file_path_ << "'.";
    return false;
  }
  file_.reset(new MappedFileImpl(file_path_, MappedFileImpl::kOpenReadOnly));
  size_ = file_->get_size();
  return bool(file_);
}

bool MappedFile::OpenReadWrite() {
  if (!Exists()) {
    LOG(ERROR) << "attempt to open non-existent file '" << file_path_ << "'.";
    return false;
  }
  file_.reset(new MappedFileImpl(file_path_, MappedFileImpl::kOpenReadWrite));
  size_ = 0;
  return bool(file_);
}

void MappedFile::Close() {
  if (file_) {
    file_.reset();
    size_ = 0;
  }
}

bool MappedFile::Exists() const {
  return std::filesystem::exists(file_path_);
}

bool MappedFile::IsOpen() const {
  return bool(file_);
}

bool MappedFile::Flush() {
  if (!file_)
    return false;
  return file_->Flush();
}

bool MappedFile::ShrinkToFit() {
  LOG(INFO) << "shrinking file to fit data size. capacity: " << capacity();
  return Resize(size_);
}

bool MappedFile::Remove() {
  if (IsOpen())
    Close();
  return MappedFileImpl::remove(file_path_);
}

bool MappedFile::Resize(size_t capacity) {
  LOG(INFO) << "resize file to: " << capacity;
  if (IsOpen())
    Close();
  try {
    std::filesystem::resize_file(file_path_, capacity);
  } catch (...) {
    return false;
  }
  return true;
}

String* MappedFile::CreateString(const string& str) {
  String* ret = Allocate<String>();
  if (ret && !str.empty()) {
    CopyString(str, ret);
  }
  return ret;
}

bool MappedFile::CopyString(const string& src, String* dest) {
  if (!dest)
    return false;
  size_t size = src.length() + 1;
  char* ptr = Allocate<char>(size);
  if (!ptr)
    return false;
  std::strncpy(ptr, src.c_str(), size);
  dest->data = ptr;
  return true;
}

size_t MappedFile::capacity() const {
  return file_->get_size();
}

char* MappedFile::address() const {
  return reinterpret_cast<char*>(file_->get_address());
}

}  // namespace rime
