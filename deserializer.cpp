#include <iostream>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <iomanip>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <cctype>

void printHex(const std::vector<uint8_t>& buffer) {
    std::stringstream ss;
    for (auto b : buffer) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
    }
    std::cout << ss.str();
}

uint64_t readVarInt(const std::vector<uint8_t>& buffer, size_t& offset) {
    uint64_t value;
    if (buffer[offset] < 0xfd) {
        value = buffer[offset];
        offset += 1;
    } else if (buffer[offset] == 0xfd) {
        value = buffer[offset + 1] | (buffer[offset + 2] << 8);
        offset += 3;
    } else if (buffer[offset] == 0xfe) {
        value = buffer[offset + 1] | (buffer[offset + 2] << 8) | (buffer[offset + 3] << 16) | (buffer[offset + 4] << 24);
        offset += 5;
    } else {
        value = static_cast<uint64_t>(buffer[offset + 1]) | (static_cast<uint64_t>(buffer[offset + 2]) << 8) |
                (static_cast<uint64_t>(buffer[offset + 3]) << 16) | (static_cast<uint64_t>(buffer[offset + 4]) << 24) |
                (static_cast<uint64_t>(buffer[offset + 5]) << 32) | (static_cast<uint64_t>(buffer[offset + 6]) << 40) |
                (static_cast<uint64_t>(buffer[offset + 7]) << 48) | (static_cast<uint64_t>(buffer[offset + 8]) << 56);
        offset += 9;
    }
    return value;
}

void deserializeTransaction(const std::vector<uint8_t>& buffer) {
    size_t offset = 0;

    // Version
    uint32_t version = buffer[offset] | (buffer[offset + 1] << 8) | (buffer[offset + 2] << 16) | (buffer[offset + 3] << 24);
    offset += 4;
    std::cout << "Version: " << version << std::endl;

    // Input Count
    uint64_t inputCount = readVarInt(buffer, offset);
    std::cout << "Input Count: " << inputCount << std::endl;

    // Parse Inputs
    for (uint64_t i = 0; i < inputCount; ++i) {
        std::vector<uint8_t> prevTxHash(buffer.begin() + offset, buffer.begin() + offset + 32);
        offset += 32; // Previous Transaction Hash
        uint32_t prevTxIndex = buffer[offset] | (buffer[offset + 1] << 8) | (buffer[offset + 2] << 16) | (buffer[offset + 3] << 24);
        offset += 4;  // Previous Transaction Index

        uint64_t scriptLength = readVarInt(buffer, offset);
        std::vector<uint8_t> scriptSig(buffer.begin() + offset, buffer.begin() + offset + scriptLength);
        offset += scriptLength; // Script Signature

        uint32_t sequence = buffer[offset] | (buffer[offset + 1] << 8) | (buffer[offset + 2] << 16) | (buffer[offset + 3] << 24);
        offset += 4;  // Sequence

        std::cout << "Input: " << std::endl;
        std::cout << "  Previous Transaction Hash: ";
        printHex(prevTxHash);
        std::cout << std::endl;
        std::cout << "  Previous Transaction Index: " << prevTxIndex << std::endl;
        std::cout << "  Script Signature: ";
        printHex(scriptSig);
        std::cout << std::endl;
        std::cout << "  Sequence: " << sequence << std::endl;
    }

    // Output Count
    uint64_t outputCount = readVarInt(buffer, offset);
    std::cout << "Output Count: " << outputCount << std::endl;

    // Parse Outputs
    for (uint64_t i = 0; i < outputCount; ++i) {
        uint64_t value = static_cast<uint64_t>(buffer[offset]) |
                         (static_cast<uint64_t>(buffer[offset + 1]) << 8) |
                         (static_cast<uint64_t>(buffer[offset + 2]) << 16) |
                         (static_cast<uint64_t>(buffer[offset + 3]) << 24) |
                         (static_cast<uint64_t>(buffer[offset + 4]) << 32) |
                         (static_cast<uint64_t>(buffer[offset + 5]) << 40) |
                         (static_cast<uint64_t>(buffer[offset + 6]) << 48) |
                         (static_cast<uint64_t>(buffer[offset + 7]) << 56);
        offset += 8;

        uint64_t scriptLength = readVarInt(buffer, offset);
        std::vector<uint8_t> scriptPubKey(buffer.begin() + offset, buffer.begin() + offset + scriptLength);
        offset += scriptLength;

        std::cout << "Output: " << std::endl;
        std::cout << "  Value: " << static_cast<double>(value) / 100000000.0 << " BTC" << std::endl;
        std::cout << "  ScriptPubKey: ";
        printHex(scriptPubKey);
        std::cout << std::endl;
    }

    // Lock Time
    uint32_t lockTime = buffer[offset] | (buffer[offset + 1] << 8) | (buffer[offset + 2] << 16) | (buffer[offset + 3] << 24);
    offset += 4;
    std::cout << "Lock Time: " << lockTime << std::endl;
}

