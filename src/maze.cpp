#include "maze.h"
#include "matrices.h"
#include "utils.h" // For PushMatrix and PopMatrix if needed, and other utilities
#include <iostream>
#include <algorithm> // For std::shuffle
#include <glm/gtc/type_ptr.hpp> // For glm::value_ptr

// Constructor
Maze::Maze(int width, int height, float wall_size)
    : width(width), height(height), wall_size(wall_size) {
    grid.resize(height, std::vector<Cell>(width));
}

void Maze::removeWall(Cell& current, Cell& neighbour, int current_x, int current_y, int neighbour_x, int neighbour_y) {
    // Going from current to neighbour
    if (neighbour_x > current_x) { // East
        current.walls[2] = false;
        neighbour.walls[3] = false;
    } else if (neighbour_x < current_x) { // West
        current.walls[3] = false;
        neighbour.walls[2] = false;
    } else if (neighbour_y > current_y) { // South
        current.walls[1] = false;
        neighbour.walls[0] = false;
    } else if (neighbour_y < current_y) { // North
        current.walls[0] = false;
        neighbour.walls[1] = false;
    }
}

void Maze::generate() {
    std::stack<std::pair<int, int>> path;
    std::random_device rd;
    std::mt19937 g(rd());

    // Start at a fixed point, e.g., (0, 0)
    path.push({0, 0});
    grid[0][0].visited = true;

    while (!path.empty()) {
        std::pair<int, int> current_pos = path.top();
        int x = current_pos.first;
        int y = current_pos.second;

        std::vector<std::pair<int, int>> neighbours;

        // Check North
        if (y > 0 && !grid[y - 1][x].visited) neighbours.push_back({x, y - 1});
        // Check South
        if (y < height - 1 && !grid[y + 1][x].visited) neighbours.push_back({x, y + 1});
        // Check East
        if (x < width - 1 && !grid[y][x + 1].visited) neighbours.push_back({x + 1, y});
        // Check West
        if (x > 0 && !grid[y][x - 1].visited) neighbours.push_back({x - 1, y});

        if (!neighbours.empty()) {
            std::shuffle(neighbours.begin(), neighbours.end(), g);
            std::pair<int, int> next_pos = neighbours[0];
            int next_x = next_pos.first;
            int next_y = next_pos.second;

            removeWall(grid[y][x], grid[next_y][next_x], x, y, next_x, next_y);

            grid[next_y][next_x].visited = true;
            path.push(next_pos);
        } else {
            path.pop();
        }
    }

    // Create an exit at the starting cell
    grid[0][0].walls[0] = false; // Remove North wall of cell (0,0)
}

void Maze::draw(GLint model_uniform, GLint object_id_uniform, GLint render_as_black_uniform) const {
    glUniform1i(object_id_uniform, 50);

    float wall_height = 12.0f;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float center_x = (x - (width - 1) / 2.0f) * wall_size;
            float center_z = (y - (height - 1) / 2.0f) * wall_size;

            // North wall
            if (grid[y][x].walls[0]) {
                glm::mat4 wall_model = Matrix_Translate(center_x, wall_height/2.0f - 0.5f, center_z - wall_size / 2.0f)
                                     * Matrix_Scale(wall_size, wall_height, 0.1f);
                glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(wall_model));
                DrawCube(render_as_black_uniform, false);
            }
            // South wall
            if (grid[y][x].walls[1]) {
                glm::mat4 wall_model = Matrix_Translate(center_x, wall_height/2.0f - 0.5f, center_z + wall_size / 2.0f)
                                     * Matrix_Scale(wall_size, wall_height, 0.1f);
                glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(wall_model));
                DrawCube(render_as_black_uniform, false);
            }
            // East wall
            if (grid[y][x].walls[2]) {
                glm::mat4 wall_model = Matrix_Translate(center_x + wall_size / 2.0f, wall_height/2.0f - 0.5f, center_z)
                                     * Matrix_Rotate_Y(1.57079632679f)
                                     * Matrix_Scale(wall_size, wall_height, 0.1f);
                glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(wall_model));
                DrawCube(render_as_black_uniform, false);
            }
            // West wall
            if (grid[y][x].walls[3]) {
                glm::mat4 wall_model = Matrix_Translate(center_x - wall_size / 2.0f, wall_height/2.0f - 0.5f, center_z)
                                     * Matrix_Rotate_Y(1.57079632679f)
                                     * Matrix_Scale(wall_size, wall_height, 0.1f);
                glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(wall_model));
                DrawCube(render_as_black_uniform, false);
            }
        }
    }
}

std::vector<Plane> Maze::getCollisionPlanes() const {
    std::vector<Plane> planes;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float center_x = (x - (width - 1) / 2.0f) * wall_size;
            float center_z = (y - (height - 1) / 2.0f) * wall_size;

            // North wall (bloqueia movimento para cima/norte, que é -Z)
            if (grid[y][x].walls[0]) {
                float wall_z = center_z - wall_size / 2.0f;
                // Normal aponta PARA DENTRO da célula (+Z)
                // d = -dot(normal, point) = -(0*x + 0*y + 1*wall_z) = -wall_z
                planes.push_back({glm::vec3(0.0f, 0.0f, 1.0f), -wall_z});
            }
            
            // South wall (bloqueia movimento para baixo/sul, que é +Z)
            if (grid[y][x].walls[1]) {
                float wall_z = center_z + wall_size / 2.0f;
                // Normal aponta PARA DENTRO da célula (-Z)
                // d = -dot(normal, point) = -(0*x + 0*y + (-1)*wall_z) = wall_z
                planes.push_back({glm::vec3(0.0f, 0.0f, -1.0f), wall_z});
            }
            
            // East wall (bloqueia movimento para direita/leste, que é +X)
            if (grid[y][x].walls[2]) {
                float wall_x = center_x + wall_size / 2.0f;
                // Normal aponta PARA DENTRO da célula (-X)
                // d = -dot(normal, point) = -((-1)*wall_x + 0*y + 0*z) = wall_x
                planes.push_back({glm::vec3(-1.0f, 0.0f, 0.0f), wall_x});
            }
            
            // West wall (bloqueia movimento para esquerda/oeste, que é -X)
            if (grid[y][x].walls[3]) {
                float wall_x = center_x - wall_size / 2.0f;
                // Normal aponta PARA DENTRO da célula (+X)
                // d = -dot(normal, point) = -(1*wall_x + 0*y + 0*z) = -wall_x
                planes.push_back({glm::vec3(1.0f, 0.0f, 0.0f), -wall_x});
            }
        }
    }
    
    return planes;
}

glm::vec4 Maze::getStartPlayerPosition() const {
    float start_x = (0 - (width - 1) / 2.0f) * wall_size;
    float start_z = (0 - (height - 1) / 2.0f) * wall_size;
    return glm::vec4(start_x, 1.7f, start_z, 1.0f);
}

glm::vec3 Maze::getValidSpawnPosition() const {
    // Procura a primeira célula que tem pelo menos uma parede removida (indicando que é corredor)
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Verifica se pelo menos uma parede foi removida (célula visitada/corredor)
            if (grid[y][x].visited) {
                float world_x = (x - (width - 1) / 2.0f) * wall_size;
                float world_z = (y - (height - 1) / 2.0f) * wall_size;
                return glm::vec3(world_x, 1.7f, world_z);
            }
        }
    }
    // Fallback: centro do labirinto
    return glm::vec3(0.0f, 1.7f, 0.0f);
}
