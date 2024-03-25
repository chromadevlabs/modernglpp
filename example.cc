
#include <GLFW/glfw3.h>
#include <cstdio>

#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "modernglpp.h"

using namespace mgl;

struct Vertex {
    glm::vec2 position;
};

namespace mgl {
    template <>
    auto Attribute<glm::vec2>(int index, size_t stride, size_t offset) -> void {
        Attribute<float>(index, 2, stride, offset);
    }

    template <>
    auto Attribute<glm::vec3>(int index, size_t stride, size_t offset) -> void {
        Attribute<float>(index, 3, stride, offset);
    }

    template <>
    auto Attribute<glm::vec4>(int index, size_t stride, size_t offset) -> void {
        Attribute<float>(index, 4, stride, offset);
    }

    template <>
    auto Uniform<glm::vec2>(Program& p, int index, const glm::vec2& matrix) -> void {
        set_uniform_f2(p, index, View<const float>::fromObject (matrix));
    }

    template <>
    auto Uniform<glm::vec3>(Program& p, int index, const glm::vec3& matrix) -> void {
        set_uniform_f3(p, index, View<const float>::fromObject (matrix));
    }

    template <>
    auto Uniform<glm::vec4>(Program& p, int index, const glm::vec4& matrix) -> void {
        set_uniform_f4(p, index, View<const float>::fromObject (matrix));
    }

    template <>
    auto Uniform<glm::mat3>(Program& p, int index, const glm::mat3& matrix) -> void {
        set_uniform_m3x3(p, index, View<const float>::fromObject (matrix));
    }

    template <>
    auto Uniform<glm::mat4>(Program& p, int index, const glm::mat4& matrix) -> void {
        set_uniform_m4x4(p, index, View<const float>::fromObject (matrix));
    }
}

static mgl::Buffer*      vbo       = nullptr;
static mgl::VertexArray* vao       = nullptr;
static mgl::Program*     program   = nullptr;
static mgl::Texture*     texture   = nullptr;
static mgl::Sampler      sampler1 = { 0 };

static auto init() -> void {
    mgl::init();

    const Vertex v[3] = {
        { { -1, -1 } },
        { {  1, -1 } },
        { {  0,  1 } }
    };

    vbo = Buffer::make(BufferType::Array, 4096);
    vbo->write(View<const Vertex>{ v }, 0);

    Buffer* buffers[] = { vbo };
    vao = VertexArray::make(View<Buffer*>{ buffers }, [] (mgl::handle_t, auto buffers) {
        {   // position
            buffers[0]->bind();
            Attribute<glm::vec2>(0, sizeof (Vertex), offsetof(Vertex, position));
        }
    });

    const int32_t pixel = 0xFFFF00FF;
    const mgl::TextureSourceData texData {
        TextureFormat::RGB,
        DataType::Byte,
        &pixel
    };

    texture = mgl::Texture::make (1, 1, TextureFormat::RGB32f, &texData);
    
    TextureOptions options{};
    options.filter.min = TextureFilterMode::Nearest;
    options.filter.mag = TextureFilterMode::Nearest;
    options.wrap.s     = TextureWrapMode::ClampToEdge;
    options.wrap.r     = TextureWrapMode::ClampToEdge;
    options.wrap.t     = TextureWrapMode::ClampToEdge;
    
    texture->setOptions(options);
    sampler1.setTexture(texture);

    char error[1024];
    View<char> errorString{error};
    program = Program::make(
        MGL_GLSL(410,
            layout(location = 0) in vec2 vertexPosition;

            uniform mat4 matrix;

            void main() {
                gl_Position = matrix * vec4(vertexPosition, 0, 1);
            }
        ),
        MGL_GLSL(410,

            uniform sampler2D sampler1;
            out vec4 fragColour;

            void main() {
                fragColour = vec4(texture(sampler1, vec2(0, 0)).rgb, 1);
            }
        ), errorString
    );

    if (! program) {
        printf("Failed to compile shader: %s\n", errorString.data());
        exit(-1);
    }
}

static auto render(int framebufferWidth, int frameBufferHeight) -> void {
    mgl::viewport(0, 0, framebufferWidth, frameBufferHeight);
    mgl::clear(0.1, 0.1, 0.1);

    vao->bind();
    sampler1.bind();
    program->use();

    program->uniform("sampler1") = sampler1;
    program->uniform("matrix") = glm::identity<glm::mat4>();

    vao->draw(DrawMode::Triangles, 0, 3);
}

auto main(int, const char**) -> int {
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    if (auto* window = glfwCreateWindow(1280, 720, "moderngl", nullptr, nullptr)) {
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);

        ::init();

        while (! glfwWindowShouldClose(window)) {
            if (glfwGetKey(window, GLFW_KEY_ESCAPE)) {
                break;
            }

            int w, h;
            glfwGetFramebufferSize(window, &w, &h);
            render(w, h);

            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        delete vbo;
        delete vao;
        delete program;

        glfwDestroyWindow(window);
    }

    glfwTerminate();
    return 0;
}
