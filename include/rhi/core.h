#pragma once

#include <array>
#include <atomic>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <type_traits>
#include <utility>

#define NVRHI_ENUM_CLASS_FLAG_OPERATORS(T) \
    inline T operator | (T a, T b) { return T(uint32_t(a) | uint32_t(b)); } \
    inline T operator & (T a, T b) { return T(uint32_t(a) & uint32_t(b)); } /* NOLINT(bugprone-macro-parentheses) */ \
    inline T operator ~ (T a) { return T(~uint32_t(a)); } /* NOLINT(bugprone-macro-parentheses) */ \
    inline bool operator !(T a) { return uint32_t(a) == 0; } \
    inline bool operator ==(T a, uint32_t b) { return uint32_t(a) == b; } \
    inline bool operator !=(T a, uint32_t b) { return uint32_t(a) != b; }

#if defined(NVRHI_SHARED_LIBRARY_BUILD)
#   if defined(_MSC_VER)
#       define NVRHI_API __declspec(dllexport)
#   elif defined(__GNUC__)
#       define NVRHI_API __attribute__((visibility("default")))
#   else
#       define NVRHI_API
#       pragma warning "Unknown dynamic link import/export semantics."
#   endif
#elif defined(NVRHI_SHARED_LIBRARY_INCLUDE)
#   if defined(_MSC_VER)
#       define NVRHI_API __declspec(dllimport)
#   else
#       define NVRHI_API
#   endif
#else
#   define NVRHI_API
#endif

namespace helicon
{
    // Version of the public API provided by NVRHI.
    // Increment this when any changes to the API are made.
    static constexpr uint32_t c_HeaderVersion = 23;

    // Verifies that the version of the implementation matches the version of the header.
    // Returns true if they match. Use this when initializing apps using NVRHI as a shared library.
    NVRHI_API bool verifyHeaderVersion(uint32_t version = c_HeaderVersion);

    static constexpr uint32_t c_MaxRenderTargets = 8;
    static constexpr uint32_t c_MaxViewports = 16;
    static constexpr uint32_t c_MaxVertexAttributes = 16;
    static constexpr uint32_t c_MaxBindingLayouts = 8;
    static constexpr uint32_t c_MaxBindlessRegisterSpaces = 16;
    static constexpr uint32_t c_MaxVolatileConstantBuffersPerLayout = 6;
    static constexpr uint32_t c_MaxVolatileConstantBuffers = 32;
    static constexpr uint32_t c_MaxPushConstantSize = 128; // D3D12: root signature is 256 bytes max., Vulkan: 128 bytes of push constants guaranteed
    static constexpr uint32_t c_ConstantBufferOffsetSizeAlignment = 256; // Partially bound constant buffers must have offsets aligned to this and sizes multiple of this

    //////////////////////////////////////////////////////////////////////////
    // Basic Types
    //////////////////////////////////////////////////////////////////////////

    struct Color
    {
        float r, g, b, a;

        Color() : r(0.f), g(0.f), b(0.f), a(0.f) { }
        Color(float c) : r(c), g(c), b(c), a(c) { }
        Color(float _r, float _g, float _b, float _a) : r(_r), g(_g), b(_b), a(_a) { }

        bool operator ==(const Color& _b) const { return r == _b.r && g == _b.g && b == _b.b && a == _b.a; }
        bool operator !=(const Color& _b) const { return !(*this == _b); }
    };

    struct Viewport
    {
        float minX, maxX;
        float minY, maxY;
        float minZ, maxZ;

        Viewport() : minX(0.f), maxX(0.f), minY(0.f), maxY(0.f), minZ(0.f), maxZ(1.f) { }

        Viewport(float width, float height) : minX(0.f), maxX(width), minY(0.f), maxY(height), minZ(0.f), maxZ(1.f) { }

        Viewport(float _minX, float _maxX, float _minY, float _maxY, float _minZ, float _maxZ)
            : minX(_minX), maxX(_maxX), minY(_minY), maxY(_maxY), minZ(_minZ), maxZ(_maxZ)
        { }

