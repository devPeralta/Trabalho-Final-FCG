#version 330 core

// ENTRADAS
in vec4 position_world;
in vec4 normal;
in vec4 vertex_color;      // Cor do vértice (do robô)
flat in int v_render_as_black_int; // MODIFICADO: Era "bool"

// UNIFORMS
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform int object_id;

// SAÍDA
out vec4 color;

void main()
{
    // Caminho de renderização para o ROBÔ (ID 99)
    if ( object_id == 99 ) 
    {
        // MODIFICADO: Checa se o int é 1
        if (v_render_as_black_int == 1)
        {
            color = vec4(0.0, 0.0, 0.0, 1.0); // Arestas pretas
        }
        else
        {
            color = vertex_color; // Cor original do vértice (laranja/azul)
        }
    }
    // CÓDIGO ORIGINAL: Caminho de renderização para ILUMINAÇÃO (Coelho, etc.)
    else 
    {
        // Obtemos a posição da câmera
        vec4 origin = vec4(0.0, 0.0, 0.0, 1.0);
        vec4 camera_position = inverse(view) * origin;
        
        // Vetores de iluminação
        vec4 p = position_world;
        vec4 n = normalize(normal);
        vec4 v = normalize(camera_position - p);
        vec4 l = v; // Luz na câmera
        vec4 r = -l + 2.0 * n * dot(n, l);
        
        // Definição dos materiais
        vec3 Kd;
        vec3 Ks;
        vec3 Ka;
        float q;

        #define BUNNY  1
        #define USP_PART1 10
        #define USP_PART2 11
        #define USP_PART3 12
        #define USP_PART4 13
        #define COW 3
        #define PLANE  4
        #define SPHERE 5

        if ( object_id == SPHERE )
        {
            Kd = vec3(0.8, 0.4, 0.08);
            Ks = vec3(0.0, 0.0, 0.0);
            Ka = vec3(0.4, 0.2, 0.04);
            q = 1.0;
        }
        else if ( object_id == BUNNY )
        {
            Kd = vec3(0.08, 0.4, 0.8);
            Ks = vec3(0.8, 0.8, 0.8);
            Ka = vec3(0.04, 0.2, 0.4);
            q = 32.0;
        }
        else if ( object_id == PLANE )
        {
            Kd = vec3(0.2, 0.2, 0.2);
            Ks = vec3(0.3, 0.3, 0.3);
            Ka = vec3(0.0, 0.0, 0.0);
            q = 20.0;
        }
        else if ( object_id == USP_PART1 )
        {
            Kd = vec3(0.1, 0.1, 0.1); // Cinza escuro (corpo principal)
            Ks = vec3(0.5, 0.5, 0.5);
            Ka = vec3(0.05, 0.05, 0.05);
            q = 50.0;
        }
        else if ( object_id == USP_PART2 )
        {
            Kd = vec3(0.8, 0.6, 0.2); // Dourado (detalhes)
            Ks = vec3(1.0, 1.0, 0.8);
            Ka = vec3(0.4, 0.3, 0.1);
            q = 100.0;
        }
        else if ( object_id == USP_PART3 )
        {
            Kd = vec3(0.3, 0.3, 0.3); // Cinza médio
            Ks = vec3(0.6, 0.6, 0.6);
            Ka = vec3(0.15, 0.15, 0.15);
            q = 60.0;
        }
        else if ( object_id == USP_PART4 )
        {
            Kd = vec3(0.05, 0.05, 0.05); // Preto (grip/cabo)
            Ks = vec3(0.1, 0.1, 0.1);
            Ka = vec3(0.02, 0.02, 0.02);
            q = 20.0;
        }
        else if ( object_id == COW)
        {
            Kd = vec3(0.5, 0.0, 0.5); // Roxo difuso
            Ks = vec3(1.0, 1.0, 1.0); // Brilho branco
            Ka = vec3(0.2, 0.0, 0.2); // Roxo ambiente
            q = 50.0;                 // Bem brilhante
        }
        else
        {
            Kd = vec3(0.0,0.0,0.0);
            Ks = vec3(0.0,0.0,0.0);
            Ka = vec3(0.0,0.0,0.0);
            q = 1.0;
        }

        // Espectro da fonte de iluminação
        vec3 I = vec3(1.0,1.0,1.0);
        // Espectro da luz ambiente
        vec3 Ia = vec3(0.2,0.2,0.2);

        // Termo difuso
        vec3 lambert_diffuse_term = Kd * I * max(0.0, dot(n, l));
        // Termo ambiente
        vec3 ambient_term = Ka * Ia;
        // Termo especular
        vec3 phong_specular_term;
        if (dot(n, l) > 0.0) {
            phong_specular_term  = Ks * I * pow(max(0.0, dot(r, v)), q);
        } else {
            phong_specular_term = vec3(0.0, 0.0, 0.0);
        }

        // Cor final
        color.a = 1;
        color.rgb = lambert_diffuse_term + ambient_term + phong_specular_term;

        // Correção gamma
        color.rgb = pow(color.rgb, vec3(1.0,1.0,1.0)/2.2);
    }
}
