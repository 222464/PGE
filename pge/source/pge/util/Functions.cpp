#include "Functions.h"

std::string getRootName(const std::string &name) {
    int i;

    for (i = static_cast<int>(name.size()) - 1; i >= 0; i--)
    if (name[i] == '/' || name[i] == '\\')
        break;

    return name.substr(0, i + 1);
}