        bool operator ==(const Viewport& b) const
        {
            return minX == b.minX
                && minY == b.minY
                && minZ == b.minZ
                && maxX == b.maxX
                && maxY == b.maxY
                && maxZ == b.maxZ;
        }
        bool operator !=(const Viewport& b) const { return !(*this == b); }

        [[nodiscard]] float width() const { return maxX - minX; }
        [[nodiscard]] float height() const { return maxY - minY; }
    };

    struct Rect
    {
        int minX, maxX;
        int minY, maxY;

        Rect() : minX(0), maxX(0), minY(0), maxY(0) { }
        Rect(int width, int height) : minX(0), maxX(width), minY(0), maxY(height) { }
        Rect(int _minX, int _maxX, int _minY, int _maxY) : minX(_minX), maxX(_maxX), minY(_minY), maxY(_maxY) { }
        explicit Rect(const Viewport& viewport)
            : minX(int(floorf(viewport.minX)))
            , maxX(int(ceilf(viewport.maxX)))
            , minY(int(floorf(viewport.minY)))
            , maxY(int(ceilf(viewport.maxY)))
        {
        }

        bool operator ==(const Rect& b) const {
            return minX == b.minX && minY == b.minY && maxX == b.maxX && maxY == b.maxY;
        }
        bool operator !=(const Rect& b) const { return !(*this == b); }

        [[nodiscard]] int width() const { return maxX - minX; }
        [[nodiscard]] int height() const { return maxY - minY; }
    };

    enum class GraphicsAPI : uint8_t
    {
        D3D11,
        D3D12,
        VULKAN
    };


// a static vector is a vector with a capacity defined at compile-time
template <typename T, uint32_t _max_elements>
struct static_vector : private std::array<T, _max_elements>
{
    typedef std::array<T, _max_elements> base;
    enum { max_elements = _max_elements };

    using typename base::value_type;
    using typename base::size_type;
    using typename base::difference_type;
    using typename base::reference;
    using typename base::const_reference;
    using typename base::pointer;
    using typename base::const_pointer;
    using typename base::iterator;
    using typename base::const_iterator;
    // xxxnsubtil: reverse iterators not implemented

    static_vector()
        : base()
        , current_size(0)
    { }

    static_vector(size_t size)
        : base()
        , current_size(size)
    {
        assert(size <= max_elements);
    }

    static_vector(std::initializer_list<T> il)
        : current_size(0)
    {
        for(auto i : il)
            push_back(i);
    }

    using base::at;

    reference operator[] (size_type pos)
    {
        assert(pos < current_size);
        return base::operator[](pos);
    }

    const_reference operator[] (size_type pos) const
    {
        assert(pos < current_size);
        return base::operator[](pos);
    }

    using base::front;

    reference back() noexcept                   { auto tmp =  end(); --tmp; return *tmp; }
    const_reference back() const noexcept       { auto tmp = cend(); --tmp; return *tmp; }

    using base::data;
    using base::begin;
    using base::cbegin;

    iterator end() noexcept                     { return iterator(begin()) + current_size; }
    const_iterator end() const noexcept         { return cend(); }
    const_iterator cend() const noexcept        { return const_iterator(cbegin()) + current_size; }

    bool empty() const noexcept                 { return current_size == 0; }
    size_t size() const noexcept                { return current_size; }
    constexpr size_t max_size() const noexcept  { return max_elements; }

    void fill(const T& value) noexcept
    {
        base::fill(value);
        current_size = max_elements;
    }

    void swap(static_vector& other) noexcept
    {
        base::swap(*this);
        std::swap(current_size, other.current_size);
    }

    void push_back(const T& value) noexcept
    {
        assert(current_size < max_elements);
        *(data() + current_size) = value;
        current_size++;
    }

    void push_back(T&& value) noexcept
    {
        assert(current_size < max_elements);
        *(data() + current_size) = std::move(value);
        current_size++;
    }

    void pop_back() noexcept
    {
        assert(current_size > 0);
        current_size--;
    }

