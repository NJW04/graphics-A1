#version 330 core

in  vec2  TexCoord;
in  vec3  FragPos;
in  vec3  Normal;
in  vec3  L1;
in  vec3  L2;

uniform sampler2D ourTexture;
uniform sampler2D cloudTexture;  
uniform vec3    lightColor1;
uniform vec3    lightColor2;
uniform float   uTime;           // time in seconds
uniform bool    uUseClouds;      // toggle clouds
uniform float   uCloudStrength;   
uniform float   uCloudBrightness; 
uniform float   uCloudSpeed; 

const float Ka        = 0.2;
const float Kd        = 0.7;
const float Ks        = 0.2;
const float shininess = 16.0;

out vec4 FragColor;

vec3 phong(vec3 L, vec3 C, vec3 N, vec3 V) {
    vec3 Ld       = normalize(L);
    vec3 ambient  = Ka * C;
    float diff    = max(dot(N, Ld), 0.0);
    vec3 diffuse  = Kd * diff * C;
    vec3 R        = reflect(-Ld, N);
    float spec    = pow(max(dot(R, V), 0.0), shininess);
    vec3 specular = Ks * spec * C;
    return ambient + diffuse + specular;
}

void main() {
    vec3 base = texture(ourTexture, TexCoord).rgb;

    if(uUseClouds) {
        vec2 c = TexCoord - vec2(0.5);
        float a = uTime * uCloudSpeed;
        mat2 rot = mat2(cos(a), -sin(a),
                        sin(a),  cos(a));
        vec2 cloudUV = rot * c + vec2(0.5);

        cloudUV += vec2(uTime * 0.005, 0.0);

        vec4 cloudS = texture(cloudTexture, cloudUV);
        float alpha = cloudS.a;
        vec3  bright = cloudS.rgb * uCloudBrightness;

        float f = clamp(alpha * uCloudStrength, 0.0, 1.0);
        base = mix(base, bright, f);
    }

    vec3 N = normalize(Normal);
    vec3 V = normalize(-FragPos);
    vec3 Ld1 = phong(L1, lightColor1, N, V);
    vec3 Ld2 = phong(L2, lightColor2, N, V);
    vec3 color = clamp((Ld1 + Ld2) * base, 0.0, 1.0);
    FragColor  = vec4(color, 1.0);
}
