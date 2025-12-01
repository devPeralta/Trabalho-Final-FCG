#ifndef MAZE_H
#define MAZE_H

#include <vector>
#include <stack>
#include <random>
#include <glad/glad.h>
#include <glm/mat4x4.hpp>
#include "collisions.h"

// Forward declaration of DrawCube from main.cpp
void DrawCube(GLint render_as_black_uniform, bool draw_axes_and_edges);

struct Cell {
    bool visited = false;
    // walls[0] = North, walls[1] = South, walls[2] = East, walls[3] = West
    bool walls[4] = {true, true, true, true};
};

class Maze {
public:
    Maze(int width, int height, float wall_size);
    void generate();
    void draw(GLint model_uniform, GLint object_id_uniform, GLint render_as_black_uniform) const;
    std::vector<Plane> getCollisionPlanes() const;
    glm::vec4 getStartPlayerPosition() const;
    glm::vec3 getValidSpawnPosition() const;

private:
    int width;
    int height;
    float wall_size;
    std::vector<std::vector<Cell>> grid;

    void removeWall(Cell& current, Cell& neighbour, int current_x, int current_y, int neighbour_x, int neighbour_y);
};

#endif // MAZE_H