    void resize(size_type new_size) noexcept
    {
        assert(new_size <= max_elements);

        if (current_size > new_size)
        {
            for (size_type i = new_size; i < current_size; i++)
                *(data() + i) = T{};
        }
        else
        {
            for (size_type i = current_size; i < new_size; i++)
                *(data() + i) = T{};
        }

        current_size = new_size;
    }

    reference emplace_back() noexcept
    {
        assert(current_size < max_elements);
        ++current_size;
        back() = T{};
        return back();
    }

private:
    size_type current_size = 0;
};

    typedef uint32_t ObjectType;

    // ObjectTypes namespace contains identifiers for various object types. 
    // All constants have to be distinct. Implementations of NVRHI may extend the list.
    //
    // The encoding is chosen to minimize potential conflicts between implementations.
    // 0x00aabbcc, where:
    //   aa is GAPI, 1 for D3D11, 2 for D3D12, 3 for VK
    //   bb is layer, 0 for native GAPI objects, 1 for reference NVRHI backend, 2 for user-defined backends
    //   cc is a sequential number

    namespace ObjectTypes
    {
        constexpr ObjectType SharedHandle                           = 0x00000001;

        constexpr ObjectType D3D11_Device                           = 0x00010001;
        constexpr ObjectType D3D11_DeviceContext                    = 0x00010002;
        constexpr ObjectType D3D11_Resource                         = 0x00010003;
        constexpr ObjectType D3D11_Buffer                           = 0x00010004;
        constexpr ObjectType D3D11_RenderTargetView                 = 0x00010005;
        constexpr ObjectType D3D11_DepthStencilView                 = 0x00010006;
        constexpr ObjectType D3D11_ShaderResourceView               = 0x00010007;
        constexpr ObjectType D3D11_UnorderedAccessView              = 0x00010008;

        constexpr ObjectType D3D12_Device                           = 0x00020001;
        constexpr ObjectType D3D12_CommandQueue                     = 0x00020002;
        constexpr ObjectType D3D12_GraphicsCommandList              = 0x00020003;
        constexpr ObjectType D3D12_Resource                         = 0x00020004;
        constexpr ObjectType D3D12_RenderTargetViewDescriptor       = 0x00020005;
        constexpr ObjectType D3D12_DepthStencilViewDescriptor       = 0x00020006;
        constexpr ObjectType D3D12_ShaderResourceViewGpuDescripror  = 0x00020007;
        constexpr ObjectType D3D12_UnorderedAccessViewGpuDescripror = 0x00020008;
        constexpr ObjectType D3D12_RootSignature                    = 0x00020009;
        constexpr ObjectType D3D12_PipelineState                    = 0x0002000a;
        constexpr ObjectType D3D12_CommandAllocator                 = 0x0002000b;

        constexpr ObjectType VK_Device                              = 0x00030001;
        constexpr ObjectType VK_PhysicalDevice                      = 0x00030002;
        constexpr ObjectType VK_Instance                            = 0x00030003;
        constexpr ObjectType VK_Queue                               = 0x00030004;
        constexpr ObjectType VK_CommandBuffer                       = 0x00030005;
        constexpr ObjectType VK_DeviceMemory                        = 0x00030006;
        constexpr ObjectType VK_Buffer                              = 0x00030007;
        constexpr ObjectType VK_Image                               = 0x00030008;
        constexpr ObjectType VK_ImageView                           = 0x00030009;
        constexpr ObjectType VK_AccelerationStructureKHR            = 0x0003000a;
        constexpr ObjectType VK_Sampler                             = 0x0003000b;
        constexpr ObjectType VK_ShaderModule                        = 0x0003000c;
        [[deprecated]]
        constexpr ObjectType VK_RenderPass                          = 0x0003000d;
        [[deprecated]]
        constexpr ObjectType VK_Framebuffer                         = 0x0003000e;
        constexpr ObjectType VK_DescriptorPool                      = 0x0003000f;
        constexpr ObjectType VK_DescriptorSetLayout                 = 0x00030010;
        constexpr ObjectType VK_DescriptorSet                       = 0x00030011;
        constexpr ObjectType VK_PipelineLayout                      = 0x00030012;
        constexpr ObjectType VK_Pipeline                            = 0x00030013;
        constexpr ObjectType VK_Micromap                            = 0x00030014;
        constexpr ObjectType VK_ImageCreateInfo                     = 0x00030015;
    };

