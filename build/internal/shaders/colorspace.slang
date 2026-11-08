module colorspace;
import constants;

float SRGBToLinear(float value)
{
    if (value <= 0.04045)
        return value / 12.92;
    else
        return pow((value + 0.055) / 1.055, 2.4);
}

public float3 SRGBToLinear(float3 color)
{
    float3 outColor;
    outColor.x = SRGBToLinear(color.x);
    outColor.y = SRGBToLinear(color.y);
    outColor.z = SRGBToLinear(color.z);
    return outColor;
}

float LinearToSRGB(float value)
{
    if (value <= 0.0031308)
        return 12.92 * value;
    else
        return 1.055 * pow(value, 1 / 2.4) - 0.055;
}

public float3 LinearToSRGB(float3 color)
{
    float3 outColor;
    outColor.x = LinearToSRGB(color.x);
    outColor.y = LinearToSRGB(color.y);
    outColor.z = LinearToSRGB(color.z);
    return outColor;
}

// sRGB Linear -> ACEScg
public float3 LinearToACEScg(const float3 c)
{
    const float3x3 M = {
        { 0.59719f, 0.35458f, 0.04823f },
        { 0.07600f, 0.90834f, 0.01566f },
        { 0.02840f, 0.13383f, 0.83777f }
    };
    return float3(
        M[0][0] * c.x + M[0][1] * c.y + M[0][2] * c.z,
        M[1][0] * c.x + M[1][1] * c.y + M[1][2] * c.z,
        M[2][0] * c.x + M[2][1] * c.y + M[2][2] * c.z
    );
}

// ACEScg -> sRGB Linear (for display)
public float3 ACEScgToLinear(const float3 c)
{
    const float3x3 M_inv = {
        { 1.60475f, -0.53108f, -0.07367f },
        { -0.10208f, 1.10813f, -0.00605f },
        { -0.00327f, -0.07276f, 1.07603f }
    };
    return float3(
        M_inv[0][0] * c.x + M_inv[0][1] * c.y + M_inv[0][2] * c.z,
        M_inv[1][0] * c.x + M_inv[1][1] * c.y + M_inv[1][2] * c.z,
        M_inv[2][0] * c.x + M_inv[2][1] * c.y + M_inv[2][2] * c.z
    );
}

// Linear RGB -> Oklab
public float3 LinearToOklab(float3 c)
{
    // LMS conversion
    float l = 0.4121656120 * c.x + 0.5362752080 * c.y + 0.0514575653 * c.z;
    float m = 0.2118591070 * c.x + 0.6807189584 * c.y + 0.1074065790 * c.z;
    float s = 0.0883097947 * c.x + 0.2818474174 * c.y + 0.6294548110 * c.z;

    // Cube root
    float l_ = pow(l, 1.0 / 3.0);
    float m_ = pow(m, 1.0 / 3.0);
    float s_ = pow(s, 1.0 / 3.0);

    // Oklab
    return float3(
        0.2104542553 * l_ + 0.7936177850 * m_ - 0.0040720468 * s_,
        1.9779984951 * l_ - 2.4285922050 * m_ + 0.4505937099 * s_,
        0.0259040371 * l_ + 0.7827717662 * m_ - 0.8086757660 * s_
    );
}

// Oklab -> Linear RGB
public float3 OklabToLinear(float3 c)
{
    // Inverse LMS
    float l_ = c.x + 0.3963377774 * c.y + 0.2158037573 * c.z;
    float m_ = c.x - 0.1055613458 * c.y - 0.0638541728 * c.z;
    float s_ = c.x - 0.0894841775 * c.y - 1.2914855480 * c.z;

    // Cube
    l_ = l_ * l_ * l_;
    m_ = m_ * m_ * m_;
    s_ = s_ * s_ * s_;

    // Back to linear RGB
    return float3(
        4.0767416621 * l_ - 3.3077115913 * m_ + 0.2309699292 * s_,
        -1.2684380046 * l_ + 2.6097574011 * m_ - 0.3413193965 * s_,
        -0.0041960863 * l_ - 0.7034186147 * m_ + 1.7076147010 * s_
    );
}

// Simple ACES Filmic tone mapping approximation
public float3 ACESFilmCurve(float3 color)
{
    // Approximate ACES RRT+ODT using Uncharted 2 / Jim Hejl filmic curve
    float A = 2.51;
    float B = 0.03;
    float C = 2.43;
    float D = 0.59;
    float E = 0.14;

    color = (color * (A * color + B)) / (color * (C * color + D) + E);

    // Clamp to [0,1] for display
    return saturate(color);
}


