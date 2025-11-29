#version 330 core

// ENTRADAS
in vec4 position_world;
in vec4 normal;
in vec4 vertex_color;      // Cor do vértice (do robô)
in vec2 v_TexCoords;       // Coordenadas de textura vindas do Vertex Shader
flat in int v_render_as_black_int; // MODIFICADO: Era "bool"

// UNIFORMS
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform int object_id;
uniform sampler2D TextureImage0; // Textura da Terra de dia
uniform sampler2D TextureImage1; // Textura da Terra de noite
uniform sampler2D TextureImage2;
uniform sampler2D TextureImage3;
uniform sampler2D TextureImage4;

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
        #define TARGET 6
        #define WALL 50

        if ( object_id == SPHERE )
        {
            Kd = vec3(0.8, 0.4, 0.08);
            Ks = vec3(0.0, 0.0, 0.0);
            Ka = vec3(0.4, 0.2, 0.04);
            q = 1.0;
        }
        else if ( object_id == WALL )
        {
            // Triplanar mapping
            vec3 n = normalize(normal.xyz);
            vec3 blend_weights = abs(n);
            blend_weights = blend_weights / (blend_weights.x + blend_weights.y + blend_weights.z);

            float scale = 0.1;
            vec3 color_x = texture(TextureImage2, position_world.yz * scale).rgb;
            vec3 color_y = texture(TextureImage2, position_world.xz * scale).rgb;
            vec3 color_z = texture(TextureImage2, position_world.xy * scale).rgb;

            Kd = color_x * blend_weights.x + color_y * blend_weights.y + color_z * blend_weights.z;

            Ks = vec3(0.1, 0.1, 0.1);
            Ka = Kd * 0.5;
            q = 10.0;
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
            Kd = vec3(0.25, 0.25, 0.27); // Cor fixa para o chão
            Ks = vec3(0.1, 0.1, 0.1);
            Ka = vec3(0.15, 0.15, 0.17);
            q = 10.0;
        }
        else if ( object_id >= USP_PART1 && object_id <= USP_PART4 )
        {
            Kd = texture(TextureImage3, v_TexCoords).rgb;
            Ks = vec3(0.8, 0.8, 0.8);
            Ka = Kd * 0.5;
            q = 32.0;
        }
        else if ( object_id == TARGET )
        {
            Kd = texture(TextureImage4, v_TexCoords).rgb;
            Ks = vec3(0.1, 0.1, 0.1);
            Ka = Kd * 0.5;
            q = 10.0;
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
