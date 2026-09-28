// Component-only harness: actual pinned save implementation, no game or renderer.
#include "../build-deps/lib/N64ModernRuntime/librecomp/src/pi.cpp"
#include <cassert>
#include <iostream>

std::filesystem::path config_path;
namespace ultramodern::error_handling {
void message_box(const char* message) { throw std::runtime_error(message); }
}

int main(int argc, char** argv) {
    assert(argc == 2);
    config_path = std::filesystem::absolute(argv[1]); // Disposable test copy only.
    set_save_file_path(u8"", u8"dora3.n64.jp");
    assert(ultramodern::get_save_file_path() == config_path / "saves/dora3.n64.jp.bin");
    std::ifstream source(ultramodern::get_save_file_path(), std::ios::binary);
    std::vector<char> original{std::istreambuf_iterator<char>(source), {}};
    assert(original.size() == get_save_size(recomp::SaveType::Eep16k));
    save_context.save_buffer.resize(original.size());
    read_save_file();
    assert(save_context.save_buffer == original);
    std::vector<uint8_t> memory(0x400000);
    uint8_t* rdram = memory.data();
    constexpr gpr address = (int32_t)0x80001000;
    save_read(rdram, (int32_t)address, 0, original.size());
    for (size_t i = 0; i < original.size(); ++i)
        assert((uint8_t)MEM_B(i, address) == (uint8_t)original[i]);
    // Exercise writes, backup and reopening using only this disposable file.
    for (size_t i = 0; i < 8; ++i) MEM_B(i, address) = 0xA0 + i;
    save_write(rdram, (int32_t)address, 8, 8);
    update_save_file();
    auto expected = original;
    for (size_t i = 0; i < 8; ++i) expected[8+i] = char(0xA0 + i);
    save_context.save_buffer.assign(original.size(), 0);
    read_save_file();
    assert(save_context.save_buffer == expected);
    std::filesystem::remove(ultramodern::get_save_file_path());
    read_save_file(); // Actual backup fallback must recover the original copy.
    assert(save_context.save_buffer == original);
    std::cout << "PASS: real runtime reads all 2048 save bytes; word-swapped read, write/reopen and backup recovery pass on disposable copy. No game launch.\n";
}
