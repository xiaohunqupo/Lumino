﻿// Copyright (c) 2019+ lriki. Distributed under the MIT license.
#pragma once

namespace ln {

class VulkanIntegration
{
public:
    static void getImageInfo(GraphicsCommandList* graphicsContext, RenderTargetTexture* texture, VkImage* outImage, VkImageView* outImageView, VkFormat* outFormat, int* outWidth, int* outHeight);
    static void getImageInfo(GraphicsCommandList* graphicsContext, DepthBuffer* texture, VkImage* outImage, VkImageView* outImageView, VkFormat* outFormat, int* outWidth, int* outHeight);
};

} // namespace ln
