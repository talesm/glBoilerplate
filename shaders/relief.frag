#version 330 core
in vec2 ourTexCoord;
in vec3 pos;
in vec3 normal;
in vec3 tangent;
in vec3 binormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec4 tint;
uniform float ambient;
uniform float diffuse;
uniform vec4 lightColor;
uniform vec4 lightSource;

out vec4 fragColor;
uniform sampler2D inputTex;   
uniform sampler2D reliefTex;

///Auxiliar
float rayIntersect(vec2 dp, vec2 ds) {
    const int linear_search_steps=15;
	const int binary_search_steps=5;
	float depth_step=1.0/linear_search_steps;

	// current size of search window
	float size=depth_step;
	// current depth position
	float depth=0.0;
	// best match found (starts with last position 1.0)
	float best_depth=1.0;

	for(int i = 0; i < linear_search_steps-1; ++i) {
        depth += size;
        float t = texture(reliefTex, dp+ds*depth).w;

        if(best_depth > 0.996) // if has no depth found
        if(depth >= t)
            best_depth = depth; // store the best depth.
	}
	depth = best_depth;

	//Recurse arround first point (depth) for closest match
	for( int i=0;i<binary_search_steps;i++ ) {
        size *= 0.5;
        float t = texture(reliefTex, dp+ds*depth).w;
        if(depth >= t) {
            best_depth = depth;
            depth -= 2*size;
        }
        depth += size;
    }

    return best_depth;
}

void main()
{
    float tile = 1;
    float depth = 0.0625f;
    vec3 lightPos;
    // if (lightColor.a <= 0.1) {
        lightPos = normalize((projection *  vec4(lightSource.xyz, 1)).xyz);
    // } else {
    //     lightPos = lightSource.xyz;
    // }

    vec3 v  = normalize(pos);
    float a = dot(normal, -v);
    vec3 s  = normalize(vec3(dot(v, binormal), dot(v, tangent), a));
    s *= depth/a;
    vec2 ds = s.xy;
    vec2 dp = ourTexCoord*tile;
    float d = rayIntersect(dp, ds);
    vec2 texCoord = dp + (ds * d);

    //Shadow
    float shadow = 1.0;
    // if (shadowFactor != 0){
        dp += ds * d;
        a = dot(normal, lightPos);
        s  = normalize(vec3(dot(v, binormal), dot(v, tangent), a));s *= depth/a;
        ds = s.xy;
        dp -= ds*d;
        float dl = rayIntersect(dp, ds);
        if( dl < d - 0.05) {
            shadow = 0.151515;
        }
    // }

    vec4 ourColor = texture(inputTex, texCoord );
    if (ourColor.a > 0.125) {
        vec3 dNormal = texture(reliefTex, texCoord).xyz * 2 - 1;
        vec3 nNormal = normalize(binormal*dNormal.x + tangent*dNormal.y + normal*dNormal.z);
        fragColor.a = ourColor.a * tint.a;
        fragColor.rgb = ourColor.rgb*ambient + shadow*clamp((ourColor * tint * vec4(lightColor.xyz, 1) * dot(lightPos, nNormal)).xyz, 0, 1)*diffuse ;
    } else {
        discard;
    }
}