    struct Object
    {
        union {
            uint64_t integer;
            void* pointer;
        };

        Object(uint64_t i) : integer(i) { }  // NOLINT(cppcoreguidelines-pro-type-member-init)
        Object(void* p) : pointer(p) { }     // NOLINT(cppcoreguidelines-pro-type-member-init)

        template<typename T> operator T* () const { return static_cast<T*>(pointer); }
    };

    class IResource
    {
    protected:
        IResource() = default;
        virtual ~IResource() = default;

    public:
        virtual unsigned long AddRef() = 0;
        virtual unsigned long Release() = 0;
        virtual unsigned long GetRefCount() = 0;

        // Returns a native object or interface, for example ID3D11Device*, or nullptr if the requested interface is unavailable.
        // Does *not* AddRef the returned interface.
        virtual Object getNativeObject(ObjectType objectType) { (void)objectType; return nullptr; }
        
        // Non-copyable and non-movable
        IResource(const IResource&) = delete;
        IResource(const IResource&&) = delete;
        IResource& operator=(const IResource&) = delete;
        IResource& operator=(const IResource&&) = delete;
    };


    //////////////////////////////////////////////////////////////////////////
    // RefCountPtr
    // Mostly a copy of Microsoft::WRL::ComPtr<T>
    //////////////////////////////////////////////////////////////////////////

    template <typename T>
    class RefCountPtr
    {
    public:
        typedef T InterfaceType;

        template <bool b, typename U = void>
        struct EnableIf
        {
        };

        template <typename U>
        struct EnableIf<true, U>
        {
            typedef U type;
        };

    protected:
        InterfaceType *ptr_;
        template<class U> friend class RefCountPtr;

        void InternalAddRef() const noexcept
        {
            if (ptr_ != nullptr)
            {
                ptr_->AddRef();
            }
        }

        unsigned long InternalRelease() noexcept
        {
            unsigned long ref = 0;
            T* temp = ptr_;

            if (temp != nullptr)
            {
                ptr_ = nullptr;
                ref = temp->Release();
            }

            return ref;
        }

    public:

        RefCountPtr() noexcept : ptr_(nullptr)
        {
        }

        RefCountPtr(std::nullptr_t) noexcept : ptr_(nullptr)
        {
        }

        template<class U>
        RefCountPtr(U *other) noexcept : ptr_(other)
        {
            InternalAddRef();
        }

        RefCountPtr(const RefCountPtr& other) noexcept : ptr_(other.ptr_)
        {
            InternalAddRef();
        }

        // copy ctor that allows to instanatiate class when U* is convertible to T*
        template<class U>
        RefCountPtr(const RefCountPtr<U> &other, typename std::enable_if<std::is_convertible<U*, T*>::value, void *>::type * = nullptr) noexcept :
            ptr_(other.ptr_)
        
        {
            InternalAddRef();
        }

        RefCountPtr(RefCountPtr &&other) noexcept : ptr_(nullptr)
        {
            if (this != reinterpret_cast<RefCountPtr*>(&reinterpret_cast<unsigned char&>(other)))
            {
                Swap(other);
            }
        }

        // Move ctor that allows instantiation of a class when U* is convertible to T*
        template<class U>
        RefCountPtr(RefCountPtr<U>&& other, typename std::enable_if<std::is_convertible<U*, T*>::value, void *>::type * = nullptr) noexcept :
            ptr_(other.ptr_)
        {
            other.ptr_ = nullptr;
        }

        ~RefCountPtr() noexcept
        {
            InternalRelease();
        }

        RefCountPtr& operator=(std::nullptr_t) noexcept
        {
            InternalRelease();
            return *this;
        }

        RefCountPtr& operator=(T *other) noexcept
        {
            if (ptr_ != other)
            {
                RefCountPtr(other).Swap(*this);
            }
            return *this;
        }

        template <typename U>
        RefCountPtr& operator=(U *other) noexcept
        {
            RefCountPtr(other).Swap(*this);
            return *this;
        }

