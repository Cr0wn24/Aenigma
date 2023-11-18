#ifndef MATH_H
#define MATH_H

#define Square(x) ((x) * (x))
#define Max(a, b) ((a) < (b) ? (b) : (a))
#define Min(a, b) ((a) > (b) ? (b) : (a))
#define Clamp(a, val, b) (Max(a, Min(b, val)))
#define Cycle(a, val, b) (val < a) ? b : ((val > b) ? a : val)
#define Abs(a) ((a) < 0 ? (-(a)) : (a))

function inline v2
V2(f32 x, f32 y)
{
    v2 result;

    result.x = x;
    result.y = y;

    return result;
}

function inline v2
V2AddV2(v2 a, v2 b)
{
    v2 result;

    result.x = a.x + b.x;
    result.y = a.y + b.y;

    return result;
}

function inline v2
V2SubV2(v2 a, v2 b)
{
    v2 result;

    result.x = a.x - b.x;
    result.y = a.y - b.y;

    return result;
}

function inline v2
V2MulF32(v2 a, f32 b)
{
    v2 result;

    result.x = a.x * b;
    result.y = a.y * b;

    return result;
}

function inline v2
V2DivF32(v2 a, f32 b)
{
    v2 result;

    result = V2MulF32(a, 1.0f / b);

    return result;
}

function inline v2s
V2s(s32 x, s32 y)
{
    v2s result;

    result.x = x;
    result.y = y;

    return result;
}

function inline v2s
V2sAddV2s(v2s a, v2s b)
{
    v2s result;

    result.x = a.x + b.x;
    result.y = a.y + b.y;

    return result;
}

function inline v2s
V2s_sub_V2s(v2s a, v2s b)
{
    v2s result;

    result.x = a.x - b.x;
    result.y = a.y - b.y;

    return result;
}
function inline v2u
V2u(u32 x, u32 y)
{
    v2u result;

    result.x = x;
    result.y = y;

    return result;
}

function inline v3
V3(f32 x, f32 y, f32 z)
{
    v3 result;

    result.x = x;
    result.y = y;
    result.z = z;

    return result;
}

function inline v3
V3AddV3(v3 a, v3 b)
{
    v3 result;

    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;

    return result;
}

function inline v3s
V3s(s32 x, s32 y, s32 z)
{
    v3s result;

    result.x = x;
    result.y = y;
    result.z = z;

    return result;
}

function inline v3s
V3sAddV3s(v3s a, v3s b)
{
    v3s result;

    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;

    return result;
}

function inline v4
V4(f32 x, f32 y, f32 z, f32 w)
{
    v4 result;

    result.x = x;
    result.y = y;
    result.z = z;
    result.w = w;

    return result;
}

function inline v4
V4MulF32(v4 a, f32 b)
{
    v4 result;

    result.x = a.x * b;
    result.y = a.y * b;
    result.z = a.z * b;
    result.w = a.w * b;

    return result;
}


function inline v4
V4AddV4(v4 a, v4 b)
{
    v4 result;

    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    result.w = a.w + b.w;

    return result;
}

//
// Shaping functions
//

function inline f32
LerpF32(f32 a, f32 b, f32 t)
{
    f32 result;

    result = a * (1 - t) + b * t;

    return result;
}

function inline f32
Lerp(f32 t)
{
    f32 result;

    result = t;

    return result;
}

function inline f32
SmoothStep(f32 t)
{
    f32 result;

    f32 t1 = t * t;
    f32 t2 = 1.0f - (1.0f - t) * (1.0f - t);

    result = LerpF32(t1, t2, t);

    return result;
}

function inline f32
QuadraticEaseOut(f32 t)
{
    f32 result;

    result = 1.0f - (1.0f - t) * (1.0f - t);

    return result;
}

function inline v2
QuadraticEaseOutV2(v2 a, v2 b, f32 t)
{
    t = QuadraticEaseOut(t);

    v2 result;

    a.x *= (1 - t);
    a.y *= (1 - t);

    b.x *= t;
    b.y *= t;

    result.x = a.x + b.x;
    result.y = a.y + b.y;

    return result;
}

function inline v3
QuadraticEaseOutV3(v3 a, v3 b, f32 t)
{
    t = QuadraticEaseOut(t);

    v3 result;

    a.x *= (1 - t);
    a.y *= (1 - t);
    a.z *= (1 - t);

    b.x *= t;
    b.y *= t;
    b.z *= t;

    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;

    return result;
}

function inline v2
LerpV2(v2 a, v2 b, f32 t)
{
    v2 result;

    result.x = LerpF32(a.x, b.x, t);
    result.y = LerpF32(a.y, b.y, t);

    return result;
}

function inline v3
LerpV3(v3 a, v3 b, f32 t)
{
    v3 result;

    result.x = LerpF32(a.x, b.x, t);
    result.y = LerpF32(a.y, b.y, t);
    result.z = LerpF32(a.z, b.z, t);

    return result;
}

function inline v4
LerpV4(v4 a, v4 b, f32 t)
{
    v4 result;

    result.r = LerpF32(a.r, b.r, t);
    result.g = LerpF32(a.g, b.g, t);
    result.b = LerpF32(a.b, b.b, t);
    result.a = LerpF32(a.a, b.a, t);

    return result;
}

function inline f32
SmoothStepF32(f32 a, f32 b, f32 t)
{
    if(t >= 1.0f)
    {
        return b;
    }

    f32 result;

    t = SmoothStep(t);

    a *= (1 - t);

    b *= (t);

    result = a + b;

    return result;
}

function inline f32
QuadraticEaseOutF32(f32 a, f32 b, f32 t)
{
    if(t >= 1.0f)
    {
        return b;
    }

    f32 result;

    t = QuadraticEaseOut(t);

    a *= (1 - t);

    b *= (t);

    result = a + b;

    return result;
}

function inline v2
SmoothStepV2(v2 a, v2 b, f32 t)
{
    v2 result;

    t = SmoothStep(t);

    a.x *= (1 - t);
    a.y *= (1 - t);

    b.x *= (t);
    b.y *= (t);

    result.x = a.x + b.x;
    result.y = a.y + b.y;

    return result;
}

function inline v3
SmoothStepV3(v3 a, v3 b, f32 t)
{
    v3 result;

    t = SmoothStep(t);

    a.x *= (1 - t);
    a.y *= (1 - t);
    a.z *= (1 - t);

    b.x *= (t);
    b.y *= (t);
    b.z *= (t);

    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;

    return result;
}

function inline v4
SmoothStepV4(v4 a, v4 b, f32 t)
{
    v4 result;

    t = SmoothStep(t);

    a.r *= (1 - t);
    a.g *= (1 - t);
    a.b *= (1 - t);
    a.a *= (1 - t);

    b.r *= (t);
    b.g *= (t);
    b.b *= (t);
    b.a *= (t);

    result.r = a.r + b.r;
    result.g = a.g + b.g;
    result.b = a.b + b.b;
    result.a = a.a + b.a;

    return result;
}

function inline v4
QuadraticEaseOutV4(v4 a, v4 b, f32 t)
{
    v4 result;

    t = QuadraticEaseOut(t);

    a.r *= (1 - t);
    a.g *= (1 - t);
    a.b *= (1 - t);
    a.a *= (1 - t);

    b.r *= (t);
    b.g *= (t);
    b.b *= (t);
    b.a *= (t);

    result.r = a.r + b.r;
    result.g = a.g + b.g;
    result.b = a.b + b.b;
    result.a = a.a + b.a;

    return result;
}

#endif