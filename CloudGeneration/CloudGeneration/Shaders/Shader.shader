#shader vertex
#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;

out vec2 vTexCoord;
out vec3 vPosition;
out float vTime;

uniform float uTime;
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;	//model, view, projection - so everything is positioned in 3d space correctly

float BobbingMotion()
{
    float pos = sin(uTime * 0.001) * 0.1;

    return pos;
}

void main()
{
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position.xy + sin(uTime * 0.0001) * 0.5, position.z, 1.0f);	//(x, y, z, w) w divides other vector components
    vTexCoord = texCoord;
    vPosition = position;
    vTime = uTime;
};

#shader fragment
#version 330 core

layout(location = 0) out vec4 colour;

in vec2 vTexCoord;
in vec3 vPosition;
in float vTime;

uniform vec4 uColour;
uniform sampler2D uTexture[2];

uniform float curNucVal;
uniform float nextNucVal;
uniform float randNumber0to1[10];

uniform int textureID;

uniform vec2 uResolution;

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


float fbm(vec2 _st, bool choice) 
{
    float v = 0.0;
    float a = 0.5;

    vec2 shift;

    mat2 rot;
    int NUM_OCTAVES;
    // Rotate to reduce axial bias

    if (choice)
    {
        NUM_OCTAVES = 6;
        shift = vec2(100.0);
        rot = mat2(cos(0.5), sin(0.5), -sin(0.5), cos(0.50));
    }
    else
    {
        NUM_OCTAVES = 2;
        shift = vec2(300.0);
        rot = mat2(sin(0.9), cos(0.9), -cos(0.9), sin(0.90));
    }

    for (int i = 0; i < NUM_OCTAVES; ++i) 
    {
        v += a * noise(_st);
        _st = rot * _st * 2.0 + shift;
        a *= 0.5;
    }

    return v;
}


//void Texture1()
//{
//    //set texture and texture coordinates
//    texColour = texture(uTexture[0], vTexCoord); //find texCoord pixel to sample
//       //texColour *= vec4(0.3, 0.5, 0.3, 1.0);
//    st = vPosition.xy / uResolution.xy;
//
//
//
//}


void main()
{
    vec4 texColour;
    vec4 worleyTex;
    vec4 fBmTex;
    vec4 perlinTex;

    //components of texture coordinates (st) instead of UV for opengl
    vec2 st = vPosition.xy / uResolution.xy * 2.;

    if (textureID == 0)
    {
        texColour = texture(uTexture[0], vTexCoord); //find texCoord pixel to sample
        //texColour *= vec4(0.3, 0.5, 0.3, 1.0);
        st = vPosition.xy / uResolution.xy;
    }
    else if(textureID == 1)
    {
        texColour = texture(uTexture[1], vTexCoord); //find texCoord pixel to sample
        //texColour *= vec4(0.8, 0.2, 0.5, 1.0);
        st = vPosition.xy / uResolution.xy;
    }

    colour = texColour;

    //////Perlin
    {
    //    vec4 perlinTex = colour;

    //    for (int i = 0; i < 1; i++)
    //    {
    //        vec2 pScale = vPosition.xy / uResolution.xy;
    //        pScale *= 0.01f;

    //        perlinTex *= SimplexNoise(pScale, vPosition.xy);
    //    }

    //    colour *= perlinTex;
    }

    ////Worley
    {
        //vec2 wScale = vPosition.xy / uResolution.xy;
        //wScale *= 100.0f;

        ////wScale *= wScale * abs(sin(uTime * 0.1f) * 3.0f);

        //vec2 F = worley(wScale + vec2(vPosition), 0.8f);

        //vec2 wScale = vPosition.xy / uResolution.xy;

        //wScale *= 10;

        //vec2 F = worley(wScale + vec2(uTime * 0.0001), 0.8f);

        ////Closest and second closest
        //float F1 = F.x;
        //float F2 = F.y;

        //float n = 1.0 - 1.5 * F1;

        //vec4 worleyNoiseTex = vec4(n, n, n, 1.0);

        //colour *= worleyNoiseTex;
    }

    //fBm
    {
        //vec2 st = gl_FragCoord.xy / uResolution.xy * 2.;

        //vec2 st = vPosition.xy / uResolution.xy * 2.;

        st *= 210.0;

        vec2 q = vec2(0.);
        vec2 r = vec2(0.);

        if (textureID == 0)
        {
            q.x = fbm(st + 0.00 * vTime * 0.0003, true);
            q.y = fbm(st + vec2(1.0), true);

            r.x = fbm(st + 1.0 * q + vec2(1.7, 9.2) + 0.15 * vTime * 0.0003, false);
            r.y = fbm(st + 1.0 * q + vec2(8.3, 2.8) + 0.126 * vTime * 0.0003, true);
        }
        else
        {
            q.x = fbm(st + vec2(vPosition.x + 5.3, vPosition.y + 9.2) + 0.15 * vTime * 0.0001, true);
            q.y = fbm(st, false);

            r.x = fbm(st + 1.0 * q + vec2(vPosition.x + 8.3, vPosition.y - 2.8) + 0.15 * vTime * 0.0003, false);
            r.y = fbm(st + 1.0 * q + vec2(vPosition.x - 1.7, vPosition.y + 9.2) + 0.126 * vTime * 0.0003, true);
        }

        float f = fbm(st + r, true);

        vec4 fBmNoiseTex = vec4((f * f * f + .6 * f * f + .5 * f) * colour.xyz, 1.);

        colour *= fBmNoiseTex;
    }

    float mixValue;

    if (textureID == 0)
    {
        mixValue = distance(st, vec2(vTexCoord.x * randNumber0to1[3], vTexCoord.y * randNumber0to1[9]));
    }
    else
    {
        mixValue = distance(st, vec2(vTexCoord.x * randNumber0to1[1], vTexCoord.y * randNumber0to1[5]));
    }
    
    colour = mix(colour, vec4(0.0, 0.0, 0.0, 0.2), mixValue);

    //colour.a = mix(curNucVal, nextNucVal, mixValue);

    //if (colour.r < 0.1 && colour.g < 0.1 && colour.b < 0.1)
    //{
    //    //discard;
    //    colour = vec4(0, 0.3, 0.7, 0.05); // fully transparent, won't show a thing
    //}


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