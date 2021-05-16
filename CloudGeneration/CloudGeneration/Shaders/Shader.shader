#shader vertex
#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;

out vec2 vTexCoord;
out vec3 vPosition;
out vec2 vSt;

uniform float uVTime;
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;	//model, view, projection - so everything is positioned in 3d space correctly

uniform vec2 uResolution;

void main()
{
    //gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position.xy, position.z, 1.0f);	//(x, y, z, w) w divides other vector components
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position.xy + sin(uVTime * 0.0001) * 0.5, position.z, 1.0f);	//(x, y, z, w) w divides other vector components

    vTexCoord = texCoord;
    vPosition = position;

    //components of texture coordinates (st) instead of UV for opengl
    vSt = position.xy / uResolution.xy;
};

#shader fragment
#version 330 core

layout(location = 0) out vec4 colour;

in vec2 vTexCoord;
in vec3 vPosition;
in vec2 vSt;

uniform vec4 uColour;
uniform int uBackground;
uniform sampler2D uTexture[5];

uniform float curNucVal;
uniform float nextNucVal;
uniform float randNumber0to1[10];
uniform float uFTime;

uniform float uTemp;
uniform float uDens;
uniform float uPress;
uniform float uAltitude;

uniform int textureID;

// Permutation polynomial: (34x^2 + x) mod 289
vec4 permute(vec4 x)
{
    return mod((34.0 * x + 1.0) * x, 289.0);
}

vec3 dist(vec3 x, vec3 y, bool manhattanDistance)
{
    return manhattanDistance ? abs(x) + abs(y) : (x * x + y * y);
}

vec2 worley(vec2 P, float jitter)
{
    float K = 0.142857142857; // 1/7
    float K2 = 0.428571428571;// 3/7

    //Create the grid for cells
    vec2 Pi = mod(floor(P), 289.0);
    vec2 Pf = fract(P);

    vec4 Pfx = Pf.x + vec4(-0.5, -1.5, -0.5, -1.5);
    vec4 Pfy = Pf.y + vec4(-0.5, -0.5, -1.5, -1.5);

    //Calculate point position
    vec4 p = permute(Pi.x + vec4(0.0, 1.0, 0.0, 1.0));
    p = permute(p + Pi.y + vec4(0.0, 0.0, 1.0, 1.0));

    vec4 ox = mod(p, 7.0) * K + K2;
    vec4 oy = mod(floor(p * K), 7.0) * K + K2;

    vec4 dx = Pfx + jitter * ox;
    vec4 dy = Pfy + jitter * oy;

    vec4 d = dx * dx + dy * dy; // d11, d12, d21 and d22, squared

    // Sort out the two smallest distances
    // Find both F1 and F2
    d.xy = (d.x < d.y) ? d.xy : d.yx; // Swap if smaller
    d.xz = (d.x < d.z) ? d.xz : d.zx;
    d.xw = (d.x < d.w) ? d.xw : d.wx;

    d.y = min(d.y, d.z);
    d.y = min(d.y, d.w);

    return sqrt(d.xy);
}

