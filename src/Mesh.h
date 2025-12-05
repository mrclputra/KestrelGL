#pragma once

// placeholder, ASSIMP logic goes here
// or maybe put it in a factory?

class Mesh {
public:
    Mesh();
    virtual ~Mesh();

    virtual void render();

    // will be implemented later

private:
    // prevent copying
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
};