#pragma once
#ifndef CM_HALCON_HPP
#define CM_HALCON_HPP
#include <vector>
#include "HalconCpp.h"


namespace {
    template<typename> inline constexpr bool always_false = false;
}

namespace cm {
    template<typename T>
    std::vector<T> tupleToVector(const HalconCpp::HTuple& inTuple) {
        std::vector<T> result;
        result.reserve(inTuple.Length());
        for (Hlong i = 0; i < inTuple.Length(); ++i) {
            if constexpr (std::is_same_v<T, double>) {
                result.push_back(inTuple[i].D());
            }
            else if constexpr (std::is_same_v<T, int>) {
                result.push_back(inTuple[i].I());
            }
            else if constexpr (std::is_same_v<T, std::string>) {
                result.push_back(std::string(inTuple[i].S()));
            }
            else {
                static_assert(always_false<T>, "Unsupported type for tupleToVector");
            }
        }
        return result;
    }

    HalconCpp::HTuple vectorToHTuple(const std::vector<std::string>& inVec) {
        HalconCpp::HTuple tuple;
        for (const auto& s : inVec)
            tuple.Append(s.c_str());
        return tuple;
    }
}

#endif
