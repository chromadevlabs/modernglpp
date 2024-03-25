
#include <glad/glad.h>
#include <new>

#include "modernglpp.h"

#if defined(_WIN32)
    #define debug_break() __debugbreak()
#elif defined(__APPLE__)
    #define debug_break() __builtin_debugtrap()
#endif

#define MGL_ASSERT(expr)    if (! (expr))                       { debug_break(); }
#define MGL_OPENGL_CHECK()  if (auto* err = glGetErrorString()) { debug_break(); }

static mgl::AllocatorFuncs* allocator = nullptr;

template <typename T, typename... Args>
static auto newObject(Args&&... args) -> T* {
    auto* ptr = allocator->allocate(allocator->user, sizeof (T));
    return new (ptr) T{args...};
}

template <typename T>
static auto deleteObject(T* ptr) -> void {
    if (ptr) {
        ptr->~T();
        allocator->free(allocator->user, ptr);
    }
}

static auto glGetErrorString() -> const char* {
    switch (glGetError()) {
        case GL_INVALID_ENUM:      return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE:     return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
        case GL_OUT_OF_MEMORY:     return "GL_OUT_OF_MEMORY";
    }

    return nullptr;
}

namespace mgl {
    static constexpr auto enum_cast(DataType type) -> GLenum {
        switch (type) {
            case DataType::Float: return GL_FLOAT;
            case DataType::Byte:  return GL_UNSIGNED_BYTE;
        }

        return GL_INVALID_ENUM;
    }

    static constexpr auto enum_cast(BufferType type) -> GLenum {
        switch (type) {
            case BufferType::Array:   return GL_ARRAY_BUFFER;
            case BufferType::Element: return GL_ELEMENT_ARRAY_BUFFER;
            case BufferType::Uniform: return GL_UNIFORM_BUFFER;
            case BufferType::Shader:  return GL_SHADER_STORAGE_BUFFER;
        }

        return GL_INVALID_ENUM;
    }

    static constexpr auto enum_cast(DrawMode type) -> GLenum {
        switch (type) {
            case DrawMode::Triangles: return GL_TRIANGLES;
            case DrawMode::Lines:     return GL_LINES;
            case DrawMode::Points:    return GL_POINTS;
        }

        return GL_INVALID_ENUM;
    }

    static constexpr auto enum_cast(TextureFormat format) -> GLenum {
        switch (format) {
            case TextureFormat::RED:     return GL_RED;
            case TextureFormat::RG:      return GL_RG;
            case TextureFormat::RGB:     return GL_RGB;
            case TextureFormat::RGBA:    return GL_RGBA;
            case TextureFormat::BGR:     return GL_BGR;
            case TextureFormat::BGRA:    return GL_BGRA;

            case TextureFormat::R8u:     return GL_R8;
            case TextureFormat::RG8u:    return GL_RG8;
            case TextureFormat::RGB8u:   return GL_RGB8;
            case TextureFormat::RGBA8u:  return GL_RGBA8;
            case TextureFormat::R32f:    return GL_R32F;
            case TextureFormat::RG32f:   return GL_RG32F;
            case TextureFormat::RGB32f:  return GL_RGB32F;
            case TextureFormat::RGBA32f: return GL_RGBA32F;
        }

        return GL_INVALID_ENUM;
    }

    static constexpr auto enum_cast(TextureFilterMode mode) -> GLenum {
        switch (mode) {
        case TextureFilterMode::Linear:  return GL_LINEAR;
        case TextureFilterMode::Nearest: return GL_NEAREST;
        }

        return GL_INVALID_ENUM;
    }

    static constexpr auto enum_cast(TextureWrapMode mode) -> GLenum {
        switch (mode) {
        case TextureWrapMode::ClampToEdge:        return GL_CLAMP_TO_EDGE;
        case TextureWrapMode::ClampToBorder:      return GL_CLAMP_TO_BORDER;
        case TextureWrapMode::MirroredRepeat:     return GL_MIRRORED_REPEAT;
        case TextureWrapMode::Repeat:             return GL_REPEAT;
        case TextureWrapMode::MirrorClampToEdge:  return GL_MIRROR_CLAMP_TO_EDGE;
        }

        return GL_INVALID_ENUM;
    }

