// pak: create and unpack Quake 1 & 2 PAK files

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;

constexpr const char* VERSION = "0.1.0";
constexpr char SIGNATURE[4] = {'P', 'A', 'C', 'K'};
constexpr uint32_t DIR_ENTRY_SIZE = 64;
constexpr size_t FILENAME_LEN = 56;
constexpr uint32_t HEADER_SIZE = 12;

struct PakEntry {
    std::string name;
    uint32_t offset;
    uint32_t len;
};

void usage() {
    std::cout << "pak " << VERSION << " - Quake PAK file utility\n\n";
    std::cout << "USAGE:\n";
    std::cout << "    pak <output.pak> <file>...    Create PAK from files/directories\n";
    std::cout << "    pak extract <file.pak>...     Extract files from PAK\n\n";
    std::cout << "OPTIONS:\n";
    std::cout << "    -h, --help       Print help\n";
    std::cout << "    -V, --version    Print version\n";
}

std::vector<PakEntry> pak_read(std::ifstream& pak) {
    char buf[4];

    pak.read(buf, 4);
    if (!pak || std::memcmp(buf, SIGNATURE, 4) != 0) {
        throw std::runtime_error("invalid signature");
    }

    pak.read(buf, 4);
    uint32_t dir_offset = static_cast<uint32_t>(
        static_cast<unsigned char>(buf[0]) |
        (static_cast<unsigned char>(buf[1]) << 8) |
        (static_cast<unsigned char>(buf[2]) << 16) |
        (static_cast<unsigned char>(buf[3]) << 24)
    );

    pak.read(buf, 4);
    uint32_t dir_len = static_cast<uint32_t>(
        static_cast<unsigned char>(buf[0]) |
        (static_cast<unsigned char>(buf[1]) << 8) |
        (static_cast<unsigned char>(buf[2]) << 16) |
        (static_cast<unsigned char>(buf[3]) << 24)
    );

    if (dir_len % DIR_ENTRY_SIZE != 0) {
        throw std::runtime_error("invalid directory");
    }

    pak.seekg(dir_offset);
    if (!pak) {
        throw std::runtime_error("failed to seek to directory");
    }

    std::vector<PakEntry> entries;
    entries.reserve(dir_len / DIR_ENTRY_SIZE);

    for (uint32_t i = 0; i < dir_len / DIR_ENTRY_SIZE; ++i) {
        char name_buf[FILENAME_LEN];
        pak.read(name_buf, FILENAME_LEN);

        size_t name_end = 0;
        while (name_end < FILENAME_LEN && name_buf[name_end] != '\0') {
            ++name_end;
        }
        std::string name(name_buf, name_end);

        pak.read(buf, 4);
        uint32_t offset = static_cast<uint32_t>(
            static_cast<unsigned char>(buf[0]) |
            (static_cast<unsigned char>(buf[1]) << 8) |
            (static_cast<unsigned char>(buf[2]) << 16) |
            (static_cast<unsigned char>(buf[3]) << 24)
        );

        pak.read(buf, 4);
        uint32_t len = static_cast<uint32_t>(
            static_cast<unsigned char>(buf[0]) |
            (static_cast<unsigned char>(buf[1]) << 8) |
            (static_cast<unsigned char>(buf[2]) << 16) |
            (static_cast<unsigned char>(buf[3]) << 24)
        );

        if (!pak) {
            throw std::runtime_error("failed to read directory entry");
        }

        entries.push_back({std::move(name), offset, len});
    }

    return entries;
}

void write_u32_le(std::ostream& out, uint32_t value) {
    char buf[4] = {
        static_cast<char>(value & 0xFF),
        static_cast<char>((value >> 8) & 0xFF),
        static_cast<char>((value >> 16) & 0xFF),
        static_cast<char>((value >> 24) & 0xFF)
    };
    out.write(buf, 4);
}

