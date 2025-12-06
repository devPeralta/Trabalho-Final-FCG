# Relatório do Trabalho Final - Fundamentos de Computação Gráfica

# https://github.com/devPeralta/Trabalho-Final-FCG

**Disciplina:** INF01047 - Fundamentos de Computação Gráfica  
**Professor:** Eduardo Gastal  
**Aluno:** Pablo Diedrich Peralta (287723) e Taylor Souza Frutuoso da Costa (213957)  
**Instituição:** Universidade Federal do Rio Grande do Sul - Instituto de Informática

---

## 1. Introdução

Este relatório documenta o desenvolvimento e implementação de uma aplicação gráfica 3D interativa desenvolvida como projeto final da disciplina de Fundamentos de Computação Gráfica. O trabalho integra conceitos teóricos e práticos estudados ao longo do semestre, incluindo transformações geométricas, sistemas de câmera, modelos de iluminação, mapeamento de texturas, detecção de colisões e animações baseadas em curvas paramétricas.

A aplicação foi construída utilizando a linguagem C++ em conjunto com a API OpenGL 3.3, seguindo as especificações e requisitos técnicos definidos no enunciado do trabalho. O desenvolvimento incluiu a implementação de sistemas complexos de renderização, física básica para detecção de colisões e mecânicas de interação do usuário, resultando em uma experiência interativa completa e funcional.

O objetivo principal deste documento é apresentar de forma detalhada as técnicas implementadas, decisões de design, estrutura do código e desafios encontrados durante o desenvolvimento, demonstrando a aplicação prática dos conhecimentos adquiridos na disciplina.

---

## 2. Descrição da Aplicação

### 2.1 Conceito do Jogo

A aplicação desenvolvida é um jogo de perseguição e colisão em primeira pessoa ambientado em um recinto fechado. O jogador assume o controle direto de uma câmera livre que simula a perspectiva de uma pessoa segurando uma arma USP. O cenário consiste em uma arena retangular delimitada por quatro paredes e um piso texturizado com dimensões de 100x100 unidades.

O elemento central da jogabilidade são alvos móveis de tiro ao arco que se deslocam autonomamente seguindo trajetórias definidas por curvas de Bézier cúbicas geradas proceduralmente. Os alvos executam movimentos suaves e imprevisíveis através do ambiente, criando um desafio dinâmico para o jogador. A cada colisão bem-sucedida entre o jogador e o alvo, o sistema implementa uma mecânica de progressão única: o alvo reduz seu tamanho pela metade e se teletransporta para uma nova posição aleatória, iniciando imediatamente uma nova trajetória de movimento.

O jogo possui um ciclo de dificuldade progressivo com até 10 fases distintas. Iniciando com um alvo de tamanho considerável (escala 50.0), o objeto diminui exponencialmente a cada colisão até atingir um tamanho mínimo extremamente desafiador na oitava fase. Ao atingir a nona fase (limite antes do reset), o sistema reinicia automaticamente, retornando o alvo ao seu tamanho original e recomeçando o ciclo. Esta mecânica cria um loop infinito de gameplay onde a dificuldade aumenta gradualmente até um ponto crítico, após o qual reinicia, proporcionando rejogabilidade contínua.

Para compensar a crescente dificuldade causada pela redução do alvo, o sistema implementa um mecanismo de assistência progressiva: a partir da quarta fase, o raio de colisão do jogador aumenta em 0.5 unidades por fase adicional. Isso permite que o jogador tenha uma área de captura maior conforme o alvo se torna menor, equilibrando a curva de dificuldade e mantendo o jogo desafiador porém jogável.

### 2.2 Mecânicas Principais

- **Movimentação livre:** O jogador pode se mover pelo cenário usando as teclas WASD, com velocidade constante de 3.0 unidades por segundo baseada em delta time, permitindo exploração fluida do ambiente
- **Câmera em primeira pessoa:** Controle intuitivo da visão através do movimento do mouse, com sensibilidade configurada em 0.005 e limitações de ângulo vertical (±89.4 graus) para evitar gimbal lock
- **Sistema de tiro por raycasting:** Disparo instantâneo através do botão esquerdo do mouse, utilizando detecção de colisão raio-AABB no espaço local do alvo, permitindo acertos precisos mesmo com o alvo em movimento e rotação
- **Alvos dinâmicos:** Movimentação automática e contínua dos alvos seguindo curvas de Bézier cúbicas geradas proceduralmente, com velocidade controlada (0.2 unidades/segundo ao longo do parâmetro t)
- **Sistema de colisão física adaptativo:** Detecção de colisão esfera-esfera entre o jogador e o alvo, com raio do jogador crescente baseado na fase atual (0.5 + bônus por fase > 3) e raio mínimo do alvo garantido em 5.0 unidades
- **Progressão cíclica de dificuldade:** Sistema de 10 fases onde o alvo reduz seu tamanho exponencialmente (divisão por 2^(fase-1)) até atingir a nona fase, quando automaticamente reinicia ao tamanho original, criando um loop infinito de gameplay
- **Câmera alternativa look-at:** Modo secundário de visualização (ativado pela tecla L) que orbita automaticamente em torno do alvo mantendo-o centralizado, útil para observação, análise da trajetória e captura de screenshots
- **Rotação contínua do alvo:** Animação visual constante de 0.5 radianos/segundo que adiciona dinamismo ao objeto e torna mais desafiador prever sua orientação exata
- **Sistema de teletransporte:** Geração procedural de novos caminhos de Bézier com posições aleatórias dentro dos limites da arena (-49 a 49 em X, 1 a 99 em Z), garantindo variedade e imprevisibilidade