    static constexpr auto sizedToBase(TextureFormat format) -> TextureFormat {
        switch (format) {
            case TextureFormat::R8u:
            case TextureFormat::R32f:    return TextureFormat::RED;

            case TextureFormat::RG8u:
            case TextureFormat::RG32f:   return TextureFormat::RG;
            
            case TextureFormat::RGB8u:   
            case TextureFormat::RGB32f:  return TextureFormat::RGB;
            
            case TextureFormat::RGBA8u:
            case TextureFormat::RGBA32f: return TextureFormat::RGBA;
        }

        return format;
    }

    auto set_uniform_f1(Program&, int index, View<const float> data) -> void {
        MGL_ASSERT(data.size() == 1);
        glUniform1fv(index, 1, data.data());
    }

    auto set_uniform_f2(Program&, int index, View<const float> data) -> void {
        MGL_ASSERT(data.size() == 2);
        glUniform2fv(index, 1, data.data());
    }

    auto set_uniform_f3(Program&, int index, View<const float> data) -> void {
        MGL_ASSERT(data.size() == 3);
        glUniform3fv(index, 1, data.data());
    }

    auto set_uniform_f4(Program&,   int index, View<const float> data) -> void {
        MGL_ASSERT(data.size() == 4);
        glUniform4fv(index, 1, data.data());
    }

    auto set_uniform_i1(Program&,   int index, View<const int> data) -> void {
        MGL_ASSERT(data.size() == 1);
        glUniform1iv(index, 1, data.data());
    }

    auto set_uniform_i2(Program&,   int index, View<const int> data) -> void {
        MGL_ASSERT(data.size() == 2);
        glUniform2iv(index, 1, data.data());
    }

    auto set_uniform_i3(Program&, int index, View<const int> data) -> void {
        MGL_ASSERT(data.size() == 3);
        glUniform3iv(index, 1, data.data());
    }

    auto set_uniform_i4(Program&, int index, View<const int> data) -> void {
        MGL_ASSERT(data.size() == 4);
        glUniform4iv(index, 1, data.data());
    }

    auto set_uniform_m3x2(Program&, int index, View<const float> data) -> void {
        MGL_ASSERT(data.size() == 3 * 2);
        glUniformMatrix3x2fv(index, 1, GL_FALSE, data.data());
    }

    auto set_uniform_m3x3(Program&, int index, View<const float> data) -> void {
        MGL_ASSERT(data.size() == 3 * 3);
        glUniformMatrix3fv(index, 1, GL_FALSE, data.data());
    }

    auto set_uniform_m4x2(Program&, int index, View<const float> data) -> void {
        MGL_ASSERT(data.size() == 4 * 2);
        glUniformMatrix4x2fv(index, 1, GL_FALSE, data.data());
    }

    auto set_uniform_m4x3(Program&, int index, View<const float> data) -> void {
        MGL_ASSERT(data.size() == 4 * 3);
        glUniformMatrix4x3fv(index, 1, GL_FALSE, data.data());
    }

    auto set_uniform_m4x4(Program&, int index, View<const float> data) -> void {
        MGL_ASSERT(data.size() == 4 * 4);
        glUniformMatrix4fv(index, 1, GL_FALSE, data.data());
    }

    template <>
    auto set_uniform<Sampler> (Program& p, int index, const Sampler& value) -> void {
        set_uniform_i1(p, index, View<const int>{ &value.index, (size_t) 1 });
    }

    template <>
    auto Uniform<float>(Program& p, int index, const float& value) -> void {
        set_uniform_f1(p, index, View<const float>{ &value, (size_t) 1 });
    }

    template <>
    auto Uniform<int>(Program& p, int index, const int& value) -> void {
        set_uniform_i1(p, index, View<const int>{ &value, (size_t) 1 });
    }

    template <>
    auto Uniform<Sampler>(Program& p, int index, const Sampler& value) -> void {
        set_uniform (p, index, value);
    }

    #define ATTRIBUTE_IMPL_I(Type, Enum) template <>                                  \
    auto Attribute<Type>(int index, int size, size_t stride, size_t offset) -> void { \
        glEnableVertexAttribArray(index);                                             \
        glVertexAttribIPointer(index, size, Enum, stride, (const void*) offset); }