int main() {
    std::string hexTransaction = "020000000001056a51edb2fdd26ff90ef8a086d1ec0c63210d0edfa67397f0dbeeeb621364e330100000000feffffff2577513282bb52facf186cf6f8893d69aacdb2b0eb0317f8794f4819af2b5010000006a47304402200607b736df8fe395d861a0754c77eb71ead51eb4a5078d9383861e163519798022059a8c9d34af3b9632b119c68dc007dcdac3001eb358925a1db68eeac370d3d4b01210233541665c6963e1b867f8b0076b241e87f7d48f3afa0566f4f22b753d36da32fefffffffd6b5eafd8070281dd36c4981000aac89bab0a54a7a052ff9e895f7443d30100000000feffffff0c8e8cbad2a4feab6875454f1ece3fd33afb333d1624350f2636098c3ccf3680100000000feffffffc64d433bb402aeba7cd58ba084140d87f8c8062ec1c5e668d0695533d776d4000000006a473044022070dea5391f01589cb87926ef8646540e453a5c90b80907680fefcbd513767d4022035ebc67a4360d629d1bdb2d89f210c78d7bc4316eacad1a4fd89427fe001ece012102461652d99350a41b7395a12263bd1e350aca7f6e884733d8ef5260e833ef49feffffff0226d6d000000000017a914978309c10b72077c2102d1853883a5a4f0b61b87b74150000000000160014f7533a613e95c63be15a5198327e7f33ac126a60247304402206ecad418dbf2ab3d6903ee44e3077e058df9968241fdf931093fbaf9a51e11b02201494d62531e6773220072856fc9726bc1e5c93767ea98da4b75d51411187cb280121039350003ee10d39a0811696f7deccf4c31c924ce55c2506191e819833f6c6baa30002473044022021962ffa5db42745d5a9a851c508fab603ca690d17ae962a3aa9a6f0ff9d9ed02204b377666ab296cef8ff5abe9bb35eabd714f8845dbac9f352bcf0af89a8c220a012102a3338fc7a90f8e3802d61f1ea73d77edd257ba978483d3a97d790eb3105845c0247304402200de7b1f320d0498f2053e7c46bad62bf5b5887f5b1f96d64ffa007afa8c0ba6022056669ccfaf4772934cd264755edb94228387802ca038e238977ca5a4dea6d4260121026491015716a9c02db53743dcfa33bd01a45fb287e092f09e5d33756f4c9677040086c70c00";

    // Remove non-hexadecimal characters from the string
    hexTransaction.erase(std::remove_if(hexTransaction.begin(), hexTransaction.end(), [](unsigned char c) { return !std::isxdigit(c); }), hexTransaction.end());

    // Check if hex string length is even
    if (hexTransaction.size() % 2 != 0) {
        std::cerr << "Invalid hex string length" << std::endl;
        return 1;
    }

    std::vector<uint8_t> buffer;
    try {
        for (size_t i = 0; i < hexTransaction.size(); i += 2) {
            uint8_t byte = static_cast<uint8_t>(std::stoul(hexTransaction.substr(i, 2), nullptr, 16));
            buffer.push_back(byte);
        }
    } catch (const std::invalid_argument& e) {
        std::cerr << "Invalid hex string: " << e.what() << std::endl;
        return 1;
    }

    deserializeTransaction(buffer);

    return 0;
}
