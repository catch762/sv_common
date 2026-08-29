#pragma once
#include "Logging.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

inline bool vectorsParallel(const glm::vec3& v1, const glm::vec3& v2, float epsilon = 0.0000001f)
{
    // Compute the cross product
    glm::vec3 crossProd = glm::cross(v1, v2);

    // Check if the squared length of the cross product is near zero
    return glm::length2(crossProd) < epsilon;
}


template <glm::length_t L, typename T, glm::qualifier Q>
struct std::formatter<glm::vec<L, T, Q>, char>
{
public:
    constexpr auto parse(std::format_parse_context& ctx)
    {
        // Accept only empty spec or a simple pass-through if you want.
        // For now, just ensure we see '}' and do nothing else.
        auto it = ctx.begin();
        if (it != ctx.end() && *it != '}') {
            // If you want to allow some custom spec later, parse it here.
            // For now, we just ignore everything until '}'.
            while (it != ctx.end() && *it != '}') {
                ++it;
            }
        }
        return it;
    }

    template <typename FormatContext>
    auto format(const glm::vec<L, T, Q>& v, FormatContext& ctx) const
    {
        auto out = ctx.out();

        out = std::format_to(out, "vec{}(", L);

        for (glm::length_t i = 0; i < L; ++i) {
            if (i != 0) {
                out = std::format_to(out, ", ");
            }
            // Use default formatting for T; no runtime-built format string.
            out = std::format_to(out, "{}", v[i]);
        }

        out = std::format_to(out, ")");
        return out;
    }
};