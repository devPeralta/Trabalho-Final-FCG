//     Universidade Federal do Rio Grande do Sul
//             Instituto de Informática
//       Departamento de Informática Aplicada
//
//    INF01047 Fundamentos de Computação Gráfica
//               Prof. Eduardo Gastal
//
//                   LABORATÓRIO 3
//

#include <cmath>
#include <cstdio>
#include <cstdlib>

// Headers abaixo são específicos de C++
#include <map>
#include <stack>
#include <string>
#include <vector>
#include <limits>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>

// Headers da biblioteca para carregar modelos obj
#include <tiny_obj_loader.h>

// Headers das bibliotecas OpenGL
#include <glad/glad.h>   // Criação de contexto OpenGL 3.3
#include <GLFW/glfw3.h>  // Criação de janelas do sistema operacional

// Headers da biblioteca GLM: criação de matrizes e vetores.
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>

// Headers locais, definidos na pasta "include/"
#include "utils.h"
#include "matrices.h"
#include <set>

// Estrutura que representa um modelo geométrico carregado a partir de um
// arquivo ".obj". Veja https://en.wikipedia.org/wiki/Wavefront_.obj_file .
struct ObjModel
{
    tinyobj::attrib_t                 attrib;
    std::vector<tinyobj::shape_t>     shapes;
    std::vector<tinyobj::material_t>  materials;

    // Este construtor lê o modelo de um arquivo utilizando a biblioteca tinyobjloader.
    // Veja: https://github.com/syoyo/tinyobjloader
    ObjModel(const char* filename, const char* basepath = NULL, bool triangulate = true)
    {
        printf("Carregando objetos do arquivo \"%s\"...\n", filename);

        // Se basepath == NULL, então setamos basepath como o dirname do
        // filename, para que os arquivos MTL sejam corretamente carregados caso
        // estejam no mesmo diretório dos arquivos OBJ.
        std::string fullpath(filename);
        std::string dirname;
        if (basepath == NULL)
        {
            auto i = fullpath.find_last_of("/");
            if (i != std::string::npos)
            {
                dirname = fullpath.substr(0, i+1);
                basepath = dirname.c_str();
            }
        }

        std::string warn;
        std::string err;
        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename, basepath, triangulate);

        if (!err.empty())
            fprintf(stderr, "\n%s\n", err.c_str());

        if (!ret)
            throw std::runtime_error("Erro ao carregar modelo.");

        for (size_t shape = 0; shape < shapes.size(); ++shape)
        {
            if (shapes[shape].name.empty())
            {
                fprintf(stderr,
                        "*********************************************\n"
                        "Erro: Objeto sem nome dentro do arquivo '%s'.\n"
                        "Veja https://www.inf.ufrgs.br/~eslgastal/fcg-faq-etc.html#Modelos-3D-no-formato-OBJ .\n"
                        "*********************************************\n",
                    filename);
                throw std::runtime_error("Objeto sem nome.");
            }
            printf("- Objeto '%s'\n", shapes[shape].name.c_str());
        }

        printf("OK.\n");
    }
};

// Declaração de funções utilizadas para pilha de matrizes de modelagem.
void PushMatrix(glm::mat4 M);
void PopMatrix(glm::mat4& M);

// Declaração de várias funções utilizadas em main().  Essas estão definidas
// logo após a definição de main() neste arquivo.
void DrawCube(GLint render_as_black_uniform); // Desenha um cubo
void BuildTrianglesAndAddToVirtualScene(ObjModel*); // Constrói representação de um ObjModel como malha de triângulos
void ComputeNormals(ObjModel* model); // Computa normais de um ObjModel, caso não existam.
void DrawVirtualObject(const char* object_name); // Desenha um objeto armazenado em g_VirtualScene
GLuint BuildTriangles(); // Constrói triângulos para renderização
void LoadShadersFromFiles(); // Carrega os shaders de vértice e fragmento, criando um programa de GPU
GLuint LoadShader_Vertex(const char* filename);   // Carrega um vertex shader
GLuint LoadShader_Fragment(const char* filename); // Carrega um fragment shader
void LoadShader(const char* filename, GLuint shader_id); // Função utilizada pelas duas acima
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id); // Cria um programa de GPU

