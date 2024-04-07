#version 330 core

out vec4 FragColor;
in vec2 TexCord;

uniform sampler2D screen_texture;

const float offset_x = 1.0 / 1920.0;  
const float offset_y = 1.0 / 1080.0;  

const int maxKernelSize = 25; 

float gaussianWeight(float x, float sigma) {
    return exp(-(x * x) / (2.0 * sigma * sigma));
}

vec3 BilateralBlur() {
    vec2 texSize = textureSize(screen_texture, 0);
    vec2 texelSize = 1.0 / texSize;

    int kernelSize = int(1); // Assuming z-coordinate holds kernel size

    if (kernelSize % 2 == 0) { // Ensure kernel size is odd for symmetry
        kernelSize += 1;
    }

    int halfSize = kernelSize / 2;

    float sigma_d = float(kernelSize) / 6.0; // Adjust the factor as needed
    float sigma_r = 0.1; // Adjust the factor as needed

    vec3 color = vec3(0.0);
    float totalWeight = 0.0;

    for (int i = -halfSize; i <= halfSize; ++i) {
        for (int j = -halfSize; j <= halfSize; ++j) {
            vec2 offset = vec2(float(i), float(j)) * texelSize;
            float spatialWeight = gaussianWeight(length(offset), sigma_d);
            vec3 centerColor = texture(screen_texture, TexCord + offset).rgb;
            vec3 currentColor = texture(screen_texture, TexCord).rgb;
            float rangeWeight = gaussianWeight(length(centerColor - currentColor), sigma_r);
            float weight = spatialWeight * rangeWeight;
            totalWeight += weight;
            color += centerColor * weight;
        }
    }

    return color / totalWeight;
}

void main() {
    vec3 pixel = BilateralBlur();
    FragColor = texture(screen_texture,TexCord);
}
