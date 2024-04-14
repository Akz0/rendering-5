#version 330 core
#extension GL_EXT_gpu_shader4 : enable

out vec4 FragColor;
in vec2 TexCord;

uniform sampler2D screen_texture;
uniform sampler2D k0;
uniform float KuwaharaStrength;
uniform float PostProcessingType = 0.0f;

uniform float N = 4;
uniform float q = 3;

float gaussianWeight(int i, int j) {
    float sigma = 1.0; 
    float distanceSquared = float(i * i + j * j);
    float weight = exp(-distanceSquared / (2.0 * sigma * sigma)) / (2.0 * 3.14159 * sigma * sigma);
    
    return weight;
}

vec4 kuwahara(int radius) {
    vec2 texture_size = textureSize(screen_texture, 0);
    vec2 uv = gl_FragCoord.xy / texture_size;
    float n = float (( radius + 1) * ( radius + 1));

    //store mean of 4 regions.
    vec3 m [4];
    // store variance of 4 regions.
    vec3 s [4];
    for (int k = 0; k < 4; ++ k) {
        m[k] = vec3 (0.0);
        s[k] = vec3 (0.0);
    }

    //Create Regions to scan values
    struct Window { int x1 , y1 , x2 , y2; };
    Window W[4] = Window [4](
        Window ( -radius , -radius , 0, 0 ),
        Window ( 0, -radius , radius , 0 ),
        Window ( 0, 0, radius , radius ),
        Window ( -radius , 0, 0, radius )
    );

    //Calculate Mean
    for (int k = 0; k < 4; ++ k) {
        for (int j = W[k]. y1; j <= W[k].y2; ++ j) {
            for (int i = W[k].x1; i <= W[k]. x2; ++ i) {
                vec3 c = texture2D(screen_texture , uv + vec2(float(i) ,float(j)) / texture_size ). rgb ;
                m[k] += c;
                s[k] += c * c;
            }
        }
    }

    //Calculate Variance of regions and Select the Final Color.
    vec4 color = vec4(0.0);
    float min_sigma2 = 1e+2;

    for (int k = 0; k < 4; ++ k) {
        m[k] /= n;
        s[k] = abs (s[k] / n - m[k] * m[k]);
        float sigma2 = s[k].r + s[k].g + s[k].b;
        if ( sigma2 < min_sigma2 ) {
            min_sigma2 = sigma2 ;
            color = vec4 (m[k], 1.0);
        }
    }

    return color;
}

const float PI = 3.14159265358979323846;
vec4 KuwaharaGeneralized(int radius){
    
    vec2 src_size = textureSize2D(screen_texture, 0);
    vec2 uv = gl_FragCoord.xy / src_size ;
    vec4 m [8];
    vec3 s [8];
    for (int k = 0; k < N; ++ k) {
        m[k] = vec4 (0.0);
        s[k] = vec3 (0.0);
    }

    float piN = 2.0 * PI / float (N);
    mat2 X = mat2(cos (piN ), sin (piN ), -sin (piN ), cos (piN ));

    for ( int j = -radius ; j <= radius ; ++ j ) {
        for ( int i = -radius ; i <= radius ; ++ i ) {
            vec2 v = 0.5 * vec2 (i,j) / float (radius );
            if (dot (v,v) <= 0.25) {
                vec3 c = texture(screen_texture , uv + vec2(i ,j) / src_size ). rgb ;
                for (int k = 0; k < N; ++ k) {
                    float w = gaussianWeight(int(v.x),int(v.y));
                    m[k] += vec4 (c * w , w);
                    s[k] += c * c * w;
                    v *= X;
                }
            }
        }
     }

    vec4 o = vec4 (0.0);
    for (int k = 0; k < N; ++ k) {
        m[k]. rgb /= m[k].w;
        s[k] = abs (s[k] / m[k].w - m[k].rgb * m[k]. rgb );
        float sigma2 = s[k].r + s[k].g + s[k].b;
        float w = 1.0 / (1.0 + pow (255.0 * sigma2 , 0.5 * q));
        o += vec4(m[k].rgb * w, w);
    }

    return vec4(o.rgb / o.w, 1.0);
}
vec4 Edge(){

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

    vec3 color = vec3(0.0f);
    for(int i = 0; i < 9; i++)
        color += vec3(texture(screen_texture, TexCord.st + offsets[i])) * kernel[i];

    return vec4(color,1.0f);
}

void main() {
    vec4 pixel = texture(screen_texture,TexCord);

    if(PostProcessingType==1.0f){
         pixel = kuwahara(int(KuwaharaStrength));
    }

    if(PostProcessingType==2.0f){
        pixel = Edge();
    }

    if(PostProcessingType==3.0f){
        pixel = KuwaharaGeneralized(int(KuwaharaStrength));
    }
    FragColor = pixel;
}
