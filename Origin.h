#pragma once

#include <cstddef>
#include <glad/glad.h>

// ============================================================================
// ESTRUCTURA DE VÉRTICE PARA LÍNEAS
// ============================================================================

struct LineVertex {
    GLfloat pos[3];   // Posición XYZ
    GLfloat color[4]; // Color RGBA
};

// ============================================================================
// CLASE ORIGIN (Dibuja los ejes de coordenadas XYZ)
// ============================================================================

class Origin {
public:
    Origin() noexcept;
    ~Origin();

    // Deshabilitar copia (evita duplicar punteros a buffers de OpenGL en memoria GPU)
    Origin(const Origin&) = delete;
    Origin& operator=(const Origin&) = delete;

    // Inicialización y Renderizado
    void init(const LineVertex* data, std::size_t count);
    void draw() const;

    std::size_t vertexCount() const noexcept { return m_count; }

private:
    void destroy() noexcept;

    GLuint m_vao = 0;      // Vertex Array Object
    GLuint m_vbo = 0;      // Vertex Buffer Object
    std::size_t m_count = 0; // Cantidad de vértices
};