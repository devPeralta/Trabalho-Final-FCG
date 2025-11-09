#version 330 core

// ENTRADAS
layout (location = 0) in vec4 model_coefficients;   // Posição (do robô E do coelho)
layout (location = 1) in vec4 color_in;           // Cor (APENAS do robô)
layout (location = 2) in vec2 texture_coefficients; // (Mantido, mas não usado)
layout (location = 3) in vec4 normal_coefficients;  // Normal (APENAS do coelho)

// UNIFORMS
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform bool render_as_black; // Flag para as arestas do robô

// SAÍDAS
out vec4 position_world;
out vec4 normal;
out vec4 vertex_color;      // Passa a cor do robô
flat out int v_render_as_black_int; // MODIFICADO: Era "bool"

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
