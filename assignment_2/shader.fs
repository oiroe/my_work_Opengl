#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 objectColor;
uniform vec3 light1Pos;
uniform vec3 light1Color;
uniform vec3 light2Pos;
uniform vec3 light2Color;

void main()
{
    vec3 norm = normalize(Normal);
    
    // Light 1
    vec3 lightDir1 = normalize(light1Pos - FragPos);
    float diff1 = max(dot(norm, lightDir1), 0.0);
    vec3 diffuse1 = diff1 * light1Color;
    
    // Light 2
    vec3 lightDir2 = normalize(light2Pos - FragPos);
    float diff2 = max(dot(norm, lightDir2), 0.0);
    vec3 diffuse2 = diff2 * light2Color;
    
    vec3 ambient = 0.1 * vec3(1.0);
    vec3 result = (ambient + diffuse1 + diffuse2) * objectColor;
    
    FragColor = vec4(result, 1.0);
}