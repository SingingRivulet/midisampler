#pragma once
#include <sstream>
#include <string>
#include <vector>

namespace midiSearch {

//构建相对值
template <typename relativeArray>
relativeArray buildRelativeArray(const relativeArray& arr) {
    relativeArray res;
    int last;
    bool first = true;
    for (auto it : arr) {
        if (it != 0) {
            if (!first) {
                res.push_back(it - last);
            }
            last = it;
            first = false;
        }
    }
    return res;
}

//将相对值转为字符串
template <typename relativeArray>
std::string buildRelativeString(const relativeArray& arr) {
    std::ostringstream res;
    for (auto it : arr) {
        res << it << " ";
    }
    return res.str();
}

}  // namespace midiSearch