#version 410

uniform vec3 lightPos;
uniform vec3 camPos;

in vec4 vPosition;
in vec3 vNormal;

out vec4 outCol;

void main()
{

	vec3 lightCol = vec3(1.0);
	vec3 objectCol = vec3(0.004, 0.12, 0.12);

	// ambient component
	float ambientStrength = 0.1;
	vec3 ambient = ambientStrength * lightCol;

	// diffuse component
	vec3 lightDir = normalize(lightPos - vPosition.xyz);
	float diffAmount = max(dot(vNormal, lightDir), 0.0);
	vec3 diffuse = diffAmount * lightCol;

	// specular component
	float specularStrength = 0.5;
	vec3 viewDir = normalize(camPos - vPosition.xyz);
	vec3 reflectDir = reflect(-lightDir, vNormal);
	float specAmount = pow(max(dot(viewDir, reflectDir), 0.0), 128.0);
	vec3 specular = specularStrength * specAmount * lightCol;
 	
    	vec3 result = (ambient + diffuse + specular) * objectCol;

	//gamma correction
	float gamma = 1.0 / 2.2;
	result = pow(result, vec3(gamma));

    	outCol = vec4(result, 1.0);
}