---

## 3. Requisitos Técnicos Implementados

### 3.1 Malhas Poligonais Complexas

O projeto utiliza modelos 3D complexos carregados a partir de arquivos no formato .obj:

- **Modelo USP:** Arma em primeira pessoa composta por múltiplas partes (Cube, Cube.001, Cube.002, Cube.003), cada uma com geometria, normais e coordenadas de textura independentes
- **Alvo de tiro:** Modelo detalhado de alvo de arco e flecha (10480_archery_target) contendo centenas de triângulos que formam os anéis concêntricos característicos
- **Geometrias auxiliares:** Cubos com 12 triângulos para paredes e plano subdividido em 2 triângulos para o chão

Os modelos são processados pela biblioteca tinyobjloader, que extrai vértices (coordenadas x, y, z), normais (vetores de superfície) e coordenadas de textura (mapeamento UV). A função `BuildTrianglesAndAddToVirtualScene()` converte cada shape do arquivo .obj em um `SceneObject` armazenado no dicionário global `g_VirtualScene`, incluindo o cálculo de bounding boxes (AABB) para cada objeto.

### 3.2 Transformações Geométricas Controladas pelo Usuário

Diversas transformações são controladas pela interação do jogador:

- **Translação da câmera:** Movimentação WASD no plano horizontal, calculada através de vetores u e w derivados da direção de visão, com componente Y zerada para manter altura constante
- **Rotação da câmera:** Controle através do mouse com ângulos theta (rotação horizontal) e phi (rotação vertical) armazenados em coordenadas esféricas e convertidos para vetores cartesianos
- **Transformações da arma:** Fixada em relação à câmera através de matriz inversa da view, criando efeito de primeira pessoa onde a arma acompanha perfeitamente os movimentos da câmera
- **Transformações do alvo:** Combinação de translação (posição na curva de Bézier), rotação Y (animação contínua) e rotação X fixa (-π/2 para orientação vertical), além de escala variável baseada na fase

Todas as transformações utilizam as funções implementadas no módulo `matrices.h`:

- `Matrix_Translate(x, y, z)`: Cria matriz de translação 4x4
- `Matrix_Rotate_X(angle)`, `Matrix_Rotate_Y(angle)`: Criam matrizes de rotação ao redor dos eixos
- `Matrix_Scale(sx, sy, sz)`: Cria matriz de escala não-uniforme

### 3.3 Câmera Livre e Câmera Look-at

O sistema implementa dois modos de câmera alternáveis pela tecla L:

**Câmera Livre (padrão):**

```cpp
float y = sin(g_CameraPhi);
float z = cos(g_CameraPhi)*cos(g_CameraTheta);
float x = cos(g_CameraPhi)*sin(g_CameraTheta);

glm::vec4 camera_lookat_l = camera_position_c - glm::vec4(x,y,z,0.0f);
g_CameraViewVector = glm::normalize(camera_lookat_l - camera_position_c);
view = Matrix_Camera_View(camera_position_c, g_CameraViewVector, camera_up_vector);
```

A câmera livre permite movimentação independente pelo cenário. A direção de visão é controlada por coordenadas esféricas (theta e phi) que são convertidas em vetores cartesianos. Os vetores u (direita) e w (frente) são calculados através de produtos vetoriais, permitindo movimento relativo à orientação da câmera. A componente Y destes vetores é zerada para garantir movimento apenas no plano horizontal.

**Câmera Look-at:**

```cpp
glm::vec3 lookat_target = g_TargetPosition + glm::vec3(0.0f, 2.0f, 0.0f);

float y = g_CameraDistance * sin(g_CameraPhi);
float z = g_CameraDistance * cos(g_CameraPhi) * cos(g_CameraTheta);
float x = g_CameraDistance * cos(g_CameraPhi) * sin(g_CameraTheta);

glm::vec3 camera_position = lookat_target + glm::vec3(x, y, z);
glm::vec4 view_vector = glm::normalize(glm::vec4(lookat_target, 1.0f) - glm::vec4(camera_position, 1.0f));
view = Matrix_Camera_View(glm::vec4(camera_position, 1.0f), view_vector, camera_up_vector);
```

A câmera look-at orbita em torno do alvo mantendo-o sempre centralizado. A posição da câmera é calculada em coordenadas esféricas relativas ao alvo, com offset vertical de 2.0 unidades. A distância pode ser ajustada via scroll do mouse. Este modo é útil para visualização, depuração e observação da trajetória do alvo.

### 3.4 Instâncias de Objetos

