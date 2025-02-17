﻿#pragma once
#include "GLHelper.hpp"

namespace ln {
namespace detail {

class GLIndexBuffer : public RHIResource {
public:
    GLIndexBuffer();
    virtual ~GLIndexBuffer();
    Result<> init(GraphicsResourceUsage usage, IndexBufferFormat format, uint32_t indexCount, const void* initialData);
    void onDestroy() override;

    GLuint objectId() const { return m_objectId; }
    IndexBufferFormat format() const { return m_format; }

    void setSubData(size_t offset, const void* data, size_t length);
    void* map(uint64_t offset, uint64_t size);
    void* map() override { return map(0, m_size); }
    void unmap() override;

private:
    GLuint m_objectId;
    IndexBufferFormat m_format;
    uint64_t m_size;
};

} // namespace detail
} // namespace ln
