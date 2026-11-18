#version 330

in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;

uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;

out vec3 fragPosition;
out vec2 fragTexCoord;
out vec3 fragNormal;

// Вершины прилипают к сетке низкого разрешения: дрожание как на PS1
const vec2 SNAP_GRID = vec2(160.0, 120.0);

void main()
{
    fragPosition = vec3(matModel * vec4(vertexPosition, 1.0));
    fragTexCoord = vertexTexCoord;
    fragNormal = normalize(vec3(matNormal * vec4(vertexNormal, 0.0)));

    vec4 position = mvp * vec4(vertexPosition, 1.0);
    position.xy = floor(position.xy / position.w * SNAP_GRID) / SNAP_GRID * position.w;
    gl_Position = position;
}
