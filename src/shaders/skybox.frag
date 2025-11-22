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
    
    // ez brightness normalization
    float brightness = length(color);
    color /= (1.0 + brightness * 0.5);
    color = toneMapACES(color);
    
    // fixed gamma
    // should work for most HDRIs
    color = pow(color, vec3(1.0/1.45));
    
    FragColor = vec4(color, 1.0);
}