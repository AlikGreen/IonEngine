#include "frustum.h"

namespace ion
{
    static void normalize(glm::vec4& p)
    {
        const float len = glm::length(glm::vec3(p));
        if (len > 0.0f) p /= len;
    }

    void Frustum::update(const glm::mat4 &projection, const glm::mat4 &view)
    {
        glm::mat4 m = projection * view;

        const glm::vec4 r0(m[0][0], m[1][0], m[2][0], m[3][0]);
        const glm::vec4 r1(m[0][1], m[1][1], m[2][1], m[3][1]);
        const glm::vec4 r2(m[0][2], m[1][2], m[2][2], m[3][2]);
        const glm::vec4 r3(m[0][3], m[1][3], m[2][3], m[3][3]);

        left  = r3 + r0;
        right = r3 - r0;
        bottom = r3 + r1;
        top    = r3 - r1;
        nearP  = r3 + r2;
        farP   = r3 - r2;

        normalize(left);
        normalize(right);
        normalize(bottom);
        normalize(top);
        normalize(nearP);
        normalize(farP);
    }

    bool Frustum::intersects(const AABB &aabb) const
    {
        const glm::vec3 c = (aabb.min + aabb.max) * 0.5f;
        const glm::vec3 e = (aabb.max - aabb.min) * 0.5f;

        for (auto* p : {&left, &right, &bottom, &top, &nearP, &farP})
        {
            glm::vec3 n(p->x, p->y, p->z);
            const float d = p->w;
            const float r = e.x * glm::abs(n.x) + e.y * glm::abs(n.y) + e.z * glm::abs(n.z);
            const float s = glm::dot(n, c) + d;
            if (s + r < 0.0f) return false;
        }

        return true;
    }
}
