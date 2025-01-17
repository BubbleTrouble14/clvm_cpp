#include "clvm_utils.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

namespace chia
{
namespace utils
{

Bytes StrToBytes(std::string_view str)
{
    Bytes b;
    b.resize(str.size());
    memcpy(b.data(), str.data(), str.size());
    return b;
}

Bytes32 BytesToHash(Bytes const& bytes) { return bytes_cast<HASH256_LEN>(bytes); }

Bytes HashToBytes(Bytes32 const& hash) { return bytes_cast<HASH256_LEN>(hash); }

Bytes32 HashFromHex(std::string_view hex)
{
    Bytes bytes = BytesFromHex(hex);
    return bytes_cast<HASH256_LEN>(bytes);
}

std::string HashToHex(Bytes32 const& hash)
{
    Bytes bytes = bytes_cast<HASH256_LEN>(hash);
    return BytesToHex(bytes);
}

Bytes MakeBytes(char const* sz)
{
    int len = strlen(sz);
    Bytes result(len, '\0');
    memcpy(result.data(), sz, len);
    return result;
}

char const hex_chars[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

char Byte4bToHexChar(uint8_t hex) { return hex_chars[hex]; }

uint8_t HexCharToByte4b(char ch)
{
    for (int i = 0; i < 16; ++i) {
        if (std::tolower(ch) == hex_chars[i]) {
            return i;
        }
    }
    // Not found, the character is invalid
    throw std::runtime_error("invalid character");
}

std::string ByteToHex(uint8_t byte)
{
    std::string hex(2, '0');
    uint8_t hi = (byte & 0xf0) >> 4;
    uint8_t lo = byte & 0x0f;
    hex[0] = Byte4bToHexChar(hi);
    hex[1] = Byte4bToHexChar(lo);
    return hex;
}

uint8_t ByteFromHex(std::string_view hex, int* consumed)
{
    if (hex.empty()) {
        if (consumed) {
            *consumed = 0;
        }
        return 0;
    }
    if (hex.size() == 1) {
        if (consumed) {
            *consumed = 1;
        }
        return HexCharToByte4b(hex[0]);
    }
    uint8_t byte = (static_cast<int>(HexCharToByte4b(hex[0])) << 4) + HexCharToByte4b(hex[1]);
    if (consumed) {
        *consumed = 2;
    }
    return byte;
}

std::string BytesToHex(Bytes const& bytes)
{
    std::stringstream ss;
    for (uint8_t byte : bytes) {
        ss << ByteToHex(byte);
    }
    return ss.str();
}

Bytes BytesFromHex(std::string_view hex)
{
    Bytes res;
    int consumed;
    uint8_t byte = ByteFromHex(hex, &consumed);
    while (consumed > 0) {
        res.push_back(byte);
        hex = hex.substr(consumed);
        // Next byte
        byte = ByteFromHex(hex, &consumed);
    }
    return res;
}

std::string ArgsToStr(std::vector<Bytes> const& args)
{
    if (args.empty()) {
        return "";
    }
    std::stringstream ss;
    ss << "(";
    auto i = std::begin(args);
    ss << BytesToHex(*i);
    ++i;
    while (i != std::end(args)) {
        ss << ", " << BytesToHex(*i);
        ++i;
    }
    ss << ")";
    return ss.str();
}

std::string LoadHexFromFile(std::string file_path)
{
    auto file_path_str = std::string(file_path);
    std::ifstream in(file_path_str);
    if (!in.is_open()) {
        std::stringstream ss;
        ss << "cannot open file: " << file_path << " to read";
        throw std::runtime_error(ss.str());
    }
    std::stringstream ss;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        ss << line;
    }
    return ss.str();
}

Bytes ByteToBytes(uint8_t b)
{
    Bytes res(1);
    res[0] = b;
    return res;
}

Bytes SubBytes(Bytes const& bytes, int start, int count)
{
    int n;
    if (count == 0) {
        n = static_cast<int>(bytes.size()) - start;
    } else {
        n = count;
    }
    Bytes res(n);
    memcpy(res.data(), bytes.data() + start, n);
    return res;
}

std::vector<Int> BytesToInts(Bytes const& bytes)
{
    std::vector<Int> res(bytes.size());
    std::transform(std::begin(bytes), std::end(bytes), std::begin(res), [](uint8_t val) -> Int { return Int(val); });
    return res;
}

Bytes IntsToBytes(std::vector<Int> const& ints)
{
    Bytes res(ints.size());
    std::transform(std::cbegin(ints), std::cend(ints), std::begin(res), [](Int const& val) -> uint8_t {
        if (val.ToInt() > 0xff || val.ToInt() < 0) {
            throw std::runtime_error(
                "The source ints is not be able to convert to bytes, value of elements is out of range");
        }
        return val.ToInt();
    });
    return res;
}

void BufferConnector::Append(Bytes const& rhs)
{
    std::size_t p = result_.size();
    result_.resize(result_.size() + rhs.size());
    memcpy(result_.data() + p, rhs.data(), rhs.size());
}

Bytes const& BufferConnector::GetResult() const { return result_; }

Bytes RevertBytes(Bytes const& in)
{
    Bytes b;
    std::copy(in.rbegin(), in.rend(), std::back_inserter(b));
    return b;
}

std::string ToUpper(std::string str)
{
    std::string res;
    std::transform(std::begin(str), std::end(str), std::back_inserter(res), [](char ch) { return std::toupper(ch); });
    return res;
}

std::string ToLower(std::string str)
{
    std::string res;
    std::transform(std::begin(str), std::end(str), std::back_inserter(res), [](char ch) { return std::tolower(ch); });
    return res;
}

} // namespace utils
} // namespace chia