/// Color helper functions

public float luminance(float3 color)
{
    return dot(color, float3(0.299, 0.587, 0.114));
}

public float3 RGBtoHSV(float3 rgb)
{
    float Cmax = max(max(rgb.x, rgb.y), rgb.z);
    float Cmin = min(min(rgb.x, rgb.y), rgb.z);
    float delta = Cmax - Cmin;

    float value = Cmax;

    float saturation = Cmax <= EPSILON ? 0 : delta / Cmax;
    float hue = 0;
    if(delta > EPSILON)
    {
        if(rgb.x >= rgb.y && rgb.x >= rgb.z)
            hue = (((rgb.y - rgb.z) / delta));
        else if(rgb.y >= rgb.x && rgb.y >= rgb.z)
            hue = (((rgb.z - rgb.x) / delta) + 2);
        else if(rgb.z >= rgb.y && rgb.z >= rgb.x)
            hue = (((rgb.x - rgb.y) / delta) + 4);
            
        hue *= 60;

        if(hue < 0) 
            hue += 360;
    }

    return float3(hue, saturation, value);
}

public float3 HSVtoRGB(float3 hsv)
{
    float C = hsv.z * hsv.y;
    float Hprime = hsv.x / 60.0;
    Hprime = Hprime - floor(Hprime / 6.0) * 6.0;
    float X = C * (1 - abs(Hprime % 2.0 - 1.0));

    float3 result = float3(0);
    if(Hprime < 1)
        result = float3(C, X, 0);
    else if(Hprime < 2)
        result = float3(X, C, 0);
    else if(Hprime < 3)
        result = float3(0, C, X);
    else if(Hprime < 4)
        result = float3(0, X, C);
    else if(Hprime < 5)
        result = float3(X, 0, C);
    else if(Hprime < 6)
        result = float3(C, 0, X);
    
    float m = hsv.z - C;
    return float3(result.x + m, result.y + m, result.z + m);
}

public float3 OklabToOklch(float3 oklab)
{
    float lightness = oklab.x;
    float chroma = sqrt(oklab.y * oklab.y + oklab.z * oklab.z);
    float hue = atan2(oklab.z, oklab.y) * RADtoDEG;
    return float3(lightness, chroma, hue);
}

public float3 OklchToOklab(float3 oklch)
{
    float lightness = oklch.x;
    float a = cos(oklch.z * DEGtoRAD) * oklch.y;
    float b = sin(oklch.z * DEGtoRAD) * oklch.y;
    return float3(lightness, a, b);
}

public float3 LinearToOklch(float3 linear)
{
    return OklabToOklch(LinearToOklab(linear));
}

public float3 OklchToLinear(float3 oklch)
{
    return OklabToLinear(OklchToOklab(oklch));
}

public float3 tint(float3 baseColor, float3 tintColor, float intensity)
{
    float3 oklab = LinearToOklab(baseColor);
    float3 oklch = OklabToOklch(oklab);
    float chroma = oklch.y;

    float3 tintOklch = OklabToOklch(LinearToOklab(tintColor));
    float tintSaturation = RGBtoHSV(tintColor).y;
    float3 tintResult = float3(oklch.x * tintOklch.x, oklch.y * tintSaturation, tintOklch.z);
    tintResult = OklchToOklab(tintResult);

    float3 result = lerp(oklab, tintResult, intensity);

    result = OklabToLinear(result);
    return result;
}

public float3 desaturate(float3 baseColor, float strength)
{
    float3 oklch = LinearToOklch(baseColor);
    oklch = float3(oklch.x, oklch.y * (1 - strength), oklch.z);
    return OklchToLinear(oklch);
}

public float3 resaturate(float3 baseColor, float strength)
{
    float3 oklch = LinearToOklch(baseColor);
    float3 hsv = RGBtoHSV(baseColor);
    float3 chromalimiter = HSVtoRGB(float3(hsv.x, 1.0, hsv.z));
    float maxChroma = LinearToOklch(chromalimiter).y * 1.2;

    oklch = float3(oklch.x, clamp(oklch.y * strength, -maxChroma, maxChroma), oklch.z);
    return OklchToLinear(oklch);
}