        RefCountPtr& operator=(const RefCountPtr &other) noexcept  // NOLINT(bugprone-unhandled-self-assignment)
        {
            if (ptr_ != other.ptr_)
            {
                RefCountPtr(other).Swap(*this);
            }
            return *this;
        }

        template<class U>
        RefCountPtr& operator=(const RefCountPtr<U>& other) noexcept
        {
            RefCountPtr(other).Swap(*this);
            return *this;
        }

        RefCountPtr& operator=(RefCountPtr &&other) noexcept
        {
            RefCountPtr(static_cast<RefCountPtr&&>(other)).Swap(*this);
            return *this;
        }

        template<class U>
        RefCountPtr& operator=(RefCountPtr<U>&& other) noexcept
        {
            RefCountPtr(static_cast<RefCountPtr<U>&&>(other)).Swap(*this);
            return *this;
        }

        void Swap(RefCountPtr&& r) noexcept
        {
            T* tmp = ptr_;
            ptr_ = r.ptr_;
            r.ptr_ = tmp;
        }

        void Swap(RefCountPtr& r) noexcept
        {
            T* tmp = ptr_;
            ptr_ = r.ptr_;
            r.ptr_ = tmp;
        }

        [[nodiscard]] T* Get() const noexcept
        {
            return ptr_;
        }
        
        operator T*() const
        {
            return ptr_;
        }

        InterfaceType* operator->() const noexcept
        {
            return ptr_;
        }

        T** operator&()   // NOLINT(google-runtime-operator)
        {
            return &ptr_;
        }

        [[nodiscard]] T* const* GetAddressOf() const noexcept
        {
            return &ptr_;
        }

        [[nodiscard]] T** GetAddressOf() noexcept
        {
            return &ptr_;
        }

        [[nodiscard]] T** ReleaseAndGetAddressOf() noexcept
        {
            InternalRelease();
            return &ptr_;
        }

        T* Detach() noexcept
        {
            T* ptr = ptr_;
            ptr_ = nullptr;
            return ptr;
        }

        // Set the pointer while keeping the object's reference count unchanged
        void Attach(InterfaceType* other)
        {
            if (ptr_ != nullptr)
            {
                auto ref = ptr_->Release();
                (void)ref;

                // Attaching to the same object only works if duplicate references are being coalesced. Otherwise
                // re-attaching will cause the pointer to be released and may cause a crash on a subsequent dereference.
                assert(ref != 0 || ptr_ != other);
            }

            ptr_ = other;
        }

        // Create a wrapper around a raw object while keeping the object's reference count unchanged
        static RefCountPtr<T> Create(T* other)
        {
            RefCountPtr<T> Ptr;
            Ptr.Attach(other);
            return Ptr;
        }

        unsigned long Reset()
        {
            return InternalRelease();
        }
    };    // RefCountPtr

    typedef RefCountPtr<IResource> ResourceHandle;

    //////////////////////////////////////////////////////////////////////////
    // RefCounter<T>
    // A class that implements reference counting in a way compatible with RefCountPtr.
    // Intended usage is to use it as a base class for interface implementations, like so:
    // class Texture : public RefCounter<ITexture> { ... }
    //////////////////////////////////////////////////////////////////////////

    template<class T>
    class RefCounter : public T
    {
    private:
        std::atomic<unsigned long> m_refCount = 1;
    public:
        virtual unsigned long AddRef() override 
        {
            return ++m_refCount;
        }

        virtual unsigned long Release() override
        {
            unsigned long result = --m_refCount;
            if (result == 0) {
                delete this;
            }
            return result;
        }

        virtual unsigned long GetRefCount() override 
        {
            return m_refCount.load();
        }
    };

    template <class T>
    void hash_combine(size_t& seed, const T& v)
    {
        std::hash<T> hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

} // namespace nvrhi

namespace std
{
    template<typename T> struct hash<nvrhi::RefCountPtr<T>>
    {
        std::size_t operator()(nvrhi::RefCountPtr<T> const& s) const noexcept
        {
            std::hash<T*> hash;
            return hash(s.Get());
        }
    };
}