O projeto utiliza múltiplas instâncias de objetos geométricos através do sistema de hierarquia de transformações com pilha de matrizes:

**Paredes do cenário (4 instâncias do mesmo cubo):**

```cpp
// Parede frontal (Z = 0)
model = Matrix_Identity();
model = model * Matrix_Translate(g_TorsoPositionX, g_TorsoPositionY -0.5f, 0.0f);
PushMatrix(model);
model = model * Matrix_Scale(100.0f, -10.0f, 0.5f);
DrawCube(render_as_black_uniform);
PopMatrix(model);

// Parede direita (X = 50)
PushMatrix(model);
model = model * Matrix_Translate(50.0f, 0.0f, 50.0f);
model = model * Matrix_Rotate(angulo_90_rad, glm::vec4(0.0f, 1.0f, 0.0f, 0.0f));
model = model * Matrix_Scale(100.0f, -10.0f, 0.5f);
DrawCube(render_as_black_uniform);
PopMatrix(model);

// Parede esquerda (X = -50)
PushMatrix(model);
model = model * Matrix_Translate(-50.0f, 0.0f, 50.0f);
model = model * Matrix_Rotate(angulo_90_rad, glm::vec4(0.0f, 1.0f, 0.0f, 0.0f));
model = model * Matrix_Scale(100.0f, -10.0f, 0.5f);
DrawCube(render_as_black_uniform);
PopMatrix(model);

// Parede traseira (Z = 100)
PushMatrix(model);
model = model * Matrix_Translate(0.0f, 0.0f, 100.0f);
model = model * Matrix_Scale(100.0f, -10.0f, 0.5f);
DrawCube(render_as_black_uniform);
PopMatrix(model);
```

As quatro paredes são instâncias do mesmo modelo de cubo, cada uma com transformações diferentes (translação, rotação, escala) aplicadas através de uma hierarquia de matrizes gerenciada por uma pilha (`g_MatrixStack`). As funções `PushMatrix()` e `PopMatrix()` permitem salvar e restaurar estados de transformação.

**Componentes da arma USP (4 partes do mesmo modelo):**

```cpp
glUniform1i(g_object_id_uniform, 10); 
DrawVirtualObject("Cube.003");  // Corpo principal

glUniform1i(g_object_id_uniform, 11);
DrawVirtualObject("Cube.002");  // Gatilho/mecanismo

glUniform1i(g_object_id_uniform, 12);
DrawVirtualObject("Cube.001");  // Cano

glUniform1i(g_object_id_uniform, 13);
DrawVirtualObject("Cube");      // Empunhadura
```

Cada componente da arma é renderizado separadamente mas compartilha a mesma matriz de transformação base (posição relativa à câmera), permitindo diferentes IDs de objeto para aplicação de texturas e iluminação específicas.

### 3.5 Três Tipos de Testes de Intersecção

Implementados no arquivo `collisions.cpp`, o projeto utiliza três tipos distintos de testes de colisão:

**1. Esfera-Plano (Sphere-Plane):**

```cpp
bool checkSpherePlaneCollision(const Sphere& sphere, const Plane& plane) {
    float signedDistance = dotproduct(glm::vec4(plane.normal, 0.0f), 
                                     glm::vec4(sphere.center, 0.0f)) + plane.distance;
    return glm::abs(signedDistance) <= sphere.radius;
}
```

Calcula a distância com sinal do centro da esfera ao plano usando produto escalar. A colisão ocorre quando a distância absoluta é menor ou igual ao raio da esfera. Este teste seria utilizado para detectar colisões da câmera com as paredes do cenário, embora não esteja ativamente implementado no código atual.

**2. Raio-AABB (Ray-Axis Aligned Bounding Box):**

```cpp
bool checkRayAABBCollision(const Ray& ray, const AABB& box) {
    glm::vec3 invDir = 1.0f / ray.direction;
    glm::vec3 tMin = (box.min - ray.origin) * invDir;
    glm::vec3 tMax = (box.max - ray.origin) * invDir;
    glm::vec3 t1 = glm::min(tMin, tMax);
    glm::vec3 t2 = glm::max(tMin, tMax);
    float tNear = glm::max(glm::max(t1.x, t1.y), t1.z);
    float tFar = glm::min(glm::min(t2.x, t2.y), t2.z);
    return tNear <= tFar && tFar >= 0.0f;
}
```

Implementa o algoritmo slab method para intersecção raio-caixa. Calcula os parâmetros t de intersecção para cada par de planos paralelos (slabs). A colisão ocorre se os intervalos de t se sobrepõem e a intersecção está à frente da origem do raio. Usado no sistema de tiro para detectar quando o raio do disparo intercepta a bounding box do alvo.

**3. Esfera-Esfera (Sphere-Sphere):**

```cpp
bool checkSphereSphereCollision(const Sphere& sphere1, const Sphere& sphere2) {
    float distance = glm::distance(sphere1.center, sphere2.center);
    float sum_radii = sphere1.radius + sphere2.radius;
    return distance <= sum_radii;
}
```