// Declaração de funções auxiliares para renderizar texto dentro da janela
// OpenGL. Estas funções estão definidas no arquivo "textrendering.cpp".
void TextRendering_Init();
float TextRendering_LineHeight(GLFWwindow* window);
float TextRendering_CharWidth(GLFWwindow* window);
void TextRendering_PrintString(GLFWwindow* window, const std::string &str, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrix(GLFWwindow* window, glm::mat4 M, float x, float y, float scale = 1.0f);
void TextRendering_PrintVector(GLFWwindow* window, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProduct(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProductMoreDigits(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProductDivW(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);

// Funções abaixo renderizam como texto na janela OpenGL algumas matrizes e
// outras informações do programa. Definidas após main().
void TextRendering_ShowModelViewProjection(GLFWwindow* window, glm::mat4 projection, glm::mat4 view, glm::mat4 model, glm::vec4 p_model);
void TextRendering_ShowEulerAngles(GLFWwindow* window);
void TextRendering_ShowProjection(GLFWwindow* window);
void TextRendering_ShowFramesPerSecond(GLFWwindow* window);

// Funções callback para comunicação com o sistema operacional e interação do
// usuário. Veja mais comentários nas definições das mesmas, abaixo.
void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
void ErrorCallback(int error, const char* description);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

// Definimos uma estrutura que armazenará dados necessários para renderizar
// cada objeto da cena virtual.
struct SceneObject
{
    std::string  name;        // Nome do objeto
    size_t       first_index; // Índice do primeiro vértice dentro do vetor indices[]
    size_t       num_indices; // Número de índices do objeto dentro do vetor indices[]
    GLenum       rendering_mode; // Modo de rasterização (GL_TRIANGLES, GL_TRIANGLE_STRIP, etc.)
    GLuint       vertex_array_object_id; // ID do VAO onde estão armazenados os atributos do modelo
    glm::vec3    bbox_min; // Axis-Aligned Bounding Box do objeto
    glm::vec3    bbox_max;
};

// Abaixo definimos variáveis globais utilizadas em várias funções do código.

// A cena virtual é uma lista de objetos nomeados, guardados em um dicionário
// (map).  Veja dentro da função BuildTrianglesAndAddToVirtualScene() como que são incluídos
// objetos dentro da variável g_VirtualScene, e veja na função main() como
// estes são acessados.
std::map<std::string, SceneObject> g_VirtualScene;

// Pilha que guardará as matrizes de modelagem.
std::stack<glm::mat4>  g_MatrixStack;

// Razão de proporção da janela (largura/altura). Veja função FramebufferSizeCallback().
float g_ScreenRatio = 1.0f;

// Ângulos de Euler que controlam a rotação de um dos cubos da cena virtual
float g_AngleX = 0.0f;
float g_AngleY = 0.0f;
float g_AngleZ = 0.0f;

// "g_LeftMouseButtonPressed = true" se o usuário está com o botão esquerdo do mouse
// pressionado no momento atual. Veja função MouseButtonCallback().
bool g_LeftMouseButtonPressed = false;
bool g_RightMouseButtonPressed = false; // Análogo para botão direito do mouse
bool g_MiddleMouseButtonPressed = false; // Análogo para botão do meio do mouse

// Teclas que definem a movimentação de camera livre
bool tecla_W_pressionada = false;
bool tecla_A_pressionada = false;
bool tecla_S_pressionada = false;
bool tecla_D_pressionada = false;

// Variáveis que definem a câmera em coordenadas esféricas, controladas pelo
// usuário através do mouse (veja função CursorPosCallback()). A posição
// efetiva da câmera é calculada dentro da função main(), dentro do loop de
// renderização.
float g_CameraTheta = 0.0f; // Ângulo no plano ZX em relação ao eixo Z
float g_CameraPhi = 0.0f;   // Ângulo em relação ao eixo Y
float g_CameraDistance = 3.5f; // Distância da câmera para a origem

// Variáveis que controlam rotação do antebraço
float g_ForearmAngleZ = 0.0f;
float g_ForearmAngleX = 0.0f;

// Variáveis que controlam translação do torso
float g_TorsoPositionX = 0.0f;
float g_TorsoPositionY = 0.0f;

// Variável que controla o tipo de projeção utilizada: perspectiva ou ortográfica.
bool g_UsePerspectiveProjection = true;

// Variável que controla se o texto informativo será mostrado na tela.
bool g_ShowInfoText = true;

// Variáveis que definem um programa de GPU (shaders). Veja função LoadShadersFromFiles().
GLuint g_GpuProgramID = 0;
GLint g_model_uniform;
GLint g_view_uniform;
GLint g_projection_uniform;
GLint g_object_id_uniform; // [COPIADO DO main.cpp, LINHA 273]

int main()
{
  // Inicializamos a biblioteca GLFW, utilizada para criar uma janela do
  // sistema operacional, onde poderemos renderizar com OpenGL.
  int success = glfwInit();
  if (!success)
  {
    fprintf(stderr, "ERROR: glfwInit() failed.\n");
    std::exit(EXIT_FAILURE);
  }

  // Definimos o callback para impressão de erros da GLFW no terminal
  glfwSetErrorCallback(ErrorCallback);

  // Pedimos para utilizar OpenGL versão 3.3 (ou superior)
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  // Pedimos para utilizar o perfil "core", isto é, utilizaremos somente as
  // funções modernas de OpenGL.
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Criamos uma janela do sistema operacional, com 800 colunas e 800 linhas
  // de pixels, e com título "INF01047 ...".
  GLFWwindow* window;
  window = glfwCreateWindow(800, 800, "INF01047 - 287723 - Pablo Diedrich Peralta", NULL, NULL);
  if (!window)
  {
    glfwTerminate();
    fprintf(stderr, "ERROR: glfwCreateWindow() failed.\n");
    std::exit(EXIT_FAILURE);
  }

  // Definimos a função de callback que será chamada sempre que o usuário
  // pressionar alguma tecla do teclado ...
  glfwSetKeyCallback(window, KeyCallback);
  // ... ou clicar os botões do mouse ...
  glfwSetMouseButtonCallback(window, MouseButtonCallback);
  // ... ou movimentar o cursor do mouse em cima da janela ...
  glfwSetCursorPosCallback(window, CursorPosCallback);
  // ... ou rolar a "rodinha" do mouse.
  glfwSetScrollCallback(window, ScrollCallback);

  // Definimos a função de callback que será chamada sempre que a janela for
  // redimensionada, por consequência alterando o tamanho do "framebuffer"
  // (região de memória onde são armazenados os pixels da imagem).
  glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
  glfwSetWindowSize(window, 800, 800); // Forçamos a chamada do callback acima, para definir g_ScreenRatio.

  // Indicamos que as chamadas OpenGL deverão renderizar nesta janela
  glfwMakeContextCurrent(window);

  // Carregamento de todas funções definidas por OpenGL 3.3, utilizando a
  // biblioteca GLAD.
  gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

  // Imprimimos no terminal informações sobre a GPU do sistema
  const GLubyte *vendor      = glGetString(GL_VENDOR);
  const GLubyte *renderer    = glGetString(GL_RENDERER);
  const GLubyte *glversion   = glGetString(GL_VERSION);
  const GLubyte *glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION);

  printf("GPU: %s, %s, OpenGL %s, GLSL %s\n", vendor, renderer, glversion, glslversion);

  // Carregamos os shaders de vértices e de fragmentos que serão utilizados
  // para renderização. Veja slides 180-200 do documento Aula_03_Rendering_Pipeline_Grafico.pdf.
  //
  LoadShadersFromFiles();

  // Construímos a representação de um triângulo (cubo original)
  GLuint vertex_array_object_id = BuildTriangles();

  // Carregamos modelos OBJ da pasta data/
  // ObjModel spheremodel("../../data/sphere.obj");
  // ComputeNormals(&spheremodel);
  // BuildTrianglesAndAddToVirtualScene(&spheremodel);

  // ObjModel bunnymodel("../../data/bunny.obj");
  // ComputeNormals(&bunnymodel);
  // BuildTrianglesAndAddToVirtualScene(&bunnymodel);

  ObjModel uspmodel("../../data/USP.obj");
  ComputeNormals(&uspmodel);
  BuildTrianglesAndAddToVirtualScene(&uspmodel);

  // ObjModel cowmodel("../../data/cow.obj");
  // ComputeNormals(&cowmodel);
  // BuildTrianglesAndAddToVirtualScene(&cowmodel);

  // Inicializamos o código para renderização de texto.
  TextRendering_Init();

  // Buscamos o endereço das variáveis definidas dentro do Vertex Shader.
  // Utilizaremos estas variáveis para enviar dados para a placa de vídeo
  // (GPU)! Veja arquivo "shader_vertex.glsl".
  GLint g_model_uniform           = glGetUniformLocation(g_GpuProgramID, "model"); // Variável da matriz "model"
  GLint g_view_uniform            = glGetUniformLocation(g_GpuProgramID, "view"); // Variável da matriz "view" em shader_vertex.glsl
  GLint g_projection_uniform      = glGetUniformLocation(g_GpuProgramID, "projection"); // Variável da matriz "projection" em shader_vertex.glsl
  GLint render_as_black_uniform = glGetUniformLocation(g_GpuProgramID, "render_as_black"); // Variável booleana em shader_vertex.glsl

  // Habilitamos o Z-buffer. Veja slides 104-116 do documento Aula_09_Projecoes.pdf.
  glEnable(GL_DEPTH_TEST);

  // Habilitamos o Backface Culling. Veja slides 8-13 do documento Aula_02_Fundamentos_Matematicos.pdf, slides 23-34 do documento Aula_13_Clipping_and_Culling.pdf e slides 112-123 do documento Aula_14_Laboratorio_3_Revisao.pdf.
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);

  glm::vec4 camera_position_c  = glm::vec4(0.0f, 0.0f, 5.0f, 1.0f);

  // Ficamos em um loop infinito, renderizando, até que o usuário feche a janela
  while (!glfwWindowShouldClose(window))
  {
    // Aqui executamos as operações de renderização

    // Definimos a cor do "fundo" do framebuffer como branco.  Tal cor é
    // definida como coeficientes RGBA: Red, Green, Blue, Alpha; isto é:
    // Vermelho, Verde, Azul, Alpha (valor de transparência).
    // Conversaremos sobre sistemas de cores nas aulas de Modelos de Iluminação.
    //
    //           R     G     B     A
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

    // "Pintamos" todos os pixels do framebuffer com a cor definida acima,
    // e também resetamos todos os pixels do Z-buffer (depth buffer).
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Pedimos para a GPU utilizar o programa de GPU criado acima (contendo
    // os shaders de vértice e fragmentos).
    glUseProgram(g_GpuProgramID);

    // "Ligamos" o VAO. Informamos que queremos utilizar os atributos de
    // vértices apontados pelo VAO criado pela função BuildTriangles(). Veja
    // comentários detalhados dentro da definição de BuildTriangles().
    glBindVertexArray(vertex_array_object_id);

    // Computamos a posição da câmera utilizando coordenadas esféricas.  As
    // variáveis g_CameraDistance, g_CameraPhi, e g_CameraTheta são
    // controladas pelo mouse do usuário. Veja as funções CursorPosCallback()
    // e ScrollCallback().
    float r = g_CameraDistance;
    float y = r*sin(g_CameraPhi);
    float z = r*cos(g_CameraPhi)*cos(g_CameraTheta);
    float x = r*cos(g_CameraPhi)*sin(g_CameraTheta);

    // Abaixo definimos as varáveis que efetivamente definem a câmera virtual.
    glm::vec4 camera_lookat_l = camera_position_c - glm::vec4(x,y,z,0.0f);
    glm::vec4 camera_view_vector = camera_lookat_l - camera_position_c; // Vetor "view", sentido para onde a câmera está virada
    glm::vec4 camera_up_vector   = glm::vec4(0.0f,1.0f,0.0f,0.0f); // Vetor "up" fixado para apontar para o "céu" (eito Y global)
    glm::vec4 u_vector = crossproduct(camera_up_vector, -camera_view_vector);

    if(tecla_W_pressionada)
      camera_position_c += camera_view_vector * 0.05f;
    if(tecla_A_pressionada)
      camera_position_c -= u_vector * 0.05f;
    if(tecla_S_pressionada)
      camera_position_c -= camera_view_vector * 0.05f;
    if(tecla_D_pressionada)
      camera_position_c += u_vector * 0.05f;

    // Computamos a matriz "View" utilizando os parâmetros da câmera para
    // definir o sistema de coordenadas da câmera.  Veja slides 2-14, 184-190 e 236-242 do documento Aula_08_Sistemas_de_Coordenadas.pdf.
    glm::mat4 view = Matrix_Camera_View(camera_position_c, camera_view_vector, camera_up_vector);

    // Agora computamos a matriz de Projeção.
    glm::mat4 projection;

    // Note que, no sistema de coordenadas da câmera, os planos near e far
    // estão no sentido negativo! Veja slides 176-204 do documento Aula_09_Projecoes.pdf.
    float nearplane = -0.1f;  // Posição do "near plane"
    float farplane  = -1000.0f; // Posição do "far plane"

    if (g_UsePerspectiveProjection)
    {
      // Projeção Perspectiva.
      // Para definição do field of view (FOV), veja slides 205-215 do documento Aula_09_Projecoes.pdf.
      float field_of_view = 3.141592 / 3.0f;
      projection = Matrix_Perspective(field_of_view, g_ScreenRatio, nearplane, farplane);
    }
    else
    {
      // Projeção Ortográfica.
      // Para definição dos valores l, r, b, t ("left", "right", "bottom", "top"),
      // PARA PROJEÇÃO ORTOGRÁFICA veja slides 219-224 do documento Aula_09_Projecoes.pdf.
      // Para simular um "zoom" ortográfico, computamos o valor de "t"
      // utilizando a variável g_CameraDistance.
      float t = 1.5f*g_CameraDistance/2.5f;
      float b = -t;
      float r = t*g_ScreenRatio;
      float l = -r;
      projection = Matrix_Orthographic(l, r, b, t, nearplane, farplane);
    }

    // Enviamos as matrizes "view" e "projection" para a placa de vídeo
    // (GPU). Veja o arquivo "shader_vertex.glsl", onde estas são
    // efetivamente aplicadas em todos os pontos.
    glUniformMatrix4fv(g_view_uniform       , 1 , GL_FALSE , glm::value_ptr(view));
    glUniformMatrix4fv(g_projection_uniform , 1 , GL_FALSE , glm::value_ptr(projection));

    // NOTE: TAREFAS DO LABORATÓRIO 3

    // Cada cópia do cubo possui uma matriz de modelagem independente,
    // já que cada cópia estará em uma posição (rotação, escala, ...)
    // diferente em relação ao espaço global (World Coordinates). Veja
    // slides 2-14 e 184-190 do documento Aula_08_Sistemas_de_Coordenadas.pdf.
    //
    // Entretanto, neste laboratório as matrizes de modelagem dos cubos
    // serão construídas de maneira hierárquica, tal que operações em
    // alguns objetos influenciem outros objetos. Por exemplo: ao
    // transladar o torso, a cabeça deve se movimentar junto.
    // Veja slides 243-273 do documento Aula_08_Sistemas_de_Coordenadas.pdf

    glm::mat4 model = Matrix_Identity(); // Transformação inicial = identidade.

    glUniform1i(g_object_id_uniform, 99);

    // Translação inicial do torso
    model = model * Matrix_Translate(g_TorsoPositionX - 1.0f, g_TorsoPositionY + 1.0f, 0.0f);
    // Guardamos matriz model atual na pilha
    PushMatrix(model);
    // Atualizamos a matriz model (multiplicação à direita) para fazer um escalamento do torso
    model = model * Matrix_Scale(0.8f, 1.0f, 0.2f);
    // Enviamos a matriz "model" para a placa de vídeo (GPU). Veja o
    // arquivo "shader_vertex.glsl", onde esta é efetivamente
    // aplicada em todos os pontos.
    glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
    // Desenhamos um cubo. Esta renderização irá executar o Vertex
    // Shader definido no arquivo "shader_vertex.glsl", e o mesmo irá
    // utilizar as matrizes "model", "view" e "projection" definidas
    // acima e já enviadas para a placa de vídeo (GPU).
    DrawCube(render_as_black_uniform); // #### TORSO
                                       // Tiramos da pilha a matriz model guardada anteriormente
    PopMatrix(model);

    PushMatrix(model);
    model = model * Matrix_Translate(0.0f, 0.05f, 0.0f);
    model = model * Matrix_Scale(0.3f, -0.3f, 0.3f);

    model = model // Atualizamos matriz model (multiplicação à direita) com a rotação do braço direito
      * Matrix_Rotate_Z(-g_AngleZ)  // TERCEIRO rotação Z de Euler
      * Matrix_Rotate_Y(-g_AngleY)  // SEGUNDO rotação Y de Euler
      * Matrix_Rotate_X(g_AngleX); // PRIMEIRO rotação X de Euler
                                   //
                                   // Enviamos a matriz "model" para a placa de vídeo (GPU). Veja o
                                   // arquivo "shader_vertex.glsl", onde esta é efetivamente
                                   // aplicada em todos os pontos.
    glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
    // Desenhamos um cubo. Esta renderização irá executar o Vertex
    // Shader definido no arquivo "shader_vertex.glsl", e o mesmo irá
    // utilizar as matrizes "model", "view" e "projection" definidas
    // acima e já enviadas para a placa de vídeo (GPU).
    DrawCube(render_as_black_uniform); // #### CABEÇA
                                       // Tiramos da pilha a matriz model guardada anteriormente
    PopMatrix(model);

    // BRAÇO DIREITO
    PushMatrix(model);
    model = model * Matrix_Translate(-0.55f, 0.0f, 0.0f);
    PushMatrix(model);
    model = model
      * Matrix_Rotate_Z(g_AngleZ)
      * Matrix_Rotate_Y(g_AngleY)
      * Matrix_Rotate_X(g_AngleX);
    PushMatrix(model);
    model = model * Matrix_Scale(0.2f, 0.6f, 0.2f);
    glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
    DrawCube(render_as_black_uniform);
    PopMatrix(model);

    // ANTEBRAÇO DIREITO
    PushMatrix(model);
    model = model * Matrix_Translate(0.0f, -0.65f, 0.0f);
    model = model
      * Matrix_Rotate_Z(g_ForearmAngleZ)
      * Matrix_Rotate_X(g_ForearmAngleX);
    PushMatrix(model);
    model = model * Matrix_Scale(0.2f, 0.6f, 0.2f);
    glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
    DrawCube(render_as_black_uniform);
    PopMatrix(model);

    // MÃO DIREITA
    PushMatrix(model); 
    model = model * Matrix_Translate(0.0f, -0.65f, 0.0f);
    model = model * Matrix_Scale(0.2f, 0.1f, 0.2f);
    glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model)); 
    DrawCube(render_as_black_uniform);
    PopMatrix(model); 
    PopMatrix(model);
    PopMatrix(model);
    PopMatrix(model);

    // BRAÇO ESQUERDO
    PushMatrix(model);
    model = model * Matrix_Translate(0.55f, 0.0f, 0.0f);
    PushMatrix(model);
    model = model
      * Matrix_Rotate_Z(-g_AngleZ)
      * Matrix_Rotate_Y(g_AngleY)
      * Matrix_Rotate_X(g_AngleX);
    PushMatrix(model);
    model = model * Matrix_Scale(0.2f, 0.6f, 0.2f);
    glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
    DrawCube(render_as_black_uniform);
    PopMatrix(model);

    // ANTEBRAÇO ESQUERDO
    PushMatrix(model);
    model = model * Matrix_Translate(0.0f, -0.65f, 0.0f);
    model = model
      * Matrix_Rotate_Z(-g_ForearmAngleZ)
      * Matrix_Rotate_X(g_ForearmAngleX);
    PushMatrix(model);
    model = model * Matrix_Scale(0.2f, 0.6f, 0.2f);
    glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
    DrawCube(render_as_black_uniform);
    PopMatrix(model);

    // MÃO ESQUERDA
    PushMatrix(model);
    model = model * Matrix_Translate(0.0f, -0.65f, 0.0f);
    model = model * Matrix_Scale(0.2f, 0.1f, 0.2f);
    glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
    DrawCube(render_as_black_uniform);
    PopMatrix(model);
    PopMatrix(model);
    PopMatrix(model);
    PopMatrix(model);

    // COXA ESQUERDA
    PushMatrix(model);
    model = model * Matrix_Translate(0.2f, -1.05f, 0.0f);
    model = model * Matrix_Scale(0.3f, 0.75f, 0.3f);
    glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
    DrawCube(render_as_black_uniform); 
    PopMatrix(model);

    // COXA DIREITA
    PushMatrix(model);
    model = model * Matrix_Translate(-0.2f, -1.05f, 0.0f);
    model = model * Matrix_Scale(0.3f, 0.75f, 0.3f);
    glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
    DrawCube(render_as_black_uniform); 
    PopMatrix(model);

    // TORNOZELO ESQUERDO 
    PushMatrix(model);
    model = model * Matrix_Translate(0.2f, -1.85f, 0.0f);
    model = model * Matrix_Scale(0.25f, 0.75f, 0.25f);
    glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
    DrawCube(render_as_black_uniform); 
    PopMatrix(model);

    // TORNOZELO DIREITO
    PushMatrix(model);
    model = model * Matrix_Translate(-0.2f, -1.85f, 0.0f);
    model = model * Matrix_Scale(0.25f, 0.75f, 0.25f);
    glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
    DrawCube(render_as_black_uniform); 
    PopMatrix(model);

    // PE ESQUERDO
    PushMatrix(model);
    model = model * Matrix_Translate(-0.2f, -2.65f, 0.1f);
    model = model * Matrix_Scale(0.25f, 0.1f, 0.45f);
    glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
    DrawCube(render_as_black_uniform); 
    PopMatrix(model);

    // PE DIREITO
    PushMatrix(model);
    model = model * Matrix_Translate(0.2f, -2.65f, 0.1f);
    model = model * Matrix_Scale(0.25f, 0.1f, 0.45f);
    glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
    DrawCube(render_as_black_uniform); 
    PopMatrix(model);

    // Neste ponto a matriz model recuperada é a matriz inicial (translação do torso)

    // Agora queremos desenhar os eixos XYZ de coordenadas GLOBAIS.
    // Para tanto, colocamos a matriz de modelagem igual à identidade.
    // Veja slides 2-14 e 184-190 do documento Aula_08_Sistemas_de_Coordenadas.pdf.

    #define BUNNY 1
    #define USP 2
    #define COW 3

    // USP em primeira pessoa (fixo na tela)
    // Pegamos a matriz inversa da view para "desfazer" a rotação da câmera
    glm::mat4 view_inverse = glm::inverse(view);

    // Extraímos apenas a parte de rotação (removendo a translação)
    glm::mat4 rotation_inverse = view_inverse;
    rotation_inverse[3][0] = 0.0f;
    rotation_inverse[3][1] = 0.0f;
    rotation_inverse[3][2] = 0.0f;

    model = Matrix_Identity();
    model = model * Matrix_Translate(camera_position_c.x, camera_position_c.y, camera_position_c.z);
    model = model * rotation_inverse; // Aplica a inversa da rotação
    model = model * Matrix_Translate(0.4f, -0.4f, -0.8f); // Offset local
    model = model * Matrix_Scale(0.1f, 0.1f, 0.1f);

    glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
    glUniform1i(g_object_id_uniform, USP);

    glUniform1i(g_object_id_uniform, 10); 
    DrawVirtualObject("Cube.003");

    glUniform1i(g_object_id_uniform, 11);
    DrawVirtualObject("Cube.002");

    glUniform1i(g_object_id_uniform, 12);
    DrawVirtualObject("Cube.001");

    glUniform1i(g_object_id_uniform, 13);
    DrawVirtualObject("Cube");

    // Enviamos a nova matriz "model" para a placa de vídeo (GPU). Veja o
    // arquivo "shader_vertex.glsl".
    glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));

    // Pedimos para OpenGL desenhar linhas com largura de 10 pixels.
    glLineWidth(10.0f);

    // Informamos para a placa de vídeo (GPU) que a variável booleana
    // "render_as_black" deve ser colocada como "false". Veja o arquivo
    // "shader_vertex.glsl".
    glUniform1i(render_as_black_uniform, false);

    // Pedimos para a GPU rasterizar os vértices dos eixos XYZ
    // apontados pelo VAO como linhas. Veja a definição de
    // g_VirtualScene["axes"] dentro da função BuildTriangles(), e veja
    // a documentação da função glDrawElements() em
    // http://docs.gl/gl3/glDrawElements.
    glDrawElements(
        g_VirtualScene["axes"].rendering_mode,
        g_VirtualScene["axes"].num_indices,
        GL_UNSIGNED_INT,
        (void*)(g_VirtualScene["axes"].first_index * sizeof(GLuint))
        );

    // "Desligamos" o VAO, evitando assim que operações posteriores venham a
    // alterar o mesmo. Isso evita bugs.
    glBindVertexArray(0);

    // Imprimimos na tela os ângulos de Euler que controlam a rotação do
    // terceiro cubo.
    TextRendering_ShowEulerAngles(window);

    // Imprimimos na informação sobre a matriz de projeção sendo utilizada.
    TextRendering_ShowProjection(window);

    // Imprimimos na tela informação sobre o número de quadros renderizados
    // por segundo (frames per second).
    TextRendering_ShowFramesPerSecond(window);

    // O framebuffer onde OpenGL executa as operações de renderização não
    // é o mesmo que está sendo mostrado para o usuário, caso contrário
    // seria possível ver artefatos conhecidos como "screen tearing". A
    // chamada abaixo faz a troca dos buffers, mostrando para o usuário
    // tudo que foi renderizado pelas funções acima.
    // Veja o link: https://en.wikipedia.org/w/index.php?title=Multiple_buffering&oldid=793452829#Double_buffering_in_computer_graphics
    glfwSwapBuffers(window);

    // Verificamos com o sistema operacional se houve alguma interação do
    // usuário (teclado, mouse, ...). Caso positivo, as funções de callback
    // definidas anteriormente usando glfwSet*Callback() serão chamadas
    // pela biblioteca GLFW.
    glfwPollEvents();
  }

  // Finalizamos o uso dos recursos do sistema operacional
  glfwTerminate();

  // Fim do programa
  return 0;
}

