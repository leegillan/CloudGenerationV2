#shader vertex
#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;

out vec2 vTexCoord;
out vec3 vPosition;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;	//model, view, projection - so everything is positioned in 3d space correctly

void main()
{
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0f);	//(x, y, z, w) w divides other vector components
    vTexCoord = texCoord;
    vPosition = position;
};

#shader fragment
#version 330 core

layout(location = 0) out vec4 colour;

in vec2 vTexCoord;
in vec3 vPosition;

uniform vec4 uColour;
uniform sampler2D uTexture;
uniform float uTime;

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

void main()
{
    vec4 texColour = texture(uTexture, vTexCoord); //find texCoord pixel to sample

    colour = texColour;

    //Worley
    vec2 wScale = vPosition.xy / uResolution.xy;

    wScale *= 150.0f;

    //wScale *= wScale * abs(sin(uTime * 0.1f) * 3.0f);

    vec2 F = worley(wScale + vec2(vPosition), 0.8f);

    //Closest and second closest
    float F1 = F.x;
    float F2 = F.y;

    float n = 1.0 - 1.5 * F1;

    vec4 worleyNoiseTex = vec4(n, n, n, 1.0);

    colour = colour * worleyNoiseTex;

    ////Perlin
    //for (int i = 0; i < 1; i++)
    //{
    //    vec2 pScale = vPosition.xy / uResolution.xy;
    //    pScale *= 2000.0f;

    //    colour = colour * SimplexNoise(pScale, vPosition.xy);
    //}

    colour.a = 0.03f;
};