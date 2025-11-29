#version 330 core
out vec4 FragColor;
in vec3 localPos;

// The 9 spherical harmonic coefficients (RGB for each)
uniform vec3 L00;   // l=0, m=0
uniform vec3 L1_1;  // l=1, m=-1
uniform vec3 L10;   // l=1, m=0
uniform vec3 L11;   // l=1, m=1
uniform vec3 L2_2;  // l=2, m=-2
uniform vec3 L2_1;  // l=2, m=-1
uniform vec3 L20;   // l=2, m=0
uniform vec3 L21;   // l=2, m=1
uniform vec3 L22;   // l=2, m=2

void main() {
    // normalized surface normal direction
    vec3 n = normalize(localPos);
    float x = n.x, y = n.y, z = n.z;
    
    // constants from equation 13 of the paper
    // these come from the A_l coefficients (aka how much each frequency contributes)
    const float c1 = 0.429043;
    const float c2 = 0.511664;
    const float c3 = 0.743125;
    const float c4 = 0.886227;
    const float c5 = 0.247708;
    
    // evaluate the SH-based irradiance (eq. 13)
    // just a quadratic polynomial in x,y,z
    vec3 irradiance = 
        // constant and linear terms
        c4 * L00 +
        2.0 * c2 * (L11 * x + L1_1 * y + L10 * z) +
        
        // quadratic terms
        c3 * L20 * z * z +
        c1 * L22 * (x * x - y * y) +
        2.0 * c1 * (L2_2 * x * y + L21 * x * z + L2_1 * y * z) -
        c5 * L20;
    
    FragColor = vec4(irradiance, 1.0);
}