// Função que pega a matriz M e guarda a mesma no topo da pilha
void PushMatrix(glm::mat4 M)
{
  g_MatrixStack.push(M);
}

// Função que remove a matriz atualmente no topo da pilha e armazena a mesma na variável M
void PopMatrix(glm::mat4& M)
{
  if ( g_MatrixStack.empty() )
  {
    M = Matrix_Identity();
  }
  else
  {
    M = g_MatrixStack.top();
    g_MatrixStack.pop();
  }
}

// Função que computa as normais de um ObjModel, caso elas não tenham sido
// especificadas dentro do arquivo ".obj"
void ComputeNormals(ObjModel* model)
{
    if ( !model->attrib.normals.empty() )
        return;

    // Primeiro computamos as normais para todos os TRIÂNGULOS.
    // Segundo, computamos as normais dos VÉRTICES através do método proposto
    // por Gouraud, onde a normal de cada vértice vai ser a média das normais de
    // todas as faces que compartilham este vértice e que pertencem ao mesmo "smoothing group".

    // Obtemos a lista dos smoothing groups que existem no objeto
    std::set<unsigned int> sgroup_ids;
    for (size_t shape = 0; shape < model->shapes.size(); ++shape)
    {
        size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

        for (size_t triangle = 0; triangle < num_triangles; ++triangle)
        {
            unsigned int sgroup = model->shapes[shape].mesh.smoothing_group_ids[triangle];
            sgroup_ids.insert(sgroup);
        }
    }

    size_t num_vertices = model->attrib.vertices.size() / 3;
    model->attrib.normals.reserve( 3*num_vertices );

    // Processamos um smoothing group por vez
    for (const unsigned int & sgroup : sgroup_ids)
    {
        std::vector<int> num_triangles_per_vertex(num_vertices, 0);
        std::vector<glm::vec4> vertex_normals(num_vertices, glm::vec4(0.0f,0.0f,0.0f,0.0f));

        // Acumulamos as normais dos vértices de todos triângulos deste smoothing group
        for (size_t shape = 0; shape < model->shapes.size(); ++shape)
        {
            size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

            for (size_t triangle = 0; triangle < num_triangles; ++triangle)
            {
                unsigned int sgroup_tri = model->shapes[shape].mesh.smoothing_group_ids[triangle];

                if (sgroup_tri != sgroup)
                    continue;

                glm::vec4  vertices[3];
                for (size_t vertex = 0; vertex < 3; ++vertex)
                {
                    tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];
                    const float vx = model->attrib.vertices[3*idx.vertex_index + 0];
                    const float vy = model->attrib.vertices[3*idx.vertex_index + 1];
                    const float vz = model->attrib.vertices[3*idx.vertex_index + 2];
                    vertices[vertex] = glm::vec4(vx,vy,vz,1.0);
                }

                const glm::vec4  a = vertices[0];
                const glm::vec4  b = vertices[1];
                const glm::vec4  c = vertices[2];

                const glm::vec4  n = crossproduct(b-a,c-a);

                for (size_t vertex = 0; vertex < 3; ++vertex)
                {
                    tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];
                    num_triangles_per_vertex[idx.vertex_index] += 1;
                    vertex_normals[idx.vertex_index] += n;
                }
            }
        }

        // Computamos a média das normais acumuladas
        std::vector<size_t> normal_indices(num_vertices, 0);

        for (size_t vertex_index = 0; vertex_index < vertex_normals.size(); ++vertex_index)
        {
            if (num_triangles_per_vertex[vertex_index] == 0)
                continue;

            glm::vec4 n = vertex_normals[vertex_index] / (float)num_triangles_per_vertex[vertex_index];
            n /= norm(n);

            model->attrib.normals.push_back( n.x );
            model->attrib.normals.push_back( n.y );
            model->attrib.normals.push_back( n.z );

            size_t normal_index = (model->attrib.normals.size() / 3) - 1;
            normal_indices[vertex_index] = normal_index;
        }

        // Escrevemos os índices das normais para os vértices dos triângulos deste smoothing group
        for (size_t shape = 0; shape < model->shapes.size(); ++shape)
        {
            size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

            for (size_t triangle = 0; triangle < num_triangles; ++triangle)
            {
                unsigned int sgroup_tri = model->shapes[shape].mesh.smoothing_group_ids[triangle];

                if (sgroup_tri != sgroup)
                    continue;

                for (size_t vertex = 0; vertex < 3; ++vertex)
                {
                    tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];
                    model->shapes[shape].mesh.indices[3*triangle + vertex].normal_index =
                        normal_indices[ idx.vertex_index ];
                }
            }
        }

    }
}

