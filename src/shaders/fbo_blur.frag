#version 330 core

out vec4 FragColor;
in vec2 TexCord;

uniform sampler2D screen_texture;

const float offset_x = 1.0f / 1920.0f;  
const float offset_y = 1.0f / 1080.0f;  

vec2 offsets[9] = vec2[]
(
    vec2(-offset_x,  offset_y), vec2( 0.0f,    offset_y), vec2( offset_x,  offset_y),
    vec2(-offset_x,  0.0f),     vec2( 0.0f,    0.0f),     vec2( offset_x,  0.0f),
    vec2(-offset_x, -offset_y), vec2( 0.0f,   -offset_y), vec2( offset_x, -offset_y) 
);

float kernel[9] = float[]
(
    1,  1, 1,
    1, -8, 1,
    1,  1, 1
);


const int maxKernelSize = 25; 

float gaussianWeight(float x, float sigma) {
    return exp(-(x * x) / (2.0 * sigma * sigma));
}


vec3 Blur(){
    vec3 color = vec3(0.0f);
    vec2 texSize = textureSize(screen_texture, 0);
    vec2 texelSize = 1.0 / texSize;

     int kernelSize = int(5);
     if (kernelSize % 2 == 0) { 
        kernelSize += 1;
    }
    int halfSize = kernelSize / 2;

    float sigma = float(kernelSize) / 6.0;
    float totalWeight = 0.0;
    
    for (int i = -halfSize; i <= halfSize; ++i) {
        for (int j = -halfSize; j <= halfSize; ++j) {
            float weight = gaussianWeight(float(i), sigma) * gaussianWeight(float(j), sigma);
            totalWeight += weight;
            color += texture(screen_texture, TexCord.st + offsets[i] + vec2(float(i), float(j)) * texelSize).rgb * weight;
        }
    }
    return color;
}

void main()
{ 
    vec3 pixel = Blur();
    FragColor = vec4(pixel, 1.0f);
}