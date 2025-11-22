#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

// ACES approximation
// https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
vec3 toneMapACES(vec3 x) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main() {
    vec3 color = texture(skybox, TexCoords).rgb;
    
    // ez exposure adjustment
    float avgLuminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
    float exposure = 1.0 / (1.0 + avgLuminance * 2.0);
    color *= exposure;

    // tone mapping
    color = toneMapACES(color);
    
    // sRGB gamma correction
    // should work for most HDRIs
    color = pow(color, vec3(1.0/2.2));
    
    FragColor = vec4(color, 1.0);
}