// Constrói triângulos para futura renderização a partir de um ObjModel.
void BuildTrianglesAndAddToVirtualScene(ObjModel* model)
{
    GLuint vertex_array_object_id;
    glGenVertexArrays(1, &vertex_array_object_id);
    glBindVertexArray(vertex_array_object_id);

    std::vector<GLuint> indices;
    std::vector<float>  model_coefficients;
    std::vector<float>  normal_coefficients;
    std::vector<float>  texture_coefficients;

    for (size_t shape = 0; shape < model->shapes.size(); ++shape)
    {
        size_t first_index = indices.size();
        size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

        const float minval = std::numeric_limits<float>::min();
        const float maxval = std::numeric_limits<float>::max();

        glm::vec3 bbox_min = glm::vec3(maxval,maxval,maxval);
        glm::vec3 bbox_max = glm::vec3(minval,minval,minval);

        for (size_t triangle = 0; triangle < num_triangles; ++triangle)
        {
            assert(model->shapes[shape].mesh.num_face_vertices[triangle] == 3);

            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];

                indices.push_back(first_index + 3*triangle + vertex);

                const float vx = model->attrib.vertices[3*idx.vertex_index + 0];
                const float vy = model->attrib.vertices[3*idx.vertex_index + 1];
                const float vz = model->attrib.vertices[3*idx.vertex_index + 2];
                model_coefficients.push_back( vx ); // X
                model_coefficients.push_back( vy ); // Y
                model_coefficients.push_back( vz ); // Z
                model_coefficients.push_back( 1.0f ); // W

                bbox_min.x = std::min(bbox_min.x, vx);
                bbox_min.y = std::min(bbox_min.y, vy);
                bbox_min.z = std::min(bbox_min.z, vz);
                bbox_max.x = std::max(bbox_max.x, vx);
                bbox_max.y = std::max(bbox_max.y, vy);
                bbox_max.z = std::max(bbox_max.z, vz);

                if ( idx.normal_index != -1 )
                {
                    const float nx = model->attrib.normals[3*idx.normal_index + 0];
                    const float ny = model->attrib.normals[3*idx.normal_index + 1];
                    const float nz = model->attrib.normals[3*idx.normal_index + 2];
                    normal_coefficients.push_back( nx ); // X
                    normal_coefficients.push_back( ny ); // Y
                    normal_coefficients.push_back( nz ); // Z
                    normal_coefficients.push_back( 0.0f ); // W
                }

                if ( idx.texcoord_index != -1 )
                {
                    const float u = model->attrib.texcoords[2*idx.texcoord_index + 0];
                    const float v = model->attrib.texcoords[2*idx.texcoord_index + 1];
                    texture_coefficients.push_back( u );
                    texture_coefficients.push_back( v );
                }
            }
        }

        size_t last_index = indices.size() - 1;

        SceneObject theobject;
        theobject.name           = model->shapes[shape].name;
        theobject.first_index    = first_index; // Primeiro índice
        theobject.num_indices    = last_index - first_index + 1; // Número de indices
        theobject.rendering_mode = GL_TRIANGLES;       // Índices correspondem ao tipo de rasterização GL_TRIANGLES.
        theobject.vertex_array_object_id = vertex_array_object_id;

        theobject.bbox_min = bbox_min;
        theobject.bbox_max = bbox_max;

        g_VirtualScene[model->shapes[shape].name] = theobject;
    }

    GLuint VBO_model_coefficients_id;
    glGenBuffers(1, &VBO_model_coefficients_id);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_model_coefficients_id);
    glBufferData(GL_ARRAY_BUFFER, model_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, model_coefficients.size() * sizeof(float), model_coefficients.data());
    GLuint location = 0; // "(location = 0)" em "shader_vertex.glsl"
    GLint  number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
    glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if ( !normal_coefficients.empty() )
    {
        GLuint VBO_normal_coefficients_id;
        glGenBuffers(1, &VBO_normal_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_normal_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, normal_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, normal_coefficients.size() * sizeof(float), normal_coefficients.data());
        
        location = 3; // <--- ALTERE AQUI (era 1)
        
        number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    if ( !texture_coefficients.empty() )
    {
        GLuint VBO_texture_coefficients_id;
        glGenBuffers(1, &VBO_texture_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_texture_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, texture_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, texture_coefficients.size() * sizeof(float), texture_coefficients.data());
        location = 2; // "(location = 2)" em "shader_vertex.glsl"
        number_of_dimensions = 2; // vec2 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    GLuint indices_id;
    glGenBuffers(1, &indices_id);

    // "Ligamos" o buffer. Note que o tipo agora é GL_ELEMENT_ARRAY_BUFFER.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(GLuint), indices.data());

    // "Desligamos" o VAO, evitando assim que operações posteriores venham a
    // alterar o mesmo. Isso evita bugs.
    glBindVertexArray(0);
}

// Função que desenha um objeto armazenado em g_VirtualScene. Veja definição
// dos objetos na função BuildTrianglesAndAddToVirtualScene().
void DrawVirtualObject(const char* object_name)
{
    // "Ligamos" o VAO. Informamos que queremos utilizar os atributos de
    // vértices apontados pelo VAO criado pela função BuildTrianglesAndAddToVirtualScene(). Veja
    // comentários detalhados dentro da definição de BuildTrianglesAndAddToVirtualScene().
    glBindVertexArray(g_VirtualScene[object_name].vertex_array_object_id);

    // Pedimos para a GPU rasterizar os vértices dos eixos XYZ
    // apontados pelo VAO como linhas. Veja a definição de
    // g_VirtualScene[""] dentro da função BuildTrianglesAndAddToVirtualScene(), e veja
    // a documentação da função glDrawElements() em
    // http://docs.gl/gl3/glDrawElements.
    glDrawElements(
        g_VirtualScene[object_name].rendering_mode,
        g_VirtualScene[object_name].num_indices,
        GL_UNSIGNED_INT,
        (void*)(g_VirtualScene[object_name].first_index * sizeof(GLuint))
    );

    // "Desligamos" o VAO, evitando assim que operações posteriores venham a
    // alterar o mesmo. Isso evita bugs.
    glBindVertexArray(0);
}

// Função que desenha um cubo com arestas em preto, definido dentro da função BuildTriangles().
void DrawCube(GLint render_as_black_uniform)
{
  // Informamos para a placa de vídeo (GPU) que a variável booleana
  // "render_as_black" deve ser colocada como "false". Veja o arquivo
  // "shader_vertex.glsl".
  glUniform1i(render_as_black_uniform, false);

  // Pedimos para a GPU rasterizar os vértices do cubo apontados pelo
  // VAO como triângulos, formando as faces do cubo. Esta
  // renderização irá executar o Vertex Shader definido no arquivo
  // "shader_vertex.glsl", e o mesmo irá utilizar as matrizes
  // "model", "view" e "projection" definidas acima e já enviadas
  // para a placa de vídeo (GPU).
  //
  // Veja a definição de g_VirtualScene["cube_faces"] dentro da
  // função BuildTriangles(), e veja a documentação da função
  // glDrawElements() em http://docs.gl/gl3/glDrawElements.
  glDrawElements(
      g_VirtualScene["cube_faces"].rendering_mode, // Veja slides 182-188 do documento Aula_04_Modelagem_Geometrica_3D.pdf
      g_VirtualScene["cube_faces"].num_indices,    //
      GL_UNSIGNED_INT,
      (void*)(g_VirtualScene["cube_faces"].first_index * sizeof(GLuint))
      );

  // Pedimos para OpenGL desenhar linhas com largura de 4 pixels.
  glLineWidth(4.0f);

  // Pedimos para a GPU rasterizar os vértices dos eixos XYZ
  // apontados pelo VAO como linhas. Veja a definição de
  // g_VirtualScene["axes"] dentro da função BuildTriangles(), e veja
  // a documentação da função glDrawElements() em
  // http://docs.gl/gl3/glDrawElements.
  //
  // Importante: estes eixos serão desenhamos com a matriz "model"
  // definida acima, e portanto sofrerão as mesmas transformações
  // geométricas que o cubo. Isto é, estes eixos estarão
  // representando o sistema de coordenadas do modelo (e não o global)!
  glDrawElements(
      g_VirtualScene["axes"].rendering_mode,
      g_VirtualScene["axes"].num_indices,
      GL_UNSIGNED_INT,
      (void*)(g_VirtualScene["axes"].first_index * sizeof(GLuint))
      );

  // Informamos para a placa de vídeo (GPU) que a variável booleana
  // "render_as_black" deve ser colocada como "true". Veja o arquivo
  // "shader_vertex.glsl".
  glUniform1i(render_as_black_uniform, true);

  // Pedimos para a GPU rasterizar os vértices do cubo apontados pelo
  // VAO como linhas, formando as arestas pretas do cubo. Veja a
  // definição de g_VirtualScene["cube_edges"] dentro da função
  // BuildTriangles(), e veja a documentação da função
  // glDrawElements() em http://docs.gl/gl3/glDrawElements.
  glDrawElements(
      g_VirtualScene["cube_edges"].rendering_mode,
      g_VirtualScene["cube_edges"].num_indices,
      GL_UNSIGNED_INT,
      (void*)(g_VirtualScene["cube_edges"].first_index * sizeof(GLuint))
      );
}

// Constrói triângulos para futura renderização (cubo original do Lab 3)
GLuint BuildTriangles()
{
  // Primeiro, definimos os atributos de cada vértice.

  GLfloat model_coefficients[] = {
    // Vértices de um cubo
    //    X      Y     Z     W
    -0.5f,  0.0f,  0.5f, 1.0f, // posição do vértice 0
    -0.5f, -1.0f,  0.5f, 1.0f, // posição do vértice 1
    0.5f, -1.0f,  0.5f, 1.0f, // posição do vértice 2
    0.5f,  0.0f,  0.5f, 1.0f, // posição do vértice 3
    -0.5f,  0.0f, -0.5f, 1.0f, // posição do vértice 4
    -0.5f, -1.0f, -0.5f, 1.0f, // posição do vértice 5
    0.5f, -1.0f, -0.5f, 1.0f, // posição do vértice 6
    0.5f,  0.0f, -0.5f, 1.0f, // posição do vértice 7
                              // Vértices para desenhar o eixo X
                              //    X      Y     Z     W
    0.0f,  0.0f,  0.0f, 1.0f, // posição do vértice 8
    1.0f,  0.0f,  0.0f, 1.0f, // posição do vértice 9
                              // Vértices para desenhar o eixo Y
                              //    X      Y     Z     W
    0.0f,  0.0f,  0.0f, 1.0f, // posição do vértice 10
    0.0f,  1.0f,  0.0f, 1.0f, // posição do vértice 11
                              // Vértices para desenhar o eixo Z
                              //    X      Y     Z     W
    0.0f,  0.0f,  0.0f, 1.0f, // posição do vértice 12
    0.0f,  0.0f,  1.0f, 1.0f, // posição do vértice 13
  };

  GLuint VBO_model_coefficients_id;
  glGenBuffers(1, &VBO_model_coefficients_id);

  GLuint vertex_array_object_id;
  glGenVertexArrays(1, &vertex_array_object_id);

  glBindVertexArray(vertex_array_object_id);

  glBindBuffer(GL_ARRAY_BUFFER, VBO_model_coefficients_id);

  glBufferData(GL_ARRAY_BUFFER, sizeof(model_coefficients), NULL, GL_STATIC_DRAW);

  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(model_coefficients), model_coefficients);

  GLuint location = 0; // "(location = 0)" em "shader_vertex.glsl"
  GLint  number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
  glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);

  glEnableVertexAttribArray(location);

  glBindBuffer(GL_ARRAY_BUFFER, 0);

  GLfloat color_coefficients[] = {
    // Cores dos vértices do cubo
    //  R     G     B     A
    1.0f, 0.5f, 0.0f, 1.0f, // cor do vértice 0
    1.0f, 0.5f, 0.0f, 1.0f, // cor do vértice 1
    0.0f, 0.5f, 1.0f, 1.0f, // cor do vértice 2
    0.0f, 0.5f, 1.0f, 1.0f, // cor do vértice 3
    1.0f, 0.5f, 0.0f, 1.0f, // cor do vértice 4
    1.0f, 0.5f, 0.0f, 1.0f, // cor do vértice 5
    0.0f, 0.5f, 1.0f, 1.0f, // cor do vértice 6
    0.0f, 0.5f, 1.0f, 1.0f, // cor do vértice 7
                            // Cores para desenhar o eixo X
    1.0f, 0.0f, 0.0f, 1.0f, // cor do vértice 8
    1.0f, 0.0f, 0.0f, 1.0f, // cor do vértice 9
                            // Cores para desenhar o eixo Y
    0.0f, 1.0f, 0.0f, 1.0f, // cor do vértice 10
    0.0f, 1.0f, 0.0f, 1.0f, // cor do vértice 11
                            // Cores para desenhar o eixo Z
    0.0f, 0.0f, 1.0f, 1.0f, // cor do vértice 12
    0.0f, 0.0f, 1.0f, 1.0f, // cor do vértice 13
  };
  GLuint VBO_color_coefficients_id;
  glGenBuffers(1, &VBO_color_coefficients_id);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_color_coefficients_id);
  glBufferData(GL_ARRAY_BUFFER, sizeof(color_coefficients), NULL, GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(color_coefficients), color_coefficients);
  location = 1; // "(location = 1)" em "shader_vertex.glsl"
  number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
  glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(location);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  GLuint indices[] = {
    // Definimos os índices dos vértices que definem as FACES de um cubo
    // através de 12 triângulos que serão desenhados com o modo de renderização
    // GL_TRIANGLES.
    0, 1, 2, // triângulo 1 
    7, 6, 5, // triângulo 2 
    3, 2, 6, // triângulo 3 
    4, 0, 3, // triângulo 4 
    4, 5, 1, // triângulo 5 
    1, 5, 6, // triângulo 6 
    0, 2, 3, // triângulo 7 
    7, 5, 4, // triângulo 8 
    3, 6, 7, // triângulo 9 
    4, 3, 7, // triângulo 10
    4, 1, 0, // triângulo 11
    1, 6, 2, // triângulo 12
             // Definimos os índices dos vértices que definem as ARESTAS de um cubo
             // através de 12 linhas que serão desenhadas com o modo de renderização
             // GL_LINES.
    0, 1, // linha 1 
    1, 2, // linha 2 
    2, 3, // linha 3 
    3, 0, // linha 4 
    0, 4, // linha 5 
    4, 7, // linha 6 
    7, 6, // linha 7 
    6, 2, // linha 8 
    6, 5, // linha 9 
    5, 4, // linha 10
    5, 1, // linha 11
    7, 3, // linha 12
          // Definimos os índices dos vértices que definem as linhas dos eixos X, Y,
          // Z, que serão desenhados com o modo GL_LINES.
    8 , 9 , // linha 1
    10, 11, // linha 2
    12, 13  // linha 3
  };

  // Criamos um primeiro objeto virtual (SceneObject) que se refere às faces
  // coloridas do cubo.
  SceneObject cube_faces;
  cube_faces.name           = "cube_faces";
  cube_faces.first_index    = 0; // Primeiro índice está em indices[0]
  cube_faces.num_indices    = 36;       // Último índice está em indices[35]; total de 36 índices.
  cube_faces.rendering_mode = GL_TRIANGLES; // Índices correspondem ao tipo de rasterização GL_TRIANGLES.
  cube_faces.vertex_array_object_id = vertex_array_object_id;

  g_VirtualScene["cube_faces"] = cube_faces;

  // Criamos um segundo objeto virtual (SceneObject) que se refere às arestas
  // pretas do cubo.
  SceneObject cube_edges;
  cube_edges.name           = "cube_edges";
  cube_edges.first_index    = 36; // Primeiro índice está em indices[36]
  cube_edges.num_indices    = 24; // Último índice está em indices[59]; total de 24 índices.
  cube_edges.rendering_mode = GL_LINES; // Índices correspondem ao tipo de rasterização GL_LINES.
  cube_edges.vertex_array_object_id = vertex_array_object_id;

  g_VirtualScene["cube_edges"] = cube_edges;

  // Criamos um terceiro objeto virtual (SceneObject) que se refere aos eixos XYZ.
  SceneObject axes;
  axes.name           = "axes";
  axes.first_index    = 60; // Primeiro índice está em indices[60]
  axes.num_indices    = 6; // Último índice está em indices[65]; total de 6 índices.
  axes.rendering_mode = GL_LINES; // Índices correspondem ao tipo de rasterização GL_LINES.
  axes.vertex_array_object_id = vertex_array_object_id;
  g_VirtualScene["axes"] = axes;

  // Criamos um buffer OpenGL para armazenar os índices acima
  GLuint indices_id;
  glGenBuffers(1, &indices_id);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);

  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), NULL, GL_STATIC_DRAW);

  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(indices), indices);

  glBindVertexArray(0);

  return vertex_array_object_id;
}

