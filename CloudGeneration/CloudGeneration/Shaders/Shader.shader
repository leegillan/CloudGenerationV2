#shader vertex
#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;

//to fragment
out vec2 vTexCoord;
out vec3 vPosition;
out vec2 vSt;

//vertex uniforms
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

//from setup of texture
layout(location = 0) out vec4 colour;

//vertex shader variables
in vec2 vTexCoord;
in vec3 vPosition;
in vec2 vSt;

//uniforms
uniform vec4 uColour;
uniform int uBackground;
uniform sampler2D uTexture[5];

uniform float curNucVal;
uniform float randNumber0to1[10];
uniform float uNucleationValues[100];
uniform float uNucleationDelta;
uniform float uFTime;

//Nucleation uniforms
uniform int uUsingNuc;

uniform int uRenderOption;

uniform int textureID;

// Permutation polynomial: (34x^2 + x) mod 289
vec4 permute(vec4 x)
{
    return mod((34.0 * x + 1.0) * x, 289.0);
}

//worley function
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

float random(vec2 p)
{
    // e^pi (Gelfond's constant), // 2^sqrt(2) (Gelfond–Schneider constant)
    vec2 K1 = vec2( 23.14069263277926, 2.665144142690225 );

    return fract(cos(dot(p, K1)) * 12345.6789);
}

//random noise for fBm
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

//fractal Brownian motion function
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

//mixes values based on distance from texture coordinate - creates ovally shapes 
float MixValue(vec2 st, int x, int y)
{
    float mixValue = distance(st, vec2((vTexCoord.x - randNumber0to1[y]) * randNumber0to1[x], (vTexCoord.y - randNumber0to1[x]) * randNumber0to1[y]));

    return mixValue;
}

//sends values for different textures to fBm function using domain warping
vec4 fbmNoiseTex(vec2 st, vec2 qTexCoordsX, vec2 qTexCoordsY, vec2 rTexCoordsX, vec2 rTexCoordsY, float offset, int octaves, float shift)
{
    vec2 q = vec2(0.);
    vec2 r = vec2(0.);

    q.x = fbm(st + qTexCoordsX * uFTime * 0.003, octaves, shift, true);
    q.y = fbm(st + qTexCoordsY, octaves, shift, true);

    r.x = fbm(st + 1.0 * q + rTexCoordsX + offset + 0.15 * uFTime * 0.003, octaves, shift, false);
    r.y = fbm(st + 1.0 * q + rTexCoordsY + offset * uFTime * 0.003, octaves, shift, true);

    float f = fbm(st + r - (uFTime * 0.0005), octaves, shift, true);

    vec4 fBmNoiseTex = vec4((f * f + .6 * f * f + .5 * f * f * f) * colour.xyz, 1.0);

    return fBmNoiseTex;
}

//sets up passing of fBm paramaters
void Setupfbm(vec2 st, float scale, vec2 qTexCoordsX, vec2 qTexCoordsY, vec2 rTexCoordsX, vec2 rTexCoordsY, float offset, int octaves, float shift, int mixValueX, int mixValueY)
{
    vec2 fbmSt = vec2(st * scale);

    colour *= fbmNoiseTex(fbmSt, qTexCoordsX, qTexCoordsX, rTexCoordsX, rTexCoordsY, offset, octaves, shift);

    float mixValue = MixValue(fbmSt, mixValueX, mixValueY);

    //colour = mix(colour, uColour, mixValue);   //background mix
}

//sets up worley noise
void SetupWorley(vec2 st, float scale, int offset)
{
    vec2 wSt = st * scale;

    vec2 F = worley(wSt + offset, 0.8f);

    //Closest and second closest
    float F1 = F.x;
    float F2 = F.y;

    float n = 1.0 - 1.5 * F1;

    vec4 worleyNoiseTex = vec4(n, n, n, 0.9);

    //checks if using nucleation is true // if not it uses the noise value as the function for alpha
    if (uUsingNuc == 1)
    {
        float nuc = uNucleationValues[int(F1 * 100)];

        float nucleationRange = nuc / uNucleationDelta;     // nucleation delta = max nucleation from ((1 * 10000) * saturation pressure) when temp is 293 Kelvin

        worleyNoiseTex.a *= nucleationRange;
    }
    else
    {
        worleyNoiseTex.a *= 0.5;
    }
    
 
    //mixes worley texture with current overrall texture
    colour = mix(colour, worleyNoiseTex, 0.5);
}

//Different textures have different paramaters to create diversity in the look
void Texture1()
{
    vec4 texColour = texture(uTexture[0], vTexCoord);
    colour = texColour;

    Setupfbm(vSt, 210., vec2(0.0), vec2(1.0), vec2(1.7, 9.2), vec2(8.3, 2.8), 0.126, 6, 100.0, 3, 9);

    //SetupWorley(vSt, 30., 61);
}
void Texture2()
{
    vec4 texColour = texture(uTexture[1], vTexCoord);
    colour = texColour;

    Setupfbm(vSt, 350., vec2(-0.150, 0.430), vec2(1.0), vec2(-0.340, 0.420), vec2(-0.400, -0.500), 0.0126, 7, 200.0, 4, 1);

    //SetupWorley(vSt, 70., 15);
}
void Texture3()
{
    vec4 texColour = texture(uTexture[2], vTexCoord);
    colour = texColour;

    Setupfbm(vSt, 310., vec2(0.), vec2(-0.750, 0.750), vec2(0.680, -0.910), vec2(0.670, 0.380), 0.326, 8, 150.0, 9, 0);

   // SetupWorley(vSt, 50., 28);
}
void Texture4()
{
    vec4 texColour = texture(uTexture[3], vTexCoord);
    colour = texColour;
  
    Setupfbm(vSt, 270., vec2(0.), vec2(1.), vec2(-0.960, 0.660), vec2(0.2), 0.05, 7, 300.0, 9, 8);

    ///SetupWorley(vSt, 43., 49);
}
void Texture5()
{
    vec4 texColour = texture(uTexture[4], vTexCoord);
    colour = texColour;

    Setupfbm(vSt, 240., vec2(1.), vec2(-0.470, -0.530), vec2(0.), vec2(0.520, -0.590), 0.2, 4, 180.0, 2, 2);

    //SetupWorley(vSt, 60., 0);
}

void main()
{
    //Determines which cloud tectures gets rendered
    if (textureID == 0)
    {
        Texture1();
    }
    else if (textureID == 1)
    {
        Texture2();
    }
    else if (textureID == 2)
    {
        Texture3();
    }
    else if (textureID == 3)
    {
        Texture4();
    }
    else if (textureID == 4)
    {
        Texture5();
    }

    if (uRenderOption == 0)
    {
        colour.a *= curNucVal;
    }
};