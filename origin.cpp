#include "Origin.h"

// ============================================================================
// SECCIÓN 1: VÉRTICES DE LOS EJES XYZ
// ============================================================================

// Vértices estáticos para los 3 ejes cartesianos usando GL_LINES:
// X (Rojo: 1,0,0), Y (Verde: 0,1,0), Z (Azul: 0,0,1)
static const LineVertex kDefaultOrigin[] = {
    // Eje X (Rojo) - Del origen al punto (1,0,0)
    { { 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
    { { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },

    // Eje Y (Verde) - Del origen al punto (0,1,0)
    { { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
    { { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },

    // Eje Z (Azul) - Del origen al punto (0,0,1)
    { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
    { { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
};

// ============================================================================
// SECCIÓN 2: CICLO DE VIDA (Constructor / Destructor)
// ============================================================================

Origin::Origin() noexcept {
    // Inicializa la geometría enviando los 6 vértices por defecto (3 líneas)
    init(kDefaultOrigin, sizeof(kDefaultOrigin) / sizeof(kDefaultOrigin[0]));
}

Origin::~Origin() {
    destroy(); // Libera buffers al destruir la instancia
}

// ============================================================================
// SECCIÓN 3: CONFIGURACIÓN DE BUFFERS Y RENDERIZADO
// ============================================================================

void Origin::init(const LineVertex* data, std::size_t count) {
    destroy();
    if (!data || count == 0) return;

    m_count = count;

    // Generar e inicializar el VAO y VBO
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(LineVertex), data, GL_STATIC_DRAW);

    // Atributo 0: Posición de la línea (vec3: x, y, z)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex), (void*)offsetof(LineVertex, pos));
    glEnableVertexAttribArray(0);

    // Atributo 1: Color de la línea (vec4: r, g, b, a)
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(LineVertex), (void*)offsetof(LineVertex, color));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Origin::draw() const {
    if (m_vao == 0 || m_count == 0) return;

    glBindVertexArray(m_vao);
    // Dibuja los segmentos de línea en pares de vértices
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_count));
    glBindVertexArray(0);
}

void Origin::destroy() noexcept {
    // Liberación de recursos en GPU
    if (m_vbo) { glDeleteBuffers(1, &m_vbo); m_vbo = 0; }
    if (m_vao) { glDeleteVertexArrays(1, &m_vao); m_vao = 0; }
    m_count = 0;
}