// Carrega um Vertex Shader de um arquivo GLSL. Veja definição de LoadShader() abaixo.
GLuint LoadShader_Vertex(const char* filename)
{
  // Criamos um identificador (ID) para este shader, informando que o mesmo
  // será aplicado nos vértices.
  GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);

  // Carregamos e compilamos o shader
  LoadShader(filename, vertex_shader_id);

  // Retorna o ID gerado acima
  return vertex_shader_id;
}

// Carrega um Fragment Shader de um arquivo GLSL . Veja definição de LoadShader() abaixo.
GLuint LoadShader_Fragment(const char* filename)
{
  // Criamos um identificador (ID) para este shader, informando que o mesmo
  // será aplicado nos fragmentos.
  GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

  // Carregamos e compilamos o shader
  LoadShader(filename, fragment_shader_id);

  // Retorna o ID gerado acima
  return fragment_shader_id;
}

// Função auxilar, utilizada pelas duas funções acima. Carrega código de GPU de
// um arquivo GLSL e faz sua compilação.
void LoadShader(const char* filename, GLuint shader_id)
{
  // Lemos o arquivo de texto indicado pela variável "filename"
  // e colocamos seu conteúdo em memória, apontado pela variável
  // "shader_string".
  std::ifstream file;
  try {
    file.exceptions(std::ifstream::failbit);
    file.open(filename);
  } catch ( std::exception& e ) {
    fprintf(stderr, "ERROR: Cannot open file \"%s\".\n", filename);
    std::exit(EXIT_FAILURE);
  }
  std::stringstream shader;
  shader << file.rdbuf();
  std::string str = shader.str();
  const GLchar* shader_string = str.c_str();
  const GLint   shader_string_length = static_cast<GLint>( str.length() );

  // Define o código do shader GLSL, contido na string "shader_string"
  glShaderSource(shader_id, 1, &shader_string, &shader_string_length);

  // Compila o código do shader GLSL (em tempo de execução)
  glCompileShader(shader_id);

  // Verificamos se ocorreu algum erro ou "warning" durante a compilação
  GLint compiled_ok;
  glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compiled_ok);

  GLint log_length = 0;
  glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);

  // Alocamos memória para guardar o log de compilação.
  // A chamada "new" em C++ é equivalente ao "malloc()" do C.
  GLchar* log = new GLchar[log_length];
  glGetShaderInfoLog(shader_id, log_length, &log_length, log);

  // Imprime no terminal qualquer erro ou "warning" de compilação
  if ( log_length != 0 )
  {
    std::string  output;

    if ( !compiled_ok )
    {
      output += "ERROR: OpenGL compilation of \"";
      output += filename;
      output += "\" failed.\n";
      output += "== Start of compilation log\n";
      output += log;
      output += "== End of compilation log\n";
    }
    else
    {
      output += "WARNING: OpenGL compilation of \"";
      output += filename;
      output += "\".\n";
      output += "== Start of compilation log\n";
      output += log;
      output += "== End of compilation log\n";
    }

    fprintf(stderr, "%s", output.c_str());
  }

  // A chamada "delete" em C++ é equivalente ao "free()" do C
  delete [] log;
}