Calcula a distância euclidiana entre os centros das esferas. A colisão ocorre quando esta distância é menor ou igual à soma dos raios. Este é o teste principal usado no jogo para detectar quando o jogador alcança o alvo. O raio do jogador aumenta progressivamente (base 0.5 + bônus de 0.5 por fase acima de 3), enquanto o raio do alvo tem mínimo garantido de 5.0 unidades.

### 3.6 Modelos de Iluminação Difusa e Blinn-Phong

Os modelos de iluminação são implementados no fragment shader (`shader_fragment.glsl`). O código suporta tanto iluminação difusa (Lambert) quanto especular (Blinn-Phong):

**Iluminação Difusa (Lambert):**

```glsl
vec3 lambert_diffuse_term = kd * I * max(0, dot(n, l));
```

O modelo de Lambert calcula a reflexão difusa baseada no ângulo entre a normal da superfície (n) e a direção da luz (l). A intensidade é máxima quando a luz incide perpendicularmente à superfície e zero quando paralela. O coeficiente kd (obtido da textura) modula a cor refletida, e I representa a intensidade da fonte luminosa.

**Iluminação Especular (Blinn-Phong):**

```glsl
vec3 h = normalize(v + l);  // Half-way vector
vec3 blinn_phong_specular_term = ks * I * pow(max(0, dot(n, h)), q);
```

O modelo Blinn-Phong calcula highlights especulares usando o vetor half-way (h), que é a bissetriz entre a direção de visão (v) e a direção da luz (l). Este método é mais eficiente computacionalmente que Phong tradicional e produz resultados similares. O expoente q controla o tamanho do highlight (valores maiores = highlight menor e mais concentrado).

**Combinação Final:**

```glsl
vec3 color = lambert_diffuse_term + ambient_term + blinn_phong_specular_term;
```

Onde:

- `kd`: coeficiente de reflexão difusa (RGB obtido da textura do objeto)
- `ks`: coeficiente de reflexão especular (tipicamente branco ou cor da luz)
- `ka`: coeficiente de reflexão ambiente (iluminação indireta)
- `I`: intensidade da fonte luminosa (vetor RGB)
- `n`: vetor normal da superfície (normalizado)
- `l`: vetor direção da luz (normalizado)
- `v`: vetor direção de visão (normalizado)
- `h`: vetor half-way = normalize(v + l)
- `q`: expoente especular (controla brilho, valores típicos: 8-256)

Diferentes objetos na cena utilizam diferentes combinações destes modelos baseados em seu `object_id`. Por exemplo, objetos metálicos (como a USP) usam maior componente especular, enquanto objetos foscos (como as paredes) usam principalmente difusa.

### 3.7 Modelos de Interpolação de Phong e Gouraud

O projeto implementa ambos os modelos de interpolação, permitindo comparação visual:

**Interpolação de Gouraud (calculada no vertex shader):**

```glsl
// Em shader_vertex.glsl
vec3 color_gouraud = /* cálculo completo de iluminação por vértice */
color_v = vec4(color_gouraud, 1.0);
```

Na interpolação de Gouraud, a iluminação (Lambert + Blinn-Phong) é calculada integralmente em cada vértice do triângulo. As cores resultantes são então interpoladas linearmente através da superfície do triângulo durante a rasterização. Este método é mais eficiente mas pode produzir artefatos visíveis em superfícies com poucos vértices ou highlights especulares grandes.

**Interpolação de Phong (calculada no fragment shader):**

```glsl
// Em shader_fragment.glsl
vec4 n = normalize(normal);  // Normal interpolada
vec4 l = normalize(/* direção da luz */);
vec4 v = normalize(camera_position - position_world);
// Cálculo de iluminação usando vetores interpolados
```

Na interpolação de Phong, apenas as normais são interpoladas através da superfície. O cálculo completo de iluminação é executado para cada fragmento (pixel) usando a normal interpolada. Isso produz resultados visuais significativamente mais suaves e realistas, especialmente para highlights especulares, mas com custo computacional maior.

**Diferenças práticas:**

- **Gouraud:** Mais rápido, adequado para superfícies com muitos vértices ou iluminação difusa predominante
- **Phong:** Mais lento, essencial para highlights especulares suaves em superfícies curvas
- No projeto, objetos como o alvo usam Phong para melhor qualidade visual, enquanto geometrias simples como paredes poderiam usar Gouraud

### 3.8 Mapeamento de Texturas em Todos os Objetos

O sistema de texturização utiliza múltiplas texturas carregadas e aplicadas a diferentes objetos:

**Texturas carregadas:**

```cpp
LoadTextureImage("../../data/tc-earth_daymap_surface.jpg");      // TextureImage0
LoadTextureImage("../../data/tc-earth_nightmap_citylights.gif"); // TextureImage1
LoadTextureImage("../../data/red_brick_pavers_diff_4k.jpg");     // TextureImage2 - Paredes
LoadTextureImage("../../data/usp_metal.jpg");                    // TextureImage3 - Arma
LoadTextureImage("../../data/target.jpg");                       // TextureImage4 - Alvo
LoadTextureImage("../../data/patterned_cobblestone_diff_4k.jpg"); // TextureImage5 - Chão
```