    template <>
    auto Attribute<float>(int index, int size, size_t stride, size_t offset) -> void {
        glEnableVertexAttribArray(index);
        glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, stride, (const void*) offset);
    }

    ATTRIBUTE_IMPL_I(uint8_t,  GL_UNSIGNED_BYTE);
    ATTRIBUTE_IMPL_I(uint16_t, GL_UNSIGNED_SHORT);
    ATTRIBUTE_IMPL_I(uint32_t, GL_UNSIGNED_INT);
    ATTRIBUTE_IMPL_I(int8_t,   GL_BYTE);
    ATTRIBUTE_IMPL_I(int16_t,  GL_SHORT);
    ATTRIBUTE_IMPL_I(int32_t,  GL_INT);

    #undef ATTRIBUTE_IMPL_I

    AllocatorFuncs defaultAllocator {
        [] (void*, size_t len) -> void* {
            return new char[len];
        },
        [] (void*, void* ptr) -> void {
            delete[] (char*) ptr;
        }
    };

    auto init(AllocatorFuncs* allocator) -> void {
        ::allocator = allocator;
        gladLoadGL();
    }

    auto viewport(float x, float y, float w, float h) -> void {
        glViewport(x, y, w, h);
    }

    auto clear(float r, float g, float b, bool clearColour, bool clearDepth) -> void {
        glClearColor(r, g, b, 1);
        glClear(clearColour ? GL_COLOR_BUFFER_BIT : 0 |
                clearDepth  ? GL_DEPTH_BUFFER_BIT : 0);
    }

    Sampler::Sampler(int slotIndex) : index(slotIndex) {
    }

    auto Sampler::setTexture(const Texture* texture) -> void {
        this->texture = texture;
    }

    auto Sampler::bind() -> void {
        glActiveTexture(GL_TEXTURE0 + index);
        glBindTexture(GL_TEXTURE_2D, texture ? texture->handle : 0);
        MGL_OPENGL_CHECK();
    }

    auto Program::uniform(StringView name) -> UniformSetter {
        return UniformSetter{*this, glGetUniformLocation(handle, name.data())};
    }

    auto Program::operator[](StringView name) -> UniformSetter {
        return UniformSetter{*this, glGetUniformLocation(handle, name.data())};
    }

    auto Program::make(StringView vertexShaderSource, StringView fragShaderSource, View<char>& result) -> Program* {
        auto compile = [](StringView source, GLenum shaderType, View<char>& result) {
            const char* sources[] = { source.data() };
            const auto  length    = (GLint) source.size();

            GLint successFlag = GL_FALSE;
            auto s = glCreateShader(shaderType);
            glShaderSource(s, 1, sources, &length);
            glCompileShader(s);
            glGetShaderiv(s, GL_COMPILE_STATUS, &successFlag);
            MGL_OPENGL_CHECK();

            if (successFlag == GL_FALSE) {
                if (! result.empty()) {
                    GLsizei errLen = 0;
                    glGetShaderInfoLog(s, result.size() - 1, &errLen, result.data());
                    result = View<char> { result.data(), (size_t) errLen };
                }

                glDeleteShader(s);
                s = 0;
            }

            MGL_OPENGL_CHECK();
            return s;
        };

        handle_t vs, fs;
        GLint successFlag = GL_FALSE;

        if (! (vs = compile(vertexShaderSource, GL_VERTEX_SHADER,   result)) ||
            ! (fs = compile(fragShaderSource,   GL_FRAGMENT_SHADER, result))) {
            return nullptr;
        }

        auto p = glCreateProgram();

        glAttachShader(p, vs);
        glAttachShader(p, fs);
        glDeleteShader(vs);
        glDeleteShader(fs);
        glLinkProgram(p);
        glGetProgramiv(p, GL_LINK_STATUS, &successFlag);
        MGL_OPENGL_CHECK();

        if (successFlag == GL_FALSE) {
            if (! result.empty()) {
                GLsizei errLen = 0;
                glGetProgramInfoLog(p, result.size() - 1, &errLen, result.data());
                result = View<char> { result.data(), (size_t) errLen };
            }

            glDeleteProgram(p);
            return nullptr;
        }

        MGL_OPENGL_CHECK();

        return newObject<Program>(p);
    }

    Program::~Program() {
        glDeleteProgram(handle);
    }

    auto Program::use() const -> void {
        glUseProgram(handle);
    };

    auto Buffer::make(BufferType type, size_t size, const void* data, bool dynamic) -> Buffer* {
        handle_t handle;

        glGenBuffers(1, &handle);
        glBindBuffer(enum_cast(type), handle);
        glBufferData(enum_cast(type), size, data, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
        MGL_OPENGL_CHECK();

        return newObject<Buffer>(handle, size, type);
    }

    Buffer::~Buffer() {
        glDeleteBuffers(1, &handle);
    }

    auto Buffer::bind() -> void {
        glBindBuffer(enum_cast(type), handle);
        MGL_OPENGL_CHECK();
    }

    auto Buffer::write(const void* data, size_t len, size_t offset) -> void {
        glBindBuffer(enum_cast(type), handle);
        glBufferSubData(enum_cast(type), offset, len, data);
        MGL_OPENGL_CHECK();
    }

    auto VertexArray::make(View<Buffer*> buffers, ConfigureCallback callback) -> VertexArray* {
        handle_t handle;

        glGenVertexArrays(1, &handle);
        glBindVertexArray(handle);
        MGL_OPENGL_CHECK();

        callback(handle, buffers);
        MGL_OPENGL_CHECK();

        auto* vao = newObject<VertexArray>(
            handle, 
            (Buffer**) allocator->allocate(allocator->user, buffers.size() * sizeof (Buffer*)),
            buffers.size()
        );

        for (int i = 0; i < buffers.size(); i++)
            vao->attachedBuffers[i] = buffers[i];

        return vao;
    }

    VertexArray::~VertexArray() {
        glDeleteVertexArrays(1, &handle);

        for (auto* buffer : getBuffers())
            buffer->~Buffer();

        allocator->free(allocator->user, attachedBuffers);
    }

    auto VertexArray::getBuffers() const -> View<Buffer*> {
        return { attachedBuffers, attachedBufferCount };
    }

    auto VertexArray::bind() const -> void {
        glBindVertexArray(handle);
        MGL_OPENGL_CHECK();
    }

    auto VertexArray::draw(DrawMode mode, int offset, int count) const -> void {
        switch (mode) {
            case DrawMode::Triangles: glDrawArrays(GL_TRIANGLES, offset, count); break;
            case DrawMode::Lines:     glDrawArrays(GL_LINES,     offset, count); break;
            case DrawMode::Points:    glDrawArrays(GL_POINTS,    offset, count); break;
        }

        MGL_OPENGL_CHECK();
    }

    auto Texture::write(int x, int y, int w, int h, DataType sourceDataType, void const* data) -> void {
        glBindTexture(GL_TEXTURE_2D, handle);
        glTexSubImage2D(GL_TEXTURE_2D,
                        0,
                        x, y, w, h,
                        enum_cast(format),
                        enum_cast(sourceDataType),
                        data);
        MGL_OPENGL_CHECK();
    }

    auto Texture::setOptions(TextureOptions options) -> void {
        glBindTexture(GL_TEXTURE_2D, handle);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, enum_cast(options.filter.min));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, enum_cast(options.filter.mag));

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, enum_cast(options.wrap.s));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, enum_cast(options.wrap.t));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, enum_cast(options.wrap.r));

        MGL_OPENGL_CHECK();
    }

    auto Texture::make(int w, int h, TextureFormat deviceFormat, const TextureSourceData* desc) -> Texture* {
        handle_t handle;

        glGenTextures(1, &handle);
        glBindTexture(GL_TEXTURE_2D, handle);

        if (desc) {
            glTexImage2D(GL_TEXTURE_2D, 0,
                         enum_cast(deviceFormat), w, h, 0,
                         enum_cast(sizedToBase(desc->format)), 
                         enum_cast(desc->type), desc->data);
        }
        else {
            glTexImage2D(GL_TEXTURE_2D, 0,
                         enum_cast(deviceFormat), w, h, 0,
                         enum_cast(sizedToBase(deviceFormat)), 
                         GL_UNSIGNED_BYTE, nullptr);
        }

        MGL_OPENGL_CHECK();
        return newObject<Texture>(handle, deviceFormat);
    }

    Texture::~Texture() {
        glDeleteTextures(1, &handle);
        MGL_OPENGL_CHECK();
    }
}
