#pragma once

#include <cstdint>

#define MGL_GLSL(version, source) "#version " #version "\n" #source "\n"

#define MGL_NO_COPY(Class)                  \
    Class::Class(const Class&)    = delete; \
    Class operator=(const Class&) = delete;

#define MGL_NO_MOVE(Class)             \
    Class::Class(Class&&)    = delete; \
    Class operator=(Class&&) = delete;

template <typename T>
struct View {
    constexpr View() : first(nullptr), len(0) {}

    template <size_t N>
    constexpr View(T(&ptr)[N])            : View(ptr, N) {}
    constexpr View(T* ptr, size_t length) : first(ptr), len(length) {}

    constexpr auto empty() const { return len == 0; }
    constexpr auto  size() const { return len; }
    constexpr auto  data() const { return first; }
    constexpr auto begin() const { return first; }
    constexpr auto   end() const { return first + len; }

    constexpr auto  operator[](size_t index) const { return first[index]; }
    constexpr auto& operator[](size_t index)       { return first[index]; }

    template <typename Object>
    static auto fromObject(const Object& object) -> View<const T> {
        const auto elementCount = sizeof(Object) / sizeof (float);
        return View<const T>{ (const float*) &object, elementCount };
    }

private:
    T*     first;
    size_t len;
};

struct StringView final : View<const char> {
    using View::View;

    constexpr StringView(const char* string) : View(string, stringLength(string)) {}
    constexpr operator bool() const { return ! empty(); }

    static constexpr auto stringLength(const char* string) -> size_t {
        size_t len = 0;

        while (string && *string++)
            len++;

        return len;
    }
};

namespace mgl {
    struct Program;
    struct VertexArray;
    struct Buffer;
    struct Texture;

    using handle_t = unsigned int;

    enum class BufferType {
        Array, Element, Uniform, Shader
    };

    enum class DataType {
        Float, Byte
    };

    enum class DrawMode {
        Triangles, Lines, Points
    };

    enum class TextureFormat {
        R8u,  RG8u,  RGB8u,  RGBA8u,
        R32f, RG32f, RGB32f, RGBA32f
    };

    template <typename T>
    auto Attribute(int index, int size, size_t stride, size_t offset) -> void;

    template <typename T>
    auto Attribute(int index, size_t stride, size_t offset) -> void;

    template <typename T>
    auto Uniform(Program& p, int index, const T& value) -> void;

    template <typename T>
    auto set_uniform(Program&,      int index, const T& value) -> void;

    auto set_uniform_f1(Program&,   int index, View<const float> data)  -> void;
    auto set_uniform_f2(Program&,   int index, View<const float> data)  -> void;
    auto set_uniform_f3(Program&,   int index, View<const float> data)  -> void;
    auto set_uniform_f4(Program&,   int index, View<const float> data)  -> void;
    auto set_uniform_i1(Program&,   int index, View<const int>   data)  -> void;
    auto set_uniform_i2(Program&,   int index, View<const int>   data)  -> void;
    auto set_uniform_i3(Program&,   int index, View<const int>   data)  -> void;
    auto set_uniform_i4(Program&,   int index, View<const int>   data)  -> void;
    auto set_uniform_m3x2(Program&, int index, View<const float> data)  -> void;
    auto set_uniform_m3x3(Program&, int index, View<const float> data)  -> void;
    auto set_uniform_m4x2(Program&, int index, View<const float> data)  -> void;
    auto set_uniform_m4x3(Program&, int index, View<const float> data)  -> void;
    auto set_uniform_m4x4(Program&, int index, View<const float> data)  -> void;

    struct AllocatorFuncs {
        void*(*allocate)(void* user, size_t size) = nullptr;
        void (*free)(void* user, void* ptr)       = nullptr;
        void* user = nullptr;
    };

    extern AllocatorFuncs defaultAllocator;

    auto init(AllocatorFuncs* allocator = &defaultAllocator) -> void;
    auto viewport(float x, float y, float w, float h)        -> void;
    auto clear(float r, float g, float b, bool clearColour = true, bool clearDepth = true) -> void;

    struct Sampler final {
        MGL_NO_COPY(Sampler);
        MGL_NO_MOVE(Sampler);

        Sampler(int slotIndex);

        auto setTexture(const Texture*) -> void;
        auto activate()               -> void;

        int            index;
        const Texture* texture;
    };

    struct UniformSetter final {
        UniformSetter(Program& p, int uniformIndex) : program(p), index(uniformIndex) {}

        template <typename T>
        auto operator=(const T& value) {
            Uniform<T>(program, index, value);
        }

        Program& program;
        int      index;
    };

    struct Texture final {
        MGL_NO_COPY(Texture);
        MGL_NO_MOVE(Texture);

        ~Texture();

        auto write(int x, int y, int w, int h, DataType sourceDataType, void const* data) -> void;

        static auto make(int w, int h, DataType sourceDataType, TextureFormat format, const void* data) -> Texture*;

        handle_t      handle;
        TextureFormat format;
    };

    struct Program final {
        MGL_NO_COPY(Program);
        MGL_NO_MOVE(Program);

        ~Program();

        auto use() const -> void;

        auto uniform(StringView name)    -> UniformSetter;
        auto operator[](StringView name) -> UniformSetter;

        static auto make(StringView vertexShaderSource,
                         StringView fragShaderSource,
                         View<char>& error) -> Program*;

        handle_t handle;
    };

    struct Buffer final {
        MGL_NO_COPY(Buffer);
        MGL_NO_MOVE(Buffer);

        ~Buffer();

        auto bind() -> void;
        auto write(const void* data, size_t len, size_t offset) -> void;

        template <typename ArrayType>
        auto write(View<const ArrayType> array, size_t offset) -> void {
            write(array.data(), array.size() * sizeof (ArrayType), offset);
        }

        static auto make(BufferType type, size_t size, const void* data = nullptr, bool dynamic = true) -> Buffer*;

        handle_t   handle;
        size_t     size;
        BufferType type;
    };

    struct VertexArray final {
        MGL_NO_COPY(VertexArray);
        MGL_NO_MOVE(VertexArray);

        using ConfigureCallback = void(*) (mgl::handle_t, View<Buffer*>);

        ~VertexArray();
        auto getBuffers() const -> View<Buffer*>;
        auto bind()       const -> void;

        auto draw(DrawMode mode, int offset, int count)    const -> void;
        static auto make(View<Buffer*>, ConfigureCallback)       -> VertexArray*;

        handle_t handle;
        Buffer** attachedBuffers;
        size_t   attachedBufferCount;
    };
}