**Configuração no código principal:**

```cpp
glUseProgram(g_GpuProgramID);
glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage0"), 0);
glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage1"), 1);
glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage2"), 2);
glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage3"), 3);
glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage4"), 4);
glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage5"), 5);
```

**Aplicação no fragment shader:**

```glsl
uniform sampler2D TextureImage0;
uniform sampler2D TextureImage1;
// ... etc

if (object_id == 6) {  // Alvo
    vec3 Kd = texture(TextureImage4, texcoords).rgb;
} else if (object_id == 7) {  // Chão
    vec3 Kd = texture(TextureImage5, texcoords).rgb;
} else if (object_id >= 10 && object_id <= 13) {  // Partes da USP
    vec3 Kd = texture(TextureImage3, texcoords).rgb;
} else if (object_id == 50) {  // Paredes
    vec3 Kd = texture(TextureImage2, texcoords).rgb;
}
```

**Processo de texturização:**

1. Imagens são carregadas do disco usando stb_image
2. Convertidas para espaço de cor sRGB (GL_SRGB8)
3. Mipmaps gerados automaticamente (glGenerateMipmap)
4. Configuração de wrapping (GL_REPEAT) para repetição nas bordas
5. Filtro de magnificação linear e minificação com mipmaps
6. Binding aos texture units (0-5)

Cada objeto possui coordenadas de textura (UV) extraídas dos arquivos .obj através do atributo `texcoord_index`. Durante a renderização, o fragment shader amostra a textura apropriada baseado no `object_id` do objeto sendo renderizado.

### 3.9 Movimentação com Curva Bézier Cúbica

O alvo se movimenta seguindo curvas de Bézier cúbicas, proporcionando trajetórias suaves e naturais:

**Função de cálculo do ponto na curva:**

```cpp
glm::vec3 CalculateBezierPoint(float t, const glm::vec3& p0, const glm::vec3& p1, 
                               const glm::vec3& p2, const glm::vec3& p3) {
    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    glm::vec3 p = uuu * p0;           // (1-t)³ * P0
    p += 3.0f * uu * t * p1;          // 3(1-t)²t * P1
    p += 3.0f * u * tt * p2;          // 3(1-t)t² * P2
    p += ttt * p3;                     // t³ * P3

    return p;
}
```

A fórmula de Bézier cúbica combina quatro pontos de controle (P0, P1, P2, P3) usando o parâmetro t [0,1]. Os coeficientes binomiais (1, 3, 3, 1) garantem interpolação suave. P0 e P3 são os pontos inicial e final da curva, enquanto P1 e P2 controlam a forma intermediária, atuando como "ímãs" que atraem a curva.

**Geração procedural de caminhos:**

```cpp
void GenerateNewBezierPath(bool teleport) {
    float min_x = -49.0f, max_x = 49.0f;
    float min_z = 1.0f, max_z = 99.0f;
    float fixed_y = -0.6f;
    
    auto rand_float = [](float min, float max) {
        float scale = rand() / (float) RAND_MAX;
        return min + scale * (max - min);
    };
    
    glm::vec3 p0;
    if (teleport || g_ControlPoints.empty()) {
        // Teletransporte: nova posição aleatória
        p0 = { rand_float(min_x, max_x), fixed_y, rand_float(min_z, max_z) };
    } else {
        // Continuidade: último ponto vira primeiro
        p0 = g_ControlPoints.back();
    }
    
    g_ControlPoints.clear();
    g_ControlPoints.push_back(p0);
    g_ControlPoints.push_back({ rand_float(min_x, max_x), fixed_y, rand_float(min_z, max_z) });
    g_ControlPoints.push_back({ rand_float(min_x, max_x), fixed_y, rand_float(min_z, max_z) });
    g_ControlPoints.push_back({ rand_float(min_x, max_x), fixed_y, rand_float(min_z, max_z) });
    
    g_BezierT = 0.0f;
}
```

**Atualização contínua no loop principal:**

```cpp
g_BezierT += g_BezierSpeed * deltaTime;  // Incrementa parâmetro t

if (g_BezierT >= 1.0f) {
    GenerateNewBezierPath(false);  // Gera novo caminho com continuidade
}

glm::vec3 p0 = g_ControlPoints[0];
glm::vec3 p1 = g_ControlPoints[1];
glm::vec3 p2 = g_ControlPoints[2];
glm::vec3 p3 = g_ControlPoints[3];

g_TargetPosition = CalculateBezierPoint(g_BezierT, p0, p1, p2, p3);
```

O sistema gera quatro pontos de controle aleatórios dentro dos limites da arena. O parâmetro `teleport` determina se deve haver continuidade (último ponto do caminho anterior vira primeiro do novo) ou teletransporte (posição completamente aleatória). A velocidade de progressão é controlada por `g_BezierSpeed` (0.2), multiplicada pelo delta time para independência de framerate.

### 3.10 Animações Baseadas no Tempo (Delta Time)

Todas as animações e movimentações utilizam delta time para garantir consistência independente da taxa de quadros:

