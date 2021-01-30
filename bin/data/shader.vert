#version 410

uniform mat4 modelViewProjectionMatrix;
uniform mat4 normalMat;

uniform sampler2DRect tex0;
uniform float noiseDisplacementFactor;

in vec4 position;
in vec2 texcoord;
in vec3 normal;

out vec4 vPosition;
out vec3 vNormal;

void main()
{

    	// here we get the red channel value from the texture
    	// to use it as displacement along the normal
    	float displacement = texture(tex0, texcoord).r;

    	// use the displacement we created from the texture data
    	// to modify the vertex position
	vec3 newPos = position.xyz;
	newPos += (displacement * noiseDisplacementFactor) * normal;
	
    	gl_Position = modelViewProjectionMatrix * vec4(newPos, 1.0);

	// lighting parameters in worldspace
	vNormal = mat3(normalMat) * normal;
	vPosition = vec4(newPos, 1.0);
}
