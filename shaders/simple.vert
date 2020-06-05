#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec4 tint;
uniform vec3 ambientColor;
uniform vec3 mainColor;
uniform vec3 mainDirection;

out vec2 ourTexCoord;
out vec4 ourColor;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    vec3 ourMainDirection = (view * vec4(mainDirection, 1.0)).rgb;
    ourTexCoord = aTexCoord;

    mat4 modelView  = view * model;
    mat4 modelViewIt = transpose(inverse(modelView));
    vec3 normal = normalize((modelViewIt * vec4(0, 0, 1, 1)).xyz);
    ourColor.a = tint.a;
    ourColor.rgb = tint.rgb * ambientColor + clamp((tint.rgb * mainColor * dot(ourMainDirection, normal)).xyz, 0, 1);
}