**Cálculo do delta time:**

```cpp
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// No loop principal
float currentFrame = (float)glfwGetTime();
deltaTime = currentFrame - lastFrame;
lastFrame = currentFrame;
```

O delta time representa o tempo decorrido (em segundos) desde o último frame. `glfwGetTime()` retorna o tempo absoluto desde o início do programa. A diferença entre frames consecutivos dá o intervalo de tempo para esse frame específico.

**Aplicação em movimentação da câmera:**

```cpp
float camera_speed = 3.0f;  // unidades por segundo
if(tecla_W_pressionada)
    camera_position_c += w_vector * camera_speed * deltaTime;
if(tecla_A_pressionada)
    camera_position_c -= u_vector * camera_speed * deltaTime;
if(tecla_S_pressionada)
    camera_position_c -= w_vector * camera_speed * deltaTime;
if(tecla_D_pressionada)
    camera_position_c += u_vector * camera_speed * deltaTime;
```

Multiplicar a velocidade pelo delta time garante que o deslocamento seja proporcional ao tempo decorrido. Em um sistema rodando a 60 FPS
(deltaTime ≈ 0.0167s), a câmera move 0.05 unidades por frame. A 30 FPS (deltaTime ≈ 0.033s), move 0.1 unidades por frame, resultando na mesma velocidade percebida de 3 unidades/segundo.

**Aplicação em animações:**

```cpp
g_TargetAngle += 0.5f * deltaTime;  // Rotação: 0.5 radianos/segundo
g_BezierT += g_BezierSpeed * deltaTime;  // Progresso na curva: 0.2 unidades/segundo
```

A rotação do alvo acumula 0.5 radianos (≈28.6°) por segundo, independente do framerate. O progresso na curva de Bézier avança uniformemente, completando a curva (t: 0→1) em exatamente 5 segundos (1/0.2).

**Importância:** Sem delta time, um jogo rodando a 30 FPS seria duas vezes mais lento que um a 60 FPS, criando inconsistência entre diferentes hardwares. Com delta time, a física, movimento e animações são independentes da taxa de quadros, garantindo experiência uniforme em qualquer sistema.

---

## 4. Estrutura do Código

### 4.1 Arquitetura Geral

O projeto segue uma estrutura modular com separação clara de responsabilidades:

```
src/
├── main.cpp              # Loop principal e lógica do jogo
├── shader_vertex.glsl    # Vertex shader (transformações e iluminação por vértice)
├── shader_fragment.glsl  # Fragment shader (iluminação por pixel e texturização)
├── collisions.cpp        # Implementação de testes de colisão
└── textrendering.cpp     # Renderização de texto na tela

include/
├── matrices.h            # Funções de transformações matriciais
├── utils.h               # Funções utilitárias (produto vetorial, norma, etc.)
└── collisions.h          # Declarações de estruturas e funções de colisão

data/
├── USP.obj              # Modelo 3D da arma
├── target.obj           # Modelo 3D do alvo
└── *.jpg/*.gif          # Texturas (metálica, tijolo, alvo, paralelepípedo)
```

### 4.2 Estruturas de Dados Principais

**SceneObject (representação de objetos renderizáveis):**

```cpp
struct SceneObject {
    std::string  name;        // Identificador único do objeto
    size_t       first_index; // Índice inicial no array de índices
    size_t       num_indices; // Quantidade de índices (vértices) a renderizar
    GLenum       rendering_mode; // GL_TRIANGLES, GL_LINES, etc.
    GLuint       vertex_array_object_id; // ID do VAO (estado de atributos)
    glm::vec3    bbox_min; // Canto mínimo da AABB
    glm::vec3    bbox_max; // Canto máximo da AABB
};
```

**Estruturas de Colisão (definidas em collisions.h):**

```cpp
struct Sphere {
    glm::vec3 center;  // Centro da esfera no espaço 3D
    float radius;      // Raio da esfera
};

struct Plane {
    glm::vec3 normal;  // Vetor normal ao plano (normalizado)
    float distance;    // Distância da origem ao plano
};

struct Ray {
    glm::vec3 origin;     // Ponto de origem do raio
    glm::vec3 direction;  // Direção do raio (deve ser normalizado)
};

struct AABB {
    glm::vec3 min;  // Canto com coordenadas mínimas
    glm::vec3 max;  // Canto com coordenadas máximas
};
```

**Variáveis Globais Principais:**

```cpp
std::map<std::string, SceneObject> g_VirtualScene;  // Dicionário de objetos renderizáveis
std::stack<glm::mat4> g_MatrixStack;                // Pilha para hierarquia de transformações
std::vector<glm::vec3> g_ControlPoints;             // Pontos de controle da curva de Bézier
glm::vec4 g_CameraPosition;                         // Posição atual da câmera
glm::vec4 g_CameraViewVector;                       // Direção de visão da câmera
float g_CameraTheta, g_CameraPhi;                   // Ângulos esféricos da câmera
int g_TargetPhase;                                  // Fase atual do jogo (1-10)
float g_TargetScale;                                // Escala atual do alvo
```