vec3 mod289(vec3 x) {
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec2 mod289(vec2 x) {
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec3 permute(vec3 x) {
    return mod289(((x * 34.0) + 1.0) * x);
}

float snoise(vec2 v)
{
    const vec4 C = vec4(0.211324865405187,  // (3.0-sqrt(3.0))/6.0
        0.366025403784439,  // 0.5*(sqrt(3.0)-1.0)
        -0.577350269189626,  // -1.0 + 2.0 * C.x
        0.024390243902439); // 1.0 / 41.0

    // First corner
    vec2 i = floor(v + dot(v, C.yy));
    vec2 x0 = v - i + dot(i, C.xx);

    // Other corners
    vec2 i1;
    //i1.x = step( x0.y, x0.x ); // x0.x > x0.y ? 1.0 : 0.0
    //i1.y = 1.0 - i1.x;

    i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
    // x0 = x0 - 0.0 + 0.0 * C.xx ;
    // x1 = x0 - i1 + 1.0 * C.xx ;
    // x2 = x0 - 1.0 + 2.0 * C.xx ;

    vec4 x12 = x0.xyxy + C.xxzz;
    x12.xy -= i1;

    // Permutations
    i = mod289(i); // Avoid truncation effects in permutation
    vec3 p = permute(permute(i.y + vec3(0.0, i1.y, 1.0))
        + i.x + vec3(0.0, i1.x, 1.0));

    vec3 m = max(0.5 - vec3(dot(x0, x0), dot(x12.xy, x12.xy), dot(x12.zw, x12.zw)), 0.0);
    m = m * m;
    m = m * m;

    // Gradients: 41 points uniformly over a line, mapped onto a diamond.
    // The ring size 17*17 = 289 is close to a multiple of 41 (41*7 = 287)

    vec3 x = 2.0 * fract(p * C.www) - 1.0;
    vec3 h = abs(x) - 0.5;
    vec3 ox = floor(x + 0.5);
    vec3 a0 = x - ox;

    // Normalise gradients implicitly by scaling m
    // Approximation of: m *= inversesqrt( a0*a0 + h*h );
    m *= 1.79284291400159 - 0.85373472095314 * (a0 * a0 + h * h);

    // Compute final noise value at P
    vec3 g;
    g.x = a0.x * x0.x + h.x * x0.y;
    g.yz = a0.yz * x12.xz + h.yz * x12.yw;

    return 130.0 * dot(m, g);
}

float GetPerlinValues(vec2 scale, vec2 pos)
{
    return snoise(scale + vec2(pos));
}

vec4 SimplexNoise(vec2 scale, vec2 pos)
{
    float n = GetPerlinValues(scale, pos);

    return vec4(0.5 + 0.5 * vec3(n, n, n), 1.0f);
}

float random(vec2 p)
{
    // e^pi (Gelfond's constant), // 2^sqrt(2) (Gelfond–Schneider constant)
    vec2 K1 = vec2( 23.14069263277926, 2.665144142690225 );

    return fract(cos(dot(p, K1)) * 12345.6789);
}

float noise(vec2 _st) 
{
    vec2 i = floor(_st);
    vec2 f = fract(_st);

    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) +  (d - b) * u.x * u.y;
}

float fbm(vec2 _st, int octaves, float texShift, bool choice)
{
    float v = 0.0;
    float a = 0.5;

    // Rotate to reduce axial bias
    vec2 shift = vec2(texShift);

    mat2 rot;

    if (choice)
    {
        rot = mat2(cos(0.5), sin(0.5), -sin(0.5), cos(0.50));
    }
    else
    {
        rot = mat2(sin(0.9), cos(0.9), -cos(0.9), sin(0.90));
    }

    for (int i = 0; i < octaves; ++i) 
    {
        v += a * noise(_st);
        _st = rot * _st * 2.0 + shift;
        a *= 0.5;
    }

    return v;
}

float MixValue(vec2 st, int x, int y)
{
    float mixValue = distance(st, vec2(vTexCoord.x * randNumber0to1[x], vTexCoord.y * randNumber0to1[y]));
    //float mixValue = distance(st, vec2(vTexCoord.x, vTexCoord.y));

    return mixValue;
}

//void CalcAtmosphere()
//{
//    int altitude[25]; //array of values for the altitude, from 0 to 25km
//
//    for (int i = 0; i < 25; i++)
//    {
//        altitude[i] = i * 1000;
//
//        //compute temperature of atmosphere
//        float common = 1.0f - (((gamma - 1.0f) / gamma) * (altitude[i] / scaleHeight));
//        float temp0 = temperature * common;
//
//        //(1 - ((gamma - 1) / gamma) * (altitude[i] / scaleHeight));
//
//        //compute density and pressure
//        float press0 = pressure * glm::pow(common, (gamma / (gamma - 1.0f)));
//        float dens0 = density * glm::pow(common, (1.0f / (gamma - 1.0f)));
//
//        //set results
//        dens.push_back(dens0);
//        temp.push_back(temp0);
//        press.push_back(press0);
//    }
//
//    CalcWaterPressure();
//}

float CalcSatPressure(float t)
{
    t -= 273;	//convert to Celsius

    return 610.78f * exp((17.27 * t) / (237.3 + t));
}

#define PI 3.1415926535897932384626433832795

float CalcNucleation(float t, float waterPress)
{
    float sigma = 72.75e-3;
    float dalton = 1.66053906660e-27;
    float m1 = 18.02 * dalton;                  //m1 = 2.99e-23f * 1e-3f  Converted into SI
    float v1 = 2.99e-23 * 1e-6;                 // converted into SI
    float kb = 1.38e-23;

    float N = waterPress / (kb * t);
    float S = waterPress / CalcSatPressure(t);

    float inside = -(16.0f * PI / 3.0f)
        * (pow(v1, 2.0) * pow(sigma, 3.0))
        / (pow(kb * t, 3.0) * pow(log(S), 2.0));

    float j = sqrt(2.0 * sigma / (PI * m1)) * (v1 * pow(N, 2.0) / S) * exp(inside);

    return j;
}

void CalcWaterPressure(float superSat, inout float nucleation)
{
    float pressH20;

    //colder depending on higher the altitude so number drops as the altitude increases

    float temp = 293.f / uAltitude;     ///could change dependiong on height value from scene

    pressH20 = (superSat * uAltitude) * CalcSatPressure(temp);

    nucleation = CalcNucleation(temp, pressH20);
}

vec4 fbmNoiseTex(vec2 st, vec2 qTexCoordsX, vec2 qTexCoordsY, vec2 rTexCoordsX, vec2 rTexCoordsY, float offset, int octaves, float shift, inout float nucleation)
{
    vec2 q = vec2(0.);
    vec2 r = vec2(0.);

    q.x = fbm(st + qTexCoordsX * uFTime * 0.003, octaves, shift, true);
    q.y = fbm(st + qTexCoordsY, octaves, shift, true);

    r.x = fbm(st + 1.0 * q + rTexCoordsX + offset + 0.15 * uFTime * 0.003, octaves, shift, false);
    r.y = fbm(st + 1.0 * q + rTexCoordsY + offset * uFTime * 0.003, octaves, shift, true);

    float f = fbm(st + r, octaves, shift, true);

    vec4 fBmNoiseTex = vec4((f * f * f + .6 * f * f + .5) * colour.xyz, 0.7);

    //CalcWaterPressure(f, nucleation);

    //float nucleationDelta = (5.2899401006521426e+28 - 3.39642e-48);

    //float nucleationRange = nucleation / nucleationDelta;

    //fBmNoiseTex.a *= nucleationRange;

    return fBmNoiseTex;
}

void Setupfbm(vec2 st, float scale, vec2 qTexCoordsX, vec2 qTexCoordsY, vec2 rTexCoordsX, vec2 rTexCoordsY, float offset, int octaves, float shift, int mixValueX, int mixValueY, inout float nucleation)
{
    vec2 fbmSt = vec2(st * scale);

    colour *= fbmNoiseTex(fbmSt, qTexCoordsX, qTexCoordsX, rTexCoordsX, rTexCoordsY, offset, octaves, shift, nucleation);

    float mixValue = MixValue(fbmSt, mixValueX, mixValueY);

    colour = mix(colour, uColour, mixValue);   //background mix
}

void SetupWorley(vec2 st, float scale, inout float nucleation)
{
    vec2 wSt = st * scale;

    vec2 F = worley(wSt, 0.8f);

    //Closest and second closest
    float F1 = F.x;
    float F2 = F.y;

    float n = 1.0 - 1.5 * F1;

    vec4 worleyNoiseTex = vec4(n * n * n * colour.xyz, 0.8);

    CalcWaterPressure(n * n, nucleation);

    float nucleationDelta = (5.2899401006521426e+28 - 3.39642e-48);

    float nucleationRange = nucleation / nucleationDelta;

    worleyNoiseTex.a *= nucleationRange;

    colour = mix(colour, worleyNoiseTex, 0.4);

    //colour *= worleyNoiseTex;
}

void Texture1(inout float nucleation)
{
    vec4 texColour = texture(uTexture[0], vTexCoord);
    colour = texColour;

    Setupfbm(vSt, 210., vec2(0.0), vec2(1.0), vec2(1.7, 9.2), vec2(8.3, 2.8), 0.126, 6, 100.0, 3, 9, nucleation);

    SetupWorley(vSt, 210., nucleation);
}
void Texture2(inout float nucleation)
{
    vec4 texColour = texture(uTexture[1], vTexCoord);
    colour = texColour;

    Setupfbm(vSt, 350., vec2(-0.150, 0.430), vec2(1.0), vec2(-0.340, 0.420), vec2(-0.400, -0.500), 0.0126, 7, 200.0, 4, 1, nucleation);

    SetupWorley(vSt, 250., nucleation);
}
void Texture3(inout float nucleation)
{
    vec4 texColour = texture(uTexture[2], vTexCoord);
    colour = texColour;

    Setupfbm(vSt, 310., vec2(0.), vec2(-0.750, 0.750), vec2(0.680, -0.910), vec2(0.670, 0.380), 0.326, 8, 150.0, 9, 0, nucleation);

    SetupWorley(vSt, 180., nucleation);
}
void Texture4(inout float nucleation)
{
    vec4 texColour = texture(uTexture[3], vTexCoord);
    colour = texColour;
  
    Setupfbm(vSt, 270., vec2(0.), vec2(1.), vec2(-0.960, 0.660), vec2(0.2), 0.05, 7, 300.0, 9, 8, nucleation);

    //SetupWorley(vSt, 100., nucleation);
}
void Texture5(inout float nucleation)
{
    vec4 texColour = texture(uTexture[4], vTexCoord);
    colour = texColour;

    Setupfbm(vSt, 240., vec2(1.), vec2(-0.470, -0.530), vec2(0.), vec2(0.520, -0.590), 0.2, 4, 180.0, 2, 2, nucleation);

    SetupWorley(vSt, 100., nucleation);
}

void main()
{
    float nucleation = 0;

    if (textureID == 0)
    {
        Texture1(nucleation);
    }
    else if (textureID == 1)
    {
        Texture2(nucleation);
    }
    else if (textureID == 2)
    {
        Texture3(nucleation);
    }
    else if (textureID == 3)
    {
        Texture4(nucleation);
    }
    else if (textureID == 4)
    {
        Texture5(nucleation);
    }

    //float nucleationRange = (.666 / log(nucleation));
    //float nucleationRange = (5.2899401006521426e+28 / nucleation);

    colour.a *= curNucVal;

    //if (textureID == 0)
    //{
    //    texColour = texture(uTexture[0], vTexCoord); //find texCoord pixel to sample
    //}
    //else if(textureID == 1)
    //{
    //    texColour = texture(uTexture[1], vTexCoord); //find texCoord pixel to sample
    //}

    //colour = texColour;

    ////fBm
    //
    //vec2 fBmSt = vPosition.xy / uResolution.xy;

    //fBmSt *= 210.0;

    //vec2 q = vec2(0.);
    //vec2 r = vec2(0.);

    //if (textureID == 0)
    //{
    //    q.x = fbm(fBmSt * uFTime * 0.0001, true);
    //    q.y = fbm(fBmSt + vec2(1.0), true);

    //    r.x = fbm(fBmSt + 1.0 * q + vec2(1.7, 9.2) + 0.15 * uFTime * 0.01, false);
    //    r.y = fbm(fBmSt + 1.0 * q + vec2(8.3, 2.8) + 0.126 * uFTime * 0.001, true);
    //}
    //else
    //{
    //    q.x = fbm(fBmSt + vec2(vPosition.x + 5.3, vPosition.y + 9.2) + 0.15 * uFTime * 0.001, true);
    //    q.y = fbm(fBmSt, false);

    //    r.x = fbm(fBmSt + 1.0 * q + vec2(vPosition.x + 8.3, vPosition.y - 2.8) + 0.15 * uFTime * 0.001, false);
    //    r.y = fbm(fBmSt + 1.0 * q + vec2(vPosition.x - 1.7, vPosition.y + 9.2) + 0.126 * uFTime * 0.001, true);
    //}

    //float f = fbm(fBmSt + r, true);

    //vec4 fBmNoiseTex = vec4((f * f * f + .6 * f * f + .5) * colour.xyz, 1.);

    //colour *= fBmNoiseTex;

    //////Worley
    //{
    //    //vec2 wScale = vPosition.xy / uResolution.xy;
    //    //wScale *= 100.0f;

    //    ////wScale *= wScale * abs(sin(uTime * 0.1f) * 3.0f);

    //    //vec2 F = worley(wScale + vec2(vPosition), 0.8f);

    //    vec2 wSt = st;

    //    wSt *= 110;

    //    vec2 F = worley(wSt, 0.8f);

    //    //Closest and second closest
    //    float F1 = F.x;
    //    float F2 = F.y;

    //    float n = 1.0 - 1.5 * F1;

    //    vec4 worleyNoiseTex = vec4(n * n * n * colour.xyz, 0.01);

    //    //colour = mix(colour, worleyNoiseTex, 0.3);

    //    //colour *= worleyNoiseTex;
    //}
   
    //colour = mix(colour, vec4(0.0, 0.0, 0.0, 0.2), mixValue); //black background for stormy clouds
    //colour = mix(colour, vec4(0.0, 0.3, 0.8, mix(curNucVal, nextNucVal, mixValue * .8)), mixValue);

    //colour.a = mix(curNucVal, nextNucVal, mixValue);

    //fBm
    /*vec2 q = vec2(0.);
    q.x = fbm(st + 0.00 * uTime * 0.003);
    q.y = fbm(st + vec2(1.0));

    vec2 r = vec2(0.);
    r.x = fbm(st + 1.0 * q + vec2(1.7, 9.2) + 0.15 * uTime * 0.003);
    r.y = fbm(st + 1.0 * q + vec2(8.3, 2.8) + 0.126 * uTime * 0.003);

    float f = fbm(st + r);

    vec4 fBmNoiseTex = vec4((f * f * f + .6 * f * f + .5 * f) * colour.xyz, 1.);*/

};