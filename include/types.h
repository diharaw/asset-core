#pragma once

#include <stdint.h>
#include <glm.hpp>
#include <string>
#include <array>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtc/quaternion.hpp>
#include <json.hpp>

#define T_SAFE_DELETE(x) if(x){ delete x; x = nullptr; }
#define T_SAFE_DELETE_ARRAY(x) if(x) { delete[] x; x = nullptr; }

using ResourceHandle = unsigned;
using String = std::string;
using Json = nlohmann::json;