### 4.3 Pipeline de Renderização

O loop principal executa as seguintes etapas a cada frame:

1. **Preparação do Frame:**
    
    ```cpp
    float currentFrame = (float)glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ```
    
    - Cálculo do delta time
    - Atualização da posição do alvo na curva de Bézier
    - Limpeza dos buffers de cor e profundidade
2. **Configuração da Câmera:**
    
    ```cpp
    // Processamento de input WASD
    camera_position_c += w_vector * camera_speed * deltaTime;
    
    // Cálculo da matriz view
    view = Matrix_Camera_View(camera_position_c, g_CameraViewVector, camera_up_vector);
    
    // Detecção de colisões
    if (checkSphereSphereCollision(cameraSphere, targetSphere)) {
        // Progressão de fase
    }
    ```
    
    - Cálculo das matrizes view e projection
    - Processamento de input (WASD para movimento, mouse para rotação)
    - Detecção de colisões jogador-alvo
3. **Renderização dos Objetos:**
    
    ```cpp
    // Para cada objeto:
    glm::mat4 model = Matrix_Identity();
    model = model * Matrix_Translate(...) * Matrix_Rotate(...) * Matrix_Scale(...);
    glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
    glUniform1i(g_object_id_uniform, object_id);
    DrawVirtualObject("object_name");
    ```
    
    - Chão (plano texturizado com paralelepípedos)
    - Paredes (4 instâncias de cubos rotacionados)
    - Alvo (com transformações dinâmicas: translação na curva, rotação, escala)
    - Arma em primeira pessoa (fixa à câmera via matriz inversa)
4. **Interface e Finalização:**
    
    ```cpp
    TextRendering_ShowEulerAngles(window);
    TextRendering_ShowProjection(window);
    TextRendering_ShowFramesPerSecond(window);
    glfwSwapBuffers(window);
    glfwPollEvents();
    ```
    
    - Renderização de informações de debug (FPS, ângulos, projeção)
    - Troca de buffers (double buffering)
    - Processamento de eventos (teclado, mouse)

---

## 5. Mecânicas de Jogo

### 5.1 Sistema de Progressão Cíclica

O jogo implementa um sistema de progressão com loop infinito:

```cpp
const float INITIAL_TARGET_SCALE = 50.0f;
const int MAX_TARGET_PHASES = 10;
int g_TargetPhase = 1;
float g_TargetScale = INITIAL_TARGET_SCALE;
```

**Lógica de progressão:**

```cpp
if (checkSphereSphereCollision(cameraSphere, targetSphere)) {
    if (g_TargetPhase == MAX_TARGET_PHASES) {
        // Limite máximo atingido (não usado, fase 9 reseta)
    } else {
        g_TargetPhase++;
        if (g_TargetPhase == 9) {
            g_TargetPhase = 1;  // Reset ao atingir fase 9
        }
        g_TargetScale = INITIAL_TARGET_SCALE / pow(2.0f, g_TargetPhase - 1);
        GenerateNewBezierPath(true);  // Teletransporte
    }
}
```

**Curva de dificuldade:**

- **Fase 1:** Escala 50.0 (alvo gigante, muito fácil)
- **Fase 2:** Escala 25.0 (redução de 50%)
- **Fase 3:** Escala 12.5
- **Fase 4:** Escala 6.25
- **Fase 5:** Escala 3.125
- **Fase 6:** Escala 1.5625
- **Fase 7:** Escala 0.78125
- **Fase 8:** Escala 0.390625 (alvo minúsculo, extremamente difícil)
- **Fase 9:** Reset → Fase 1 (escala 50.0)

O alvo diminui exponencialmente (divisão por 2) a cada fase, tornando-se progressivamente mais difícil de alcançar. Ao atingir a nona fase, o sistema automaticamente reinicia para a fase 1, criando um loop infinito de gameplay onde a dificuldade aumenta gradualmente até um ponto crítico, após o qual o ciclo recomeça.

### 5.2 Sistema de Assistência Adaptativa

Para balancear a dificuldade crescente, o raio de colisão do jogador aumenta progressivamente:

```cpp
cameraSphere.center = glm::vec3(camera_position_c.x, camera_position_c.y, camera_position_c.z);

float base_radius = 0.5f;
float bonus_radius = 0.0f;
if (g_TargetPhase > 3) {
    bonus_radius = (g_TargetPhase - 3) * 0.5f;
}
cameraSphere.radius = base_radius + bonus_radius;
```

**Progressão do raio do jogador:**

- **Fases 1-3:** 0.5 unidades (raio base)
- **Fase 4:** 1.0 unidades (+0.5 de bônus)
- **Fase 5:** 1.5 unidades (+1.0 de bônus)
- **Fase 6:** 2.0 unidades (+1.5 de bônus)
- **Fase 7:** 2.5 unidades (+2.0 de bônus)
- **Fase 8:** 3.0 unidades (+2.5 de bônus)

Este sistema de assistência garante que, mesmo com o alvo ficando muito pequeno, o jogador ainda tenha uma chance razoável de alcançá-lo, equilibrando desafio com jogabilidade.

