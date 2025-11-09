#version 330 core

// ENTRADAS
layout (location = 0) in vec4 model_coefficients;
layout (location = 1) in vec4 color_in;
layout (location = 2) in vec2 texture_coefficients;
layout (location = 3) in vec4 normal_coefficients;

// UNIFORMS
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform bool render_as_black;

// SAÍDAS
out vec4 position_world;
out vec4 normal;
out vec4 vertex_color;
flat out int v_render_as_black_int;

void main()
{
    // Posição final em Coordenadas de Recorte
    gl_Position = projection * view * model * model_coefficients;

    // Posição em Coordenadas do Mundo
    position_world = model * model_coefficients;

    // Normal em Coordenadas do Mundo (usa a entrada da location = 3)
    normal = inverse(transpose(model)) * normal_coefficients;
    normal.w = 0.0;

    // Passa os atributos do robô para o fragment shader
    vertex_color = color_in; // Passa a cor da location = 1
    
    // MODIFICADO: Converte o bool para int
    v_render_as_black_int = render_as_black ? 1 : 0;
}
