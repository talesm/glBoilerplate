#version 330 core
in vec2 ourTexCoord;
in vec3 ourMainDirection;
in vec3 pos;
in vec3 normal;
in vec3 tangent;
in vec3 binormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec4 tint;
uniform vec3 ambientColor;
uniform vec3 mainColor;

out vec4 fragColor;
uniform sampler2D inputTex;   
uniform sampler2D reliefTex;

void main()
{
    vec4 ourColor = texture(inputTex, ourTexCoord );
    if (ourColor.a > 0.125) {
        vec3 dNormal = texture(reliefTex, ourTexCoord).xyz * 2 - 1;
        vec3 nNormal = normalize(binormal*dNormal.x + tangent*dNormal.y + normal*dNormal.z);
        fragColor.a = ourColor.a * tint.a;
        fragColor.rgb = ourColor.rgb*ambientColor + clamp((ourColor * tint * vec4(mainColor, 1) * dot(ourMainDirection, nNormal)).xyz, 0, 1);
    } else {
        discard;
    }
}