void pak_write(std::ofstream& pak, const std::vector<std::pair<std::string, uint32_t>>& files) {
    uint32_t total_size = 0;
    for (const auto& [path, len] : files) {
        total_size += len;
    }

    uint32_t dir_offset = HEADER_SIZE + total_size;
    uint32_t dir_len = static_cast<uint32_t>(files.size()) * DIR_ENTRY_SIZE;

    pak.write(SIGNATURE, 4);
    write_u32_le(pak, dir_offset);
    write_u32_le(pak, dir_len);

    // Write file contents
    for (const auto& [path, len] : files) {
        std::ifstream infile(path, std::ios::binary);
        if (!infile) {
            throw std::runtime_error("failed to open file: " + path);
        }

        constexpr size_t BUFFER_SIZE = 8192;
        char buffer[BUFFER_SIZE];
        while (infile) {
            infile.read(buffer, BUFFER_SIZE);
            if (infile.gcount() > 0) {
                pak.write(buffer, infile.gcount());
            }
        }
    }

    // Write directory
    uint32_t pos = HEADER_SIZE;
    for (const auto& [path, len] : files) {
        std::string name = path;

        // Strip leading '/'
        while (!name.empty() && name[0] == '/') {
            name = name.substr(1);
        }

        // Strip leading './'
        while (name.size() >= 2 && name[0] == '.' && name[1] == '/') {
            name = name.substr(2);
        }

        char entry[DIR_ENTRY_SIZE] = {0};
        size_t write_len = std::min(name.size(), FILENAME_LEN - 1);
        std::memcpy(entry, name.c_str(), write_len);

        entry[FILENAME_LEN] = static_cast<char>(pos & 0xFF);
        entry[FILENAME_LEN + 1] = static_cast<char>((pos >> 8) & 0xFF);
        entry[FILENAME_LEN + 2] = static_cast<char>((pos >> 16) & 0xFF);
        entry[FILENAME_LEN + 3] = static_cast<char>((pos >> 24) & 0xFF);

        entry[FILENAME_LEN + 4] = static_cast<char>(len & 0xFF);
        entry[FILENAME_LEN + 5] = static_cast<char>((len >> 8) & 0xFF);
        entry[FILENAME_LEN + 6] = static_cast<char>((len >> 16) & 0xFF);
        entry[FILENAME_LEN + 7] = static_cast<char>((len >> 24) & 0xFF);

        pak.write(entry, DIR_ENTRY_SIZE);
        pos += len;
    }

    if (!pak) {
        throw std::runtime_error("failed to write PAK file");
    }
}

void collect_files(const fs::path& path, std::vector<std::pair<std::string, uint32_t>>& files) {
    if (fs::is_regular_file(path)) {
        auto len = static_cast<uint32_t>(fs::file_size(path));
        files.emplace_back(path.string(), len);
    } else if (fs::is_directory(path)) {
        std::vector<fs::directory_entry> entries;
        for (const auto& entry : fs::directory_iterator(path)) {
            entries.push_back(entry);
        }

        std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b) {
            return a.path() < b.path();
        });

        for (const auto& entry : entries) {
            collect_files(entry.path(), files);
        }
    }
}

int cmd_extract(int argc, char* argv[]) {
    if (argc < 1) {
        std::cerr << "usage: pak extract <file.pak>...\n";
        return 1;
    }

    for (int i = 0; i < argc; ++i) {
        const char* pak_path = argv[i];
        std::ifstream pak(pak_path, std::ios::binary);
        if (!pak) {
            std::cerr << "error: failed to open " << pak_path << "\n";
            return 1;
        }

        auto entries = pak_read(pak);

        for (const auto& entry : entries) {
            fs::path filepath(entry.name);
            if (filepath.has_parent_path()) {
                fs::create_directories(filepath.parent_path());
            }

            pak.seekg(entry.offset);
            std::ofstream outfile(entry.name, std::ios::binary);
            if (!outfile) {
                std::cerr << "error: failed to create " << entry.name << "\n";
                return 1;
            }

            constexpr size_t BUFFER_SIZE = 8192;
            char buffer[BUFFER_SIZE];
            uint32_t remaining = entry.len;
            while (remaining > 0) {
                size_t to_read = std::min(static_cast<size_t>(remaining), BUFFER_SIZE);
                pak.read(buffer, to_read);
                outfile.write(buffer, pak.gcount());
                remaining -= static_cast<uint32_t>(pak.gcount());
            }

            std::cout << "  " << entry.name << "\n";
        }
    }

    return 0;
}

int cmd_create(const char* pak_path, int argc, char* argv[]) {
    if (argc < 1) {
        std::cerr << "usage: pak <output.pak> <file>...\n";
        return 1;
    }

    std::vector<std::pair<std::string, uint32_t>> files;
    for (int i = 0; i < argc; ++i) {
        collect_files(fs::path(argv[i]), files);
    }

    if (files.empty()) {
        std::cerr << "error: no files to pack\n";
        return 1;
    }

    std::ofstream pak(pak_path, std::ios::binary);
    if (!pak) {
        std::cerr << "error: failed to create " << pak_path << "\n";
        return 1;
    }

    pak_write(pak, files);
    std::cout << "created '" << pak_path << "' (" << files.size() << " files)\n";

    return 0;
}

int main(int argc, char* argv[]) {
    try {
        if (argc < 2) {
            usage();
            return 1;
        }

        std::string cmd = argv[1];

        if (cmd == "extract") {
            return cmd_extract(argc - 2, argv + 2);
        } else if (cmd == "--version" || cmd == "-V") {
            std::cout << "pak " << VERSION << "\n";
            return 0;
        } else if (cmd == "--help" || cmd == "-h" || cmd == "help") {
            usage();
            return 0;
        } else if (cmd[0] != '-') {
            return cmd_create(argv[1], argc - 2, argv + 2);
        } else {
            usage();
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
}
