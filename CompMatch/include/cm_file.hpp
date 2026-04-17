#pragma once
#ifndef CM_FILE_HPP
#define CM_FILE_HPP
#include <vector>
#include <string>
#include <regex>


namespace {
    std::string globToRegex(const std::string& inGlob) {
        std::string regex;
        for (char c : inGlob) {
            switch (c) {
            case '*': regex += ".*"; break;
            case '?': regex += "."; break;
            case '.': regex += "\\."; break;
            case '\\': regex += "\\\\"; break;
            default:
                if (std::isalnum(c)) regex += c;
                else regex += '\\' + std::string(1, c);
            }
        }
        return regex;
    }
}

namespace cm {
    std::vector<std::string> glob(const std::string& inPat) {
        std::vector<std::string> result;

        std::filesystem::path pathPattern = inPat;
        std::filesystem::path baseDir = pathPattern.parent_path();
        std::string filenamePattern = pathPattern.filename().string();

        if (baseDir.empty()) baseDir = ".";

        std::regex regexPattern(globToRegex(filenamePattern));

        for (const auto& entry : std::filesystem::directory_iterator(baseDir)) {
            std::string name = entry.path().filename().string();
            if (std::regex_match(name, regexPattern)) {
                result.push_back(entry.path().string());
            }
        }

        return result;
    }

    std::vector<std::string> split(const std::string& inStr, const std::string& inDel) {
        std::vector<std::string> result;
        size_t posStart = 0, posEnd;
        size_t delimLen = inDel.length();

        while ((posEnd = inStr.find(inDel, posStart)) != std::string::npos) {
            result.emplace_back(inStr.substr(posStart, posEnd - posStart));
            posStart = posEnd + delimLen;
        }

        result.emplace_back(inStr.substr(posStart));
        return result;
    }
}

#endif
