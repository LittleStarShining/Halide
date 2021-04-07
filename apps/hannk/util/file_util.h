#ifndef HANNK_FILE_UTIL_H
#define HANNK_FILE_UTIL_H

#include <fstream>
#include <memory>
#include <vector>

#include "util/error_util.h"

namespace hannk {

inline std::vector<char> read_entire_file(const std::string &filename) {
    std::ifstream f(filename, std::ios::in | std::ios::binary);
    CHECK(f.is_open()) << "Unable to open file: " << filename;

    std::vector<char> result;

    f.seekg(0, std::ifstream::end);
    size_t size = f.tellg();
    result.resize(size);
    f.seekg(0, std::ifstream::beg);
    f.read(result.data(), result.size());
    CHECK(f.good()) << "Unable to read file: " << filename;
    f.close();
    return result;
}

inline void write_entire_file(const std::string &filename, const void *source, size_t source_len) {
    std::ofstream f(filename, std::ios::out | std::ios::binary);
    CHECK(f.is_open()) << "Unable to open file: " << filename;

    f.write(reinterpret_cast<const char *>(source), source_len);
    f.flush();
    CHECK(f.good()) << "Unable to write file: " << filename;
    f.close();
}

inline void write_entire_file(const std::string &filename, const std::vector<char> &source) {
    write_entire_file(filename, source.data(), source.size());
}

}  // namespace hannk

#endif  // HANNK_FILE_UTIL_H