### 5.3 Sistema de Tiro (Alternativo)

O disparo utiliza raycasting no espaço local do alvo:

```cpp
if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && !g_UseLookAtCamera) {
    // Construir matriz de transformação do alvo
    glm::mat4 model = Matrix_Identity();
    model = model * Matrix_Translate(g_TargetPosition.x, g_TargetPosition.y, g_TargetPosition.z);
    model = model * Matrix_Rotate_Y(g_TargetAngle);
    model = model * Matrix_Rotate_X(-1.57079632679f);
    model = model * Matrix_Scale(0.015f * g_TargetScale, 0.015f * g_TargetScale, 0.015f * g_TargetScale);
    
    // Transformar raio para espaço local
    glm::mat4 invModel = glm::inverse(model);
    Ray local_ray;
    local_ray.origin = glm::vec3(invModel * glm::vec4(g_CameraPosition, 1.0f));
    local_ray.direction = glm::vec3(invModel * glm::vec4(g_CameraViewVector, 0.0f));
    
    // Testar colisão com AABB do alvo
    AABB target_bbox;
    target_bbox.min = target.bbox_min;
    target_bbox.max = target.bbox_max;
    
    if (checkRayAABBCollision(local_ray, target_bbox)) {
        GenerateNewBezierPath(true);
        g_TargetPosition = g_ControlPoints[0];
    }
}
```

O sistema de tiro transforma o raio da câmera para o espaço local do alvo, permitindo usar a bounding box original do modelo. Ao acertar, o alvo é teletransportado para uma nova posição (primeiro ponto de controle da nova curva de Bézier).

### 5.4 Controles

**Teclado:**
- `W, A, S, D`: Movimentação da câmera (frente, esquerda, trás, direita)
- `L`: Alternar entre câmera livre (primeira pessoa) e look-at (orbita o alvo)

**Mouse:**
- **Movimento:** Controle da direção de visão (theta e phi)
- **Botão esquerdo:** Disparar (sistema alternativo de acerto por raycasting)
- **Scroll:** Ajustar distância da câmera (apenas em modo look-at)

---

## 6. Contribuições Individuais

Com base no código pré-pronto fornecido pelo professor, que já incluía as bibliotecas, as seguintes contribuições foram feitas para aplicar os conceitos de computação gráfica:

- **Pablo Peralta:**
    - Implementação dos sistemas de câmera, incluindo a câmera livre (controlada por WASD e mouse) e a câmera "look-at" para observar o alvo.
    - Desenvolvimento da lógica de movimentação do alvo, utilizando curvas de Bézier para criar trajetórias dinâmicas e em looping.
    - Criação do sistema de colisão, incluindo a detecção de colisão esfera-plano para as paredes e a colisão do tiro (raycasting) com o alvo, resultando no seu teletransporte.
    - Inserção e configuração dos modelos 3D (`.obj`) da arma (USP) e do alvo, aplicando suas respectivas texturas e ajustando a perspectiva em primeira pessoa.
    - Construção do cenário base, com texturas para o chão e posicionamento das paredes.
    - Implementação da movimentação baseada em tempo (delta time) para garantir consistência da velocidade da câmera.
    - Correção de bugs relacionados a transformações, como o cálculo da matriz inversa para a visualização da arma.
- **Taylor Souza Frutuoso da Costa:**
    - Ajustes no `CMakeLists.txt` para garantir a compilação em diferentes sistemas operacionais.

---

## 7. Uso de Inteligência Artificial e Análise Crítica

Durante o desenvolvimento, a Inteligência Artificial foi empregada como uma ferramenta de suporte, focada principalmente em duas áreas: depuração de código e prototipagem de funções.

No quesito de depuração, a IA foi utilizada para identificar erros complexos que surgiam no pipeline gráfico. Por exemplo, quando a colisão entre o tiro e a parede apresentava um comportamento inesperado, a IA foi consultada para analisar o código de intersecção raio-plano e sugerir possíveis falhas lógicas. Da mesma forma, problemas de transformação de coordenadas, como o posicionamento incorreto da arma em relação à câmera, foram resolvidos mais rapidamente com o auxílio da IA para revisar os cálculos matriciais.

Para a prototipagem, a IA serviu como uma ferramenta para gerar rapidamente implementações iniciais de conceitos. Funções como o cálculo de pontos em uma curva de Bézier ou a lógica para a movimentação da câmera baseada em tempo foram inicialmente prototipadas com a IA. Esse código gerado, embora nem sempre perfeito, serviu como um ponto de partida que foi posteriormente refinado, adaptado e integrado ao projeto, acelerando o ciclo de desenvolvimento.

---

## 8. Como Compilar e Executar

O projeto utiliza `make` para compilação e execução. Para compilar o projeto, navegue até o diretório raiz e execute o seguinte comando:

```bash
make
```

Este comando irá compilar o código-fonte e criar o executável em `bin/Linux/main`.

Para executar a aplicação após a compilação, utilize o comando:

```bash
make run
```

Este comando simplesmente executa o arquivo `bin/Linux/main`.
