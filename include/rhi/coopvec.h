/*
* Copyright (c) 2014-2021, NVIDIA CORPORATION. All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
* DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include <nvrhi/resource.h>

#include <vector>

namespace nvrhi
{
    //////////////////////////////////////////////////////////////////////////
    // Linear Algebra / Cooperative Vectors
    //////////////////////////////////////////////////////////////////////////

    namespace coopvec
    {
        enum class DataType
        {
            UInt8,
            SInt8,
            UInt8Packed,
            SInt8Packed,
            UInt16,
            SInt16,
            UInt32,
            SInt32,
            UInt64,
            SInt64,
            FloatE4M3,
            FloatE5M2,
            Float16,
            BFloat16,
            Float32,
            Float64
        };

        enum class MatrixLayout
        {
            RowMajor,
            ColumnMajor,
            InferencingOptimal,
            TrainingOptimal
        };

        // Describes a combination of input and output data types for matrix multiplication with Cooperative Vectors.
        // - DX12: Maps from D3D12_COOPERATIVE_VECTOR_PROPERTIES_MUL.
        // - Vulkan: Maps from VkCooperativeVectorPropertiesNV.
        struct MatMulFormatCombo
        {
            DataType inputType;
            DataType inputInterpretation;
            DataType matrixInterpretation;
            DataType biasInterpretation;
            DataType outputType;
            bool transposeSupported;
        };

        struct DeviceFeatures
        {
            // Format combinations supported by the device for matrix multiplication with Cooperative Vectors.
            std::vector<MatMulFormatCombo> matMulFormats;

            // - DX12: True if FLOAT16 is supported as accumulation format for both outer product accumulation
            //         and vector accumulation.
            // - Vulkan: True if cooperativeVectorTrainingFloat16Accumulation is supported.
            bool trainingFloat16 = false;

            // - DX12: True if FLOAT32 is supported as accumulation format for both outer product accumulation
            //         and vector accumulation.
            // - Vulkan: True if cooperativeVectorTrainingFloat32Accumulation is supported.
            bool trainingFloat32 = false;
        };

        struct MatrixLayoutDesc
        {
            // Buffer where the matrix is stored.
            nvrhi::IBuffer* buffer = nullptr;

            // Offset in bytes from the start of the buffer where the matrix starts.
            uint64_t offset = 0;

            // Data type of the matrix elements.
            DataType type = DataType::UInt8;

            // Layout of the matrix in memory.
            MatrixLayout layout = MatrixLayout::RowMajor;

            // Size in bytes of the matrix.
            size_t size = 0;

            // Stride in bytes between rows or columns, depending on the layout.
            // For RowMajor and ColumnMajor layouts, stride may be zero, in which case it is computed automatically.
            // For InferencingOptimal and TrainingOptimal layouts, stride does not matter and should be zero.
            size_t stride = 0;
        };

        // Describes a single matrix layout conversion operation.
        // Used by ICommandList::convertCoopVecMatrices(...)
        struct ConvertMatrixLayoutDesc
        {
            MatrixLayoutDesc src;
            MatrixLayoutDesc dst;

            uint32_t numRows = 0;
            uint32_t numColumns = 0;
        };

        // Returns the size in bytes of a given data type.
        NVRHI_API size_t getDataTypeSize(DataType type);

        // Returns the stride for a given matrix if it's stored in a RowMajor or ColumnMajor layout.
        // For other layouts, returns 0.
        NVRHI_API size_t getOptimalMatrixStride(DataType type, MatrixLayout layout, uint32_t rows, uint32_t columns);
    }
} // namespace nvrhi