// Função que carrega os shaders de vértices e de fragmentos que serão
// utilizados para renderização. Veja slides 180-200 do documento Aula_03_Rendering_Pipeline_Grafico.pdf.
//
void LoadShadersFromFiles()
{
    // Note que o caminho para os arquivos "shader_vertex.glsl" e
    // "shader_fragment.glsl" estão fixados, sendo que assumimos a existência
    // da seguinte estrutura no sistema de arquivos:
    //
    //    + FCG_Lab_01/
    //    |
    //    +--+ bin/
    //    |  |
    //    |  +--+ Release/  (ou Debug/ ou Linux/)
    //    |     |
    //    |     o-- main.exe
    //    |
    //    +--+ src/
    //       |
    //       o-- shader_vertex.glsl
    //       |
    //       o-- shader_fragment.glsl
    //
    GLuint vertex_shader_id = LoadShader_Vertex("../../src/shader_vertex.glsl");
    GLuint fragment_shader_id = LoadShader_Fragment("../../src/shader_fragment.glsl");

    // Deletamos o programa de GPU anterior, caso ele exista.
    if ( g_GpuProgramID != 0 )
        glDeleteProgram(g_GpuProgramID);

    // Criamos um programa de GPU utilizando os shaders carregados acima.
    g_GpuProgramID = CreateGpuProgram(vertex_shader_id, fragment_shader_id);

    // Buscamos o endereço das variáveis definidas dentro do Vertex Shader.
    // Utilizaremos estas variáveis para enviar dados para a placa de vídeo
    // (GPU)! Veja arquivo "shader_vertex.glsl" e "shader_fragment.glsl".
    g_model_uniform      = glGetUniformLocation(g_GpuProgramID, "model"); // Variável da matriz "model"
    g_view_uniform       = glGetUniformLocation(g_GpuProgramID, "view"); // Variável da matriz "view" em shader_vertex.glsl
    g_projection_uniform = glGetUniformLocation(g_GpuProgramID, "projection"); // Variável da matriz "projection" em shader_vertex.glsl
    g_object_id_uniform  = glGetUniformLocation(g_GpuProgramID, "object_id"); // Variável "object_id" em shader_fragment.glsl
}

