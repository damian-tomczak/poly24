#pragma once

struct Vector2D final
{
    float x, y;

    constexpr Vector2D(float x = 0.f, float y = 0.f) : x(x), y(y)
    {}

    constexpr Vector2D operator+(const Vector2D& v) const
    {
        return Vector2D(x + v.x, y + v.y);
    }

    constexpr Vector2D& operator+=(const Vector2D& v)
    {
        x += v.x;
        y += v.y;
        return *this;
    }

    constexpr Vector2D operator*(float scalar) const
    {
        return Vector2D(x * scalar, y * scalar);
    }

    constexpr Vector2D& operator*=(float scalar)
    {
        x *= scalar;
        y *= scalar;
        return *this;
    }
};