// Esta função cria um programa de GPU, o qual contém obrigatoriamente um
// Vertex Shader e um Fragment Shader.
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id)
{
  // Criamos um identificador (ID) para este programa de GPU
  GLuint program_id = glCreateProgram();

  // Definição dos dois shaders GLSL que devem ser executados pelo programa
  glAttachShader(program_id, vertex_shader_id);
  glAttachShader(program_id, fragment_shader_id);

  // Linkagem dos shaders acima ao programa
  glLinkProgram(program_id);

  // Verificamos se ocorreu algum erro durante a linkagem
  GLint linked_ok = GL_FALSE;
  glGetProgramiv(program_id, GL_LINK_STATUS, &linked_ok);

  // Imprime no terminal qualquer erro de linkagem
  if ( linked_ok == GL_FALSE )
  {
    GLint log_length = 0;
    glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_length);

    // Alocamos memória para guardar o log de compilação.
    // A chamada "new" em C++ é equivalente ao "malloc()" do C.
    GLchar* log = new GLchar[log_length];

    glGetProgramInfoLog(program_id, log_length, &log_length, log);

    std::string output;

    output += "ERROR: OpenGL linking of program failed.\n";
    output += "== Start of link log\n";
    output += log;
    output += "\n== End of link log\n";

    // A chamada "delete" em C++ é equivalente ao "free()" do C
    delete [] log;

    fprintf(stderr, "%s", output.c_str());
  }

  // Retornamos o ID gerado acima
  return program_id;
}

// Definição da função que será chamada sempre que a janela do sistema
// operacional for redimensionada, por consequência alterando o tamanho do
// "framebuffer" (região de memória onde são armazenados os pixels da imagem).
void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
  glViewport(0, 0, width, height);

  g_ScreenRatio = (float)width / height;
}

// Variáveis globais que armazenam a última posição do cursor do mouse, para
// que possamos calcular quanto que o mouse se movimentou entre dois instantes
// de tempo. Utilizadas no callback CursorPosCallback() abaixo.
double g_LastCursorPosX, g_LastCursorPosY;

// Função callback chamada sempre que o usuário aperta algum dos botões do mouse
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
  {
    glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
    g_LeftMouseButtonPressed = true;
  }
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
  {
    g_LeftMouseButtonPressed = false;
  }
  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
  {
    glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
    g_RightMouseButtonPressed = true;
  }
  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
  {
    g_RightMouseButtonPressed = false;
  }
  if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS)
  {
    glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
    g_MiddleMouseButtonPressed = true;
  }
  if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE)
  {
    g_MiddleMouseButtonPressed = false;
  }
}

// Função callback chamada sempre que o usuário movimentar o cursor do mouse em
// cima da janela OpenGL.
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
  if (g_LeftMouseButtonPressed)
  {
    float dx = xpos - g_LastCursorPosX;
    float dy = ypos - g_LastCursorPosY;

    g_CameraTheta -= 0.01f*dx;
    g_CameraPhi   += 0.01f*dy;

    float phimax = 3.141592f/2;
    float phimin = -phimax;

    if (g_CameraPhi > phimax)
      g_CameraPhi = phimax;

    if (g_CameraPhi < phimin)
      g_CameraPhi = phimin;

    g_LastCursorPosX = xpos;
    g_LastCursorPosY = ypos;
  }

  if (g_RightMouseButtonPressed)
  {
    float dx = xpos - g_LastCursorPosX;
    float dy = ypos - g_LastCursorPosY;

    g_ForearmAngleZ -= 0.01f*dx;
    g_ForearmAngleX += 0.01f*dy;

    g_LastCursorPosX = xpos;
    g_LastCursorPosY = ypos;
  }

  if (g_MiddleMouseButtonPressed)
  {
    float dx = xpos - g_LastCursorPosX;
    float dy = ypos - g_LastCursorPosY;

    g_TorsoPositionX += 0.01f*dx;
    g_TorsoPositionY -= 0.01f*dy;

    g_LastCursorPosX = xpos;
    g_LastCursorPosY = ypos;
  }
}

// Função callback chamada sempre que o usuário movimenta a "rodinha" do mouse.
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
  g_CameraDistance -= 0.1f*yoffset;

  const float verysmallnumber = std::numeric_limits<float>::epsilon();
  if (g_CameraDistance < verysmallnumber)
    g_CameraDistance = verysmallnumber;
}

// Definição da função que será chamada sempre que o usuário pressionar alguma
// tecla do teclado. Veja http://www.glfw.org/docs/latest/input_guide.html#input_key
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mod)
{
  // =======================
  // Não modifique este loop! Ele é utilizando para correção automatizada dos
  // laboratórios. Deve ser sempre o primeiro comando desta função KeyCallback().
  for (int i = 0; i < 10; ++i)
    if (key == GLFW_KEY_0 + i && action == GLFW_PRESS && mod == GLFW_MOD_SHIFT)
      std::exit(100 + i);
  // =======================

  // Se o usuário pressionar a tecla ESC, fechamos a janela.
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);

  float delta = 3.141592 / 16; // 22.5 graus, em radianos.

  if (key == GLFW_KEY_X && action == GLFW_PRESS)
  {
    g_AngleX += (mod & GLFW_MOD_SHIFT) ? -delta : delta;
  }

  if (key == GLFW_KEY_Y && action == GLFW_PRESS)
  {
    g_AngleY += (mod & GLFW_MOD_SHIFT) ? -delta : delta;
  }
  if (key == GLFW_KEY_Z && action == GLFW_PRESS)
  {
    g_AngleZ += (mod & GLFW_MOD_SHIFT) ? -delta : delta;
  }

  // Se o usuário apertar a tecla espaço, resetamos os ângulos de Euler para zero.
  if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
  {
    g_AngleX = 0.0f;
    g_AngleY = 0.0f;
    g_AngleZ = 0.0f;
    g_ForearmAngleX = 0.0f;
    g_ForearmAngleZ = 0.0f;
    g_TorsoPositionX = 0.0f;
    g_TorsoPositionY = 0.0f;
  }

  // Se o usuário apertar a tecla P, utilizamos projeção perspectiva.
  if (key == GLFW_KEY_P && action == GLFW_PRESS)
  {
    g_UsePerspectiveProjection = true;
  }

  // Se o usuário apertar a tecla O, utilizamos projeção ortográfica.
  if (key == GLFW_KEY_O && action == GLFW_PRESS)
  {
    g_UsePerspectiveProjection = false;
  }

  // Se o usuário apertar a tecla H, fazemos um "toggle" do texto informativo mostrado na tela.
  if (key == GLFW_KEY_H && action == GLFW_PRESS)
  {
    g_ShowInfoText = !g_ShowInfoText;
  }

  if (key == GLFW_KEY_W)
  {
    if (action == GLFW_PRESS)
        tecla_W_pressionada = true;

    else if (action == GLFW_RELEASE)
        tecla_W_pressionada = false;

    else if (action == GLFW_REPEAT)
        ;
  }

  if (key == GLFW_KEY_S)
  {
    if (action == GLFW_PRESS)
        tecla_S_pressionada = true;

    else if (action == GLFW_RELEASE)
        tecla_S_pressionada = false;

    else if (action == GLFW_REPEAT)
        ;
  }

  if (key == GLFW_KEY_A)
  {
    if (action == GLFW_PRESS)
        tecla_A_pressionada = true;

    else if (action == GLFW_RELEASE)
        tecla_A_pressionada = false;

    else if (action == GLFW_REPEAT)
        ;
  }

  if (key == GLFW_KEY_D)
  {
    if (action == GLFW_PRESS)
        tecla_D_pressionada = true;

    else if (action == GLFW_RELEASE)
        tecla_D_pressionada = false;

    else if (action == GLFW_REPEAT)
        ;
  }
}

// Definimos o callback para impressão de erros da GLFW no terminal
void ErrorCallback(int error, const char* description)
{
  fprintf(stderr, "ERROR: GLFW: %s\n", description);
}

// Esta função recebe um vértice com coordenadas de modelo p_model e passa o
// mesmo por todos os sistemas de coordenadas armazenados nas matrizes model,
// view, e projection; e escreve na tela as matrizes e pontos resultantes
// dessas transformações.
void TextRendering_ShowModelViewProjection(
    GLFWwindow* window,
    glm::mat4 projection,
    glm::mat4 view,
    glm::mat4 model,
    glm::vec4 p_model
    )
{
  if ( !g_ShowInfoText )
    return;

  glm::vec4 p_world = model*p_model;
  glm::vec4 p_camera = view*p_world;
  glm::vec4 p_clip = projection*p_camera;
  glm::vec4 p_ndc = p_clip / p_clip.w;

  float pad = TextRendering_LineHeight(window);

  TextRendering_PrintString(window, " Model matrix             Model     In World Coords.", -1.0f, 1.0f-pad, 1.0f);
  TextRendering_PrintMatrixVectorProduct(window, model, p_model, -1.0f, 1.0f-2*pad, 1.0f);

  TextRendering_PrintString(window, "                                        |  ", -1.0f, 1.0f-6*pad, 1.0f);
  TextRendering_PrintString(window, "                            .-----------'  ", -1.0f, 1.0f-7*pad, 1.0f);
  TextRendering_PrintString(window, "                            V              ", -1.0f, 1.0f-8*pad, 1.0f);

  TextRendering_PrintString(window, " View matrix              World     In Camera Coords.", -1.0f, 1.0f-9*pad, 1.0f);
  TextRendering_PrintMatrixVectorProduct(window, view, p_world, -1.0f, 1.0f-10*pad, 1.0f);

  TextRendering_PrintString(window, "                                        |  ", -1.0f, 1.0f-14*pad, 1.0f);
  TextRendering_PrintString(window, "                            .-----------'  ", -1.0f, 1.0f-15*pad, 1.0f);
  TextRendering_PrintString(window, "                            V              ", -1.0f, 1.0f-16*pad, 1.0f);

  TextRendering_PrintString(window, " Projection matrix        Camera                    In NDC", -1.0f, 1.0f-17*pad, 1.0f);
  TextRendering_PrintMatrixVectorProductDivW(window, projection, p_camera, -1.0f, 1.0f-18*pad, 1.0f);

  int width, height;
  glfwGetFramebufferSize(window, &width, &height);

  glm::vec2 a = glm::vec2(-1, -1);
  glm::vec2 b = glm::vec2(+1, +1);
  glm::vec2 p = glm::vec2( 0,  0);
  glm::vec2 q = glm::vec2(width, height);

  glm::mat4 viewport_mapping = Matrix(
      (q.x - p.x)/(b.x-a.x), 0.0f, 0.0f, (b.x*p.x - a.x*q.x)/(b.x-a.x),
      0.0f, (q.y - p.y)/(b.y-a.y), 0.0f, (b.y*p.y - a.y*q.y)/(b.y-a.y),
      0.0f , 0.0f , 1.0f , 0.0f ,
      0.0f , 0.0f , 0.0f , 1.0f
      );

  TextRendering_PrintString(window, "                                                       |  ", -1.0f, 1.0f-22*pad, 1.0f);
  TextRendering_PrintString(window, "                            .--------------------------'  ", -1.0f, 1.0f-23*pad, 1.0f);
  TextRendering_PrintString(window, "                            V                           ", -1.0f, 1.0f-24*pad, 1.0f);

  TextRendering_PrintString(window, " Viewport matrix           NDC      In Pixel Coords.", -1.0f, 1.0f-25*pad, 1.0f);
  TextRendering_PrintMatrixVectorProductMoreDigits(window, viewport_mapping, p_ndc, -1.0f, 1.0f-26*pad, 1.0f);
}

// Escrevemos na tela os ângulos de Euler definidos nas variáveis globais
// g_AngleX, g_AngleY, e g_AngleZ.
void TextRendering_ShowEulerAngles(GLFWwindow* window)
{
  if ( !g_ShowInfoText )
    return;

  float pad = TextRendering_LineHeight(window);

  char buffer[80];
  snprintf(buffer, 80, "Euler Angles rotation matrix = Z(%.2f)*Y(%.2f)*X(%.2f)\n", g_AngleZ, g_AngleY, g_AngleX);

  TextRendering_PrintString(window, buffer, -1.0f+pad/10, -1.0f+2*pad/10, 1.0f);
}

// Escrevemos na tela qual matriz de projeção está sendo utilizada.
void TextRendering_ShowProjection(GLFWwindow* window)
{
  if ( !g_ShowInfoText )
    return;

  float lineheight = TextRendering_LineHeight(window);
  float charwidth = TextRendering_CharWidth(window);

  if ( g_UsePerspectiveProjection )
    TextRendering_PrintString(window, "Perspective", 1.0f-13*charwidth, -1.0f+2*lineheight/10, 1.0f);
  else
    TextRendering_PrintString(window, "Orthographic", 1.0f-13*charwidth, -1.0f+2*lineheight/10, 1.0f);
}

// Escrevemos na tela o número de quadros renderizados por segundo (frames per
// second).
void TextRendering_ShowFramesPerSecond(GLFWwindow* window)
{
  if ( !g_ShowInfoText )
    return;

  // Variáveis estáticas (static) mantém seus valores entre chamadas
  // subsequentes da função!
  static float old_seconds = (float)glfwGetTime();
  static int   ellapsed_frames = 0;
  static char  buffer[20] = "?? fps";
  static int   numchars = 7;

  ellapsed_frames += 1;

  // Recuperamos o número de segundos que passou desde a execução do programa
  float seconds = (float)glfwGetTime();

  // Número de segundos desde o último cálculo do fps
  float ellapsed_seconds = seconds - old_seconds;

  if ( ellapsed_seconds > 1.0f )
  {
    numchars = snprintf(buffer, 20, "%.2f fps", ellapsed_frames / ellapsed_seconds);

    old_seconds = seconds;
    ellapsed_frames = 0;
  }

  float lineheight = TextRendering_LineHeight(window);
  float charwidth = TextRendering_CharWidth(window);

  TextRendering_PrintString(window, buffer, 1.0f-(numchars + 1)*charwidth, 1.0f-lineheight, 1.0f);
}

// set makeprg=cd\ ..\ &&\ make\ run\ >/dev/null
// vim: set spell spelllang=pt_br :
