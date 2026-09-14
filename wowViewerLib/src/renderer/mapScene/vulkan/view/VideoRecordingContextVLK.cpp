//
// Created for video recording with Nvidia Video Codec SDK
//

#include "VideoRecordingContextVLK.h"
#include "SimpleMp4Muxer.h"
#include "../../../../gapi/vulkan/textures/GTextureVLK.h"
#include <iostream>
#include <cstring>
#include <stdexcept>
#include <algorithm>

#ifdef NVENC_AVAILABLE
// Include NVIDIA Video Codec SDK headers
#ifdef _WIN32
#include <windows.h>
#endif

// CUDA headers
#include <cuda.h>

// CUDA NPP (NVIDIA Performance Primitives) headers for image processing
#include <nppi.h>
#include <nppi_geometry_transforms.h>
#include <npp.h>

// NVENC headers
#include "nvEncodeAPI.h"

// Helper macros for error checking
#define NVENC_API_CALL(nvencAPI, session)                                                                  \
    do {                                                                                                   \
        NVENCSTATUS errorCode = nvencAPI;                                                                  \
        if (errorCode != NV_ENC_SUCCESS) {                                                                 \
            auto errString = nvencFuncs->nvEncGetLastErrorString(m_nvencSession);                          \
            std::cerr << #nvencAPI << " returned error " << errorCode << " errString = " << errString << std::endl;    \
            throw std::runtime_error("NVENC API call failed");                                             \
        }                                                                                                  \
    } while (0)

#define CUDA_API_CALL(cuAPI)                                                                               \
    do {                                                                                                   \
        CUresult errorCode = cuAPI;                                                                        \
        if (errorCode != CUDA_SUCCESS) {                                                                   \
            const char* errorStr = nullptr;                                                                \
            cuGetErrorString(errorCode, &errorStr);                                                        \
            std::cerr << #cuAPI << " returned error " << errorCode << ": " << (errorStr ? errorStr : "unknown") << std::endl; \
            throw std::runtime_error("CUDA API call failed");                                              \
        }                                                                                                  \
    } while (0)

#else // !NVENC_AVAILABLE
// Stub implementation when NVENC is not available
#endif // NVENC_AVAILABLE

VideoRecordingContextVLK::VideoRecordingContextVLK(uint32_t framebufferWidth, uint32_t framebufferHeight,
                                                     uint32_t outputWidth, uint32_t outputHeight,
                                                     const std::string &outputFilename, std::shared_ptr<IRenderView> renderView)
    : m_framebufferWidth(framebufferWidth), m_framebufferHeight(framebufferHeight),
      m_outputWidth(outputWidth), m_outputHeight(outputHeight),
      m_outputFilename(outputFilename), m_renderView(renderView),
      m_nvencEncoder(nullptr), m_nvencSession(nullptr), m_cuContext(nullptr), m_registeredResource(nullptr),
      m_cudaDevice(-1), m_usingPrimaryContext(false),
      m_encoderInitialized(false), m_currentInputBufferIndex(0), m_frameCount(0) {
    
#ifdef NVENC_AVAILABLE
    try {
        // Initialize NVENC encoder
        initializeNVENC();
        
        std::cout << "VideoRecordingContextVLK initialized successfully for " << outputFilename << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize VideoRecordingContextVLK: " << e.what() << std::endl;
        cleanupNVENC();
        throw;
    }
#else
    std::cerr << "VideoRecordingContextVLK: NVENC support not available - built without NVIDIA Video Codec SDK" << std::endl;
    throw std::runtime_error("NVENC support not available");
#endif
}

VideoRecordingContextVLK::~VideoRecordingContextVLK() {
#ifdef NVENC_AVAILABLE
    finalizeEncoding();
    cleanupNVENC();
#endif
}

#ifdef NVENC_AVAILABLE

void* VideoRecordingContextVLK::createCudaContext() {
    // Initialize CUDA
    CUDA_API_CALL(cuInit(0));
    
    // Get CUDA device
    CUdevice cudaDevice;
    CUDA_API_CALL(cuDeviceGet(&cudaDevice, 0)); // Use first CUDA device
    
    // Store the device for cleanup
    m_cudaDevice = cudaDevice;
    
    // Print device information for debugging
    char deviceName[256];
    cuDeviceGetName(deviceName, sizeof(deviceName), cudaDevice);
    std::cout << "Creating CUDA context on device: " << deviceName << std::endl;
    
    // Try using primary context first (recommended approach for interop scenarios)
    CUcontext cuContext;
    CUresult result = cuDevicePrimaryCtxRetain(&cuContext, cudaDevice);
    if (result == CUDA_SUCCESS) {
        std::cout << "Using primary CUDA context" << std::endl;
        m_usingPrimaryContext = true;
        CUDA_API_CALL(cuCtxSetCurrent(cuContext));
        return cuContext;
    }
    
    // If primary context fails, fall back to explicit context creation with v4 API
    std::cout << "Primary context failed, creating explicit context" << std::endl;
    m_usingPrimaryContext = false;
    
    // Properly initialize CUctxCreateParams for CUDA 13 v4 API
    CUctxCreateParams ctxParams;
    memset(&ctxParams, 0, sizeof(CUctxCreateParams));
    ctxParams.numExecAffinityParams = 0;  // No execution affinity
    ctxParams.execAffinityParams = nullptr;
    
    // Use CU_CTX_SCHED_AUTO flag
    CUDA_API_CALL(cuCtxCreate(&cuContext, &ctxParams, CU_CTX_SCHED_AUTO, cudaDevice));
    
    return cuContext;
}

void VideoRecordingContextVLK::destroyCudaContext() {
    if (m_cuContext != nullptr) {
        if (m_usingPrimaryContext && m_cudaDevice >= 0) {
            // Release primary context
            std::cout << "Releasing primary CUDA context" << std::endl;
            cuDevicePrimaryCtxRelease(m_cudaDevice);
        } else {
            // Destroy explicit context
            std::cout << "Destroying explicit CUDA context" << std::endl;
            cuCtxDestroy((CUcontext)m_cuContext);
        }
        
        m_cuContext = nullptr;
        m_cudaDevice = -1;
        m_usingPrimaryContext = false;
    }
}

bool VideoRecordingContextVLK::loadNVENCFunctions() {
    // Allocate function list
    NV_ENCODE_API_FUNCTION_LIST* nvencFuncs = new NV_ENCODE_API_FUNCTION_LIST;
    memset(nvencFuncs, 0, sizeof(NV_ENCODE_API_FUNCTION_LIST));
    nvencFuncs->version = NV_ENCODE_API_FUNCTION_LIST_VER;
    
    // Load NVENC functions
    NVENCSTATUS status = NvEncodeAPICreateInstance(nvencFuncs);
    if (status != NV_ENC_SUCCESS) {
        delete nvencFuncs;
        std::cerr << "Failed to create NVENC API instance" << std::endl;
        return false;
    }
    
    m_nvencEncoder = nvencFuncs;
    return true;
}

void VideoRecordingContextVLK::initializeNVENC() {
    // Load NVENC functions
    if (!loadNVENCFunctions()) {
        throw std::runtime_error("Failed to load NVENC functions");
    }
    
    NV_ENCODE_API_FUNCTION_LIST* nvencFuncs = (NV_ENCODE_API_FUNCTION_LIST*)m_nvencEncoder;
    
    std::cout << "NVENC API Function List version: 0x" << std::hex << nvencFuncs->version << std::dec << std::endl;
    
    // Create CUDA context
    m_cuContext = createCudaContext();
    
    // Make sure the context is current before using NVENC
    CUDA_API_CALL(cuCtxSetCurrent((CUcontext)m_cuContext));
    
    // Add a debug output to verify context
    CUcontext currentCtx;
    cuCtxGetCurrent(&currentCtx);
    std::cout << "Current CUDA context: " << currentCtx << " (should match " << m_cuContext << ")" << std::endl;

    if (!nvencFuncs->nvEncOpenEncodeSession)
    {
        std::cerr << "EncodeAPI not found" << std::endl;
    }

    NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS sessionParams = { NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS_VER };
    sessionParams.device = m_cuContext;
    sessionParams.deviceType = NV_ENC_DEVICE_TYPE_CUDA;
    sessionParams.apiVersion = NVENCAPI_VERSION;
    
    std::cout << "Opening NVENC session with adjusted versions:" << std::endl;
    std::cout << "  Structure version: 0x" << std::hex << sessionParams.version << std::dec << std::endl;
    std::cout << "  Device type: " << sessionParams.deviceType << std::endl;
    std::cout << "  Device: " << sessionParams.device << std::endl;
    std::cout << "  API Version: 0x" << std::hex << sessionParams.apiVersion << std::dec << std::endl;
    
    NVENCSTATUS status = nvencFuncs->nvEncOpenEncodeSessionEx(&sessionParams, &m_nvencSession);
    
    if (status != 0) {  // NV_ENC_SUCCESS
        std::cerr << "nvEncOpenEncodeSessionEx returned error " << status << std::endl;
        throw std::runtime_error("NVENC API call failed");
    }
    
    std::cout << "Successfully opened NVENC session!" << std::endl;
    
    // Set encode GUID (H.264)
    GUID encodeGUID = NV_ENC_CODEC_H264_GUID;

    // 1. Get the number of available presets
    uint32_t presetCount = 0;
    NVENC_API_CALL(nvencFuncs->nvEncGetEncodePresetCount(
        m_nvencSession,
        encodeGUID,
        &presetCount
    ), m_nvencSession);

    std::cout << "Available presets: " << presetCount << std::endl;

    // 2. Get the list of preset GUIDs
    std::vector<GUID> presetGUIDs(presetCount);
    uint32_t actualPresetCount = 0;
    NVENC_API_CALL(nvencFuncs->nvEncGetEncodePresetGUIDs(
        m_nvencSession,
        encodeGUID,
        presetGUIDs.data(),
        presetCount,
        &actualPresetCount
    ), m_nvencSession);

    std::vector<GUID> availablePresetGUIDs;
    availablePresetGUIDs.reserve(presetCount);
    for (uint32_t i = 0; i < actualPresetCount; i++) {
        NV_ENC_PRESET_CONFIG presetConfig = {};
        presetConfig.version = NV_ENC_PRESET_CONFIG_VER;
        presetConfig.presetCfg.version = NV_ENC_CONFIG_VER;

        // Get config for this preset to examine it
        NVENCSTATUS status = nvencFuncs->nvEncGetEncodePresetConfigEx(
            m_nvencSession,
            encodeGUID,
            presetGUIDs[i],
            NV_ENC_TUNING_INFO_ULTRA_LOW_LATENCY,
            &presetConfig
        );

        if (status == NV_ENC_SUCCESS) {
            // You can examine presetConfig here
            std::cout << "Preset " << i << " retrieved successfully" << std::endl;
            availablePresetGUIDs.push_back(presetGUIDs[i]);
        }
    }


   // Get preset config first
    GUID selectedPresetGUID = *(availablePresetGUIDs.end()-1);

    NV_ENC_PRESET_CONFIG presetConfig = {};
    presetConfig.version = NV_ENC_PRESET_CONFIG_VER;
    presetConfig.presetCfg.version = NV_ENC_CONFIG_VER;

    NVENC_API_CALL(nvencFuncs->nvEncGetEncodePresetConfigEx(
        m_nvencSession,
        encodeGUID,
        selectedPresetGUID,
        NV_ENC_TUNING_INFO_ULTRA_LOW_LATENCY,
        &presetConfig
    ), m_nvencSession);

    // Modify the config
    NV_ENC_CONFIG encodeConfig = presetConfig.presetCfg;

    // Verify config version
    std::cout << "Config version: " << encodeConfig.version << std::endl;
    std::cout << "Expected version: " << NV_ENC_CONFIG_VER << std::endl;

    // Initialize params
    NV_ENC_INITIALIZE_PARAMS initParams = {};
    initParams.version = NV_ENC_INITIALIZE_PARAMS_VER;
    initParams.encodeGUID = encodeGUID;
    initParams.presetGUID = selectedPresetGUID;
    initParams.encodeWidth = m_outputWidth;
    initParams.encodeHeight = m_outputHeight;
    initParams.darWidth = m_outputWidth;
    initParams.darHeight = m_outputHeight;
    initParams.frameRateNum = 60;
    initParams.frameRateDen = 1;
    initParams.enablePTD = 1; // Enable Picture Type Decision to help with encoding decisions
    initParams.enableEncodeAsync = 0; // Synchronous encoding for simplicity
    initParams.reportSliceOffsets = 0;
    initParams.enableSubFrameWrite = 0;
    initParams.maxEncodeWidth = m_outputWidth;
    initParams.maxEncodeHeight = m_outputHeight;
    initParams.encodeConfig = &encodeConfig;
    initParams.tuningInfo = NV_ENC_TUNING_INFO_HIGH_QUALITY;
    
    // Configure for proper H.264 stream with headers and regular keyframes
    encodeConfig.encodeCodecConfig.h264Config.repeatSPSPPS = 1; // Repeat SPS/PPS in every IDR frame
    encodeConfig.encodeCodecConfig.h264Config.outputBufferingPeriodSEI = 1;
    encodeConfig.encodeCodecConfig.h264Config.outputPictureTimingSEI = 1;
    encodeConfig.encodeCodecConfig.h264Config.outputAUD = 1; // Output Access Unit Delimiters
    encodeConfig.encodeCodecConfig.h264Config.idrPeriod = KEYFRAME_INTERVAL; // IDR frame every KEYFRAME_INTERVAL frames
    encodeConfig.encodeCodecConfig.h264Config.sliceMode = 0; // Whole frame as one slice
    encodeConfig.encodeCodecConfig.h264Config.sliceModeData = 0;
    encodeConfig.gopLength = KEYFRAME_INTERVAL; // GOP length matches keyframe interval
    encodeConfig.frameIntervalP = 1; // Every frame is either I or P frame (no B-frames for simplicity)

    NVENC_API_CALL(nvencFuncs->nvEncInitializeEncoder(m_nvencSession, &initParams), m_nvencSession);

//    // Get and write SPS/PPS headers immediately after initialization
//    // This is critical for H.264 streams - without these headers, decoders can't process the stream
//    uint32_t spsPpsSize = 0;
//    NV_ENC_SEQUENCE_PARAM_PAYLOAD seqParams = {};
//    seqParams.version = NV_ENC_SEQUENCE_PARAM_PAYLOAD_VER;
//    seqParams.inBufferSize = 0;
//    seqParams.spsId = 0;
//    seqParams.ppsId = 0;
//    seqParams.spsppsBuffer = nullptr;
//    seqParams.outSPSPPSPayloadSize = &spsPpsSize;
//
//    // First call to get the size
//    NVENC_API_CALL(nvencFuncs->nvEncGetSequenceParams(m_nvencSession, &seqParams), m_nvencSession);
//
//    if (spsPpsSize > 0) {
//        std::vector<uint8_t> spsPpsBuffer(spsPpsSize);
//        seqParams.spsppsBuffer = spsPpsBuffer.data();
//        seqParams.inBufferSize = spsPpsSize;
//
//        // Second call to get actual data
//        NVENC_API_CALL(nvencFuncs->nvEncGetSequenceParams(m_nvencSession, &seqParams), m_nvencSession);
//
//        // Open output file and write headers
//        m_outputFile.open(m_outputFilename, std::ios::binary);
//        if (!m_outputFile.is_open()) {
//            throw std::runtime_error("Failed to open output file: " + m_outputFilename);
//        }
//
//        m_outputFile.write((const char*)spsPpsBuffer.data(), spsPpsSize);
//        m_outputFile.flush();
//
//        std::cout << "Written SPS/PPS headers: " << spsPpsSize << " bytes" << std::endl;
//    } else {
//        std::cerr << "WARNING: No SPS/PPS headers retrieved!" << std::endl;
//
//        // Still open the file for frame data
//        m_outputFile.open(m_outputFilename, std::ios::binary);
//        if (!m_outputFile.is_open()) {
//            throw std::runtime_error("Failed to open output file: " + m_outputFilename);
//        }
//    }

    // Create input and output buffers
    const int numBuffers = 3; // Triple buffering
    m_inputBuffers.resize(numBuffers);
    m_outputBitstreams.resize(numBuffers);
    
    for (int i = 0; i < numBuffers; i++) {
        // Create input buffer (use output size for encoder)
        NV_ENC_CREATE_INPUT_BUFFER createInputBufferParams = {};
        createInputBufferParams.version = NV_ENC_CREATE_INPUT_BUFFER_VER;
        createInputBufferParams.width = m_outputWidth;
        createInputBufferParams.height = m_outputHeight;
        createInputBufferParams.bufferFmt = NV_ENC_BUFFER_FORMAT_ABGR;
        
        NVENC_API_CALL(nvencFuncs->nvEncCreateInputBuffer(m_nvencSession, &createInputBufferParams), m_nvencSession);
        m_inputBuffers[i] = createInputBufferParams.inputBuffer;
        
        // Create output bitstream buffer
        NV_ENC_CREATE_BITSTREAM_BUFFER createBitstreamBufferParams = {};
        createBitstreamBufferParams.version = NV_ENC_CREATE_BITSTREAM_BUFFER_VER;
        
        NVENC_API_CALL(nvencFuncs->nvEncCreateBitstreamBuffer(m_nvencSession, &createBitstreamBufferParams), m_nvencSession);
        m_outputBitstreams[i] = createBitstreamBufferParams.bitstreamBuffer;
    }
    
    // Initialize MP4 muxer
    // Ensure filename ends with .mp4
    std::string mp4Filename = m_outputFilename;
    if (mp4Filename.size() < 4 || mp4Filename.substr(mp4Filename.size() - 4) != ".mp4") {
        // Replace extension or add .mp4
        size_t dotPos = mp4Filename.rfind('.');
        if (dotPos != std::string::npos) {
            mp4Filename = mp4Filename.substr(0, dotPos) + ".mp4";
        } else {
            mp4Filename += ".mp4";
        }
    }
    m_mp4Muxer = std::make_unique<SimpleMp4Muxer>(mp4Filename, m_outputWidth, m_outputHeight, 60);
    std::cout << "MP4 output file: " << mp4Filename << std::endl;
    
    // Allocate CUDA device memory for image resizing if needed
    if (m_framebufferWidth != m_outputWidth || m_framebufferHeight != m_outputHeight) {
        size_t framebufferSize = m_framebufferWidth * m_framebufferHeight * 4; // RGBA
        size_t outputSize = m_outputWidth * m_outputHeight * 4; // RGBA
        
        CUdeviceptr framebufferPtr, resizedPtr;
        CUDA_API_CALL(cuMemAlloc(&framebufferPtr, framebufferSize));
        CUDA_API_CALL(cuMemAlloc(&resizedPtr, outputSize));
        
        m_cudaFramebufferBuffer = (void*)framebufferPtr;
        m_cudaResizedBuffer = (void*)resizedPtr;
        
        // Initialize NPP stream context
        NppStreamContext* nppCtx = new NppStreamContext;
        nppCtx->hStream = nullptr; // Use default stream
        
        // Get CUDA device properties for NPP context
        CUdevice cuDevice;
        CUDA_API_CALL(cuCtxGetDevice(&cuDevice));
        
        int cudaDeviceId;
        CUDA_API_CALL(cuDeviceGet(&cuDevice, 0));
        
        // Initialize NPP context with device properties
        nppCtx->nCudaDeviceId = 0;
        cuDeviceGetAttribute(&nppCtx->nMultiProcessorCount, CU_DEVICE_ATTRIBUTE_MULTIPROCESSOR_COUNT, cuDevice);
        cuDeviceGetAttribute(&nppCtx->nMaxThreadsPerMultiProcessor, CU_DEVICE_ATTRIBUTE_MAX_THREADS_PER_MULTIPROCESSOR, cuDevice);
        cuDeviceGetAttribute(&nppCtx->nMaxThreadsPerBlock, CU_DEVICE_ATTRIBUTE_MAX_THREADS_PER_BLOCK, cuDevice);
        cuDeviceGetAttribute(reinterpret_cast<int *>(&nppCtx->nSharedMemPerBlock), CU_DEVICE_ATTRIBUTE_MAX_SHARED_MEMORY_PER_BLOCK, cuDevice);
        nppCtx->nCudaDevAttrComputeCapabilityMajor = 0;
        nppCtx->nCudaDevAttrComputeCapabilityMinor = 0;
        cuDeviceGetAttribute(&nppCtx->nCudaDevAttrComputeCapabilityMajor, CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MAJOR, cuDevice);
        cuDeviceGetAttribute(&nppCtx->nCudaDevAttrComputeCapabilityMinor, CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MINOR, cuDevice);
        nppCtx->nStreamFlags = 0;
        
        m_nppStreamContext = nppCtx;
        
        std::cout << "Allocated CUDA buffers for GPU-accelerated image resizing (NPP)" << std::endl;
    }
    
    m_encoderInitialized = true;
    std::cout << "NVENC encoder initialized:" << std::endl;
    std::cout << "  Framebuffer: " << m_framebufferWidth << "x" << m_framebufferHeight << std::endl;
    std::cout << "  Output: " << m_outputWidth << "x" << m_outputHeight << " @ 60fps" << std::endl;
}

void VideoRecordingContextVLK::cleanupNVENC() {
    if (!m_encoderInitialized) {
        return;
    }
    
    NV_ENCODE_API_FUNCTION_LIST* nvencFuncs = (NV_ENCODE_API_FUNCTION_LIST*)m_nvencEncoder;
    
    if (nvencFuncs && m_nvencSession) {
        // Destroy input buffers
        for (void* inputBuffer : m_inputBuffers) {
            if (inputBuffer) {
                nvencFuncs->nvEncDestroyInputBuffer(m_nvencSession, inputBuffer);
            }
        }
        m_inputBuffers.clear();
        
        // Destroy output bitstream buffers
        for (void* outputBitstream : m_outputBitstreams) {
            if (outputBitstream) {
                nvencFuncs->nvEncDestroyBitstreamBuffer(m_nvencSession, outputBitstream);
            }
        }
        m_outputBitstreams.clear();
        
        // Destroy encoder session
        nvencFuncs->nvEncDestroyEncoder(m_nvencSession);
        m_nvencSession = nullptr;
    }
    
    // Free CUDA device memory for resizing
    if (m_cudaFramebufferBuffer) {
        cuMemFree((CUdeviceptr)m_cudaFramebufferBuffer);
        m_cudaFramebufferBuffer = nullptr;
    }
    if (m_cudaResizedBuffer) {
        cuMemFree((CUdeviceptr)m_cudaResizedBuffer);
        m_cudaResizedBuffer = nullptr;
    }
    
    // Clean up NPP stream context
    if (m_nppStreamContext) {
        delete (NppStreamContext*)m_nppStreamContext;
        m_nppStreamContext = nullptr;
    }
    
    // Destroy CUDA context
    destroyCudaContext();
    
    // Clean up function list
    if (m_nvencEncoder) {
        delete (NV_ENCODE_API_FUNCTION_LIST*)m_nvencEncoder;
        m_nvencEncoder = nullptr;
    }
    
    // Finalize MP4 muxer
    if (m_mp4Muxer) {
        m_mp4Muxer->finalize();
        m_mp4Muxer.reset();
    }
    
    m_encoderInitialized = false;
    std::cout << "NVENC encoder cleaned up. Total frames encoded: " << m_frameCount << std::endl;
}

void VideoRecordingContextVLK::encodeFrame(int frameNumber, const void* pixelData) {
    if (!m_encoderInitialized) {
        std::cerr << "Encoder not initialized, skipping frame " << frameNumber << std::endl;
        return;
    }
    
    NV_ENCODE_API_FUNCTION_LIST* nvencFuncs = (NV_ENCODE_API_FUNCTION_LIST*)m_nvencEncoder;
    
    try {
        // Lock input buffer and copy data
        NV_ENC_LOCK_INPUT_BUFFER lockInputBufferParams = {};
        lockInputBufferParams.version = NV_ENC_LOCK_INPUT_BUFFER_VER;
        lockInputBufferParams.inputBuffer = m_inputBuffers[m_currentInputBufferIndex];
        
        NVENC_API_CALL(nvencFuncs->nvEncLockInputBuffer(m_nvencSession, &lockInputBufferParams), m_nvencSession);
        
        // Check if we need to resize
        bool needsResize = (m_framebufferWidth != m_outputWidth) || (m_framebufferHeight != m_outputHeight);
        
        if (needsResize) {
            // Use CUDA NPP for GPU-accelerated image resizing
            // Upload framebuffer data to GPU
            CUDA_API_CALL(cuMemcpyHtoD((CUdeviceptr)m_cudaFramebufferBuffer, pixelData, 
                                       m_framebufferWidth * m_framebufferHeight * 4));
            
            // Set up NPP image descriptors
            NppiSize srcSize = {(int)m_framebufferWidth, (int)m_framebufferHeight};
            NppiRect srcROI = {0, 0, (int)m_framebufferWidth, (int)m_framebufferHeight};
            NppiSize dstSize = {(int)m_outputWidth, (int)m_outputHeight};
            NppiRect dstROI = {0, 0, (int)m_outputWidth, (int)m_outputHeight};
            
            int srcStep = m_framebufferWidth * 4; // bytes per row
            int dstStep = m_outputWidth * 4;
            
            // Perform GPU-accelerated resize using NPP (bilinear interpolation)
            // nppiResize_8u_C4R_Ctx: 8-bit unsigned, 4 channels (RGBA), with context
            NppStreamContext* nppCtx = (NppStreamContext*)m_nppStreamContext;
            NppStatus nppStatus = nppiResize_8u_C4R_Ctx(
                (const Npp8u*)m_cudaFramebufferBuffer, srcStep, srcSize, srcROI,
                (Npp8u*)m_cudaResizedBuffer, dstStep, dstSize, dstROI,
                NPPI_INTER_LINEAR,  // Bilinear interpolation
                *nppCtx
            );
            
            if (nppStatus != NPP_SUCCESS) {
                std::cerr << "NPP resize failed with error code: " << nppStatus << std::endl;
                throw std::runtime_error("NPP image resize failed");
            }
            
            // Allocate temporary host buffer for resized data
            std::vector<uint8_t> resizedData(m_outputWidth * m_outputHeight * 4);
            
            // Download resized data from GPU to host
            CUDA_API_CALL(cuMemcpyDtoH(resizedData.data(), (CUdeviceptr)m_cudaResizedBuffer,
                                       m_outputWidth * m_outputHeight * 4));
            
            // Copy resized data to NVENC input buffer (flip vertically)
            for (uint32_t i = 0; i < m_outputHeight; i++) {
                std::memcpy(
                    (uint8_t*)lockInputBufferParams.bufferDataPtr + ((m_outputHeight - i - 1) * (4 * m_outputWidth)),
                    resizedData.data() + (i * 4 * m_outputWidth),
                    (4 * m_outputWidth)
                );
            }
        } else {
            // No resize needed, copy directly (flip vertically)
            for (uint32_t i = 0; i < m_outputHeight; i++) {
                std::memcpy(
                    (uint8_t*)lockInputBufferParams.bufferDataPtr + ((m_outputHeight - i - 1) * (4 * m_outputWidth)),
                    (uint8_t*)pixelData + (i * 4 * m_outputWidth),
                    (4 * m_outputWidth)
                );
            }
        }

        NVENC_API_CALL(nvencFuncs->nvEncUnlockInputBuffer(m_nvencSession, m_inputBuffers[m_currentInputBufferIndex]), m_nvencSession);
        
        // Encode the frame
        NV_ENC_PIC_PARAMS picParams = {};
        picParams.version = NV_ENC_PIC_PARAMS_VER;
        picParams.inputBuffer = m_inputBuffers[m_currentInputBufferIndex];
        picParams.bufferFmt = NV_ENC_BUFFER_FORMAT_ABGR;
        picParams.inputWidth = m_outputWidth;
        picParams.inputHeight = m_outputHeight;
        picParams.outputBitstream = m_outputBitstreams[m_currentInputBufferIndex];
        picParams.pictureStruct = NV_ENC_PIC_STRUCT_FRAME;
        
        NVENC_API_CALL(nvencFuncs->nvEncEncodePicture(m_nvencSession, &picParams), m_nvencSession);
        
        // Lock output bitstream and write to file
        NV_ENC_LOCK_BITSTREAM lockBitstreamParams = {};
        lockBitstreamParams.version = NV_ENC_LOCK_BITSTREAM_VER;
        lockBitstreamParams.outputBitstream = m_outputBitstreams[m_currentInputBufferIndex];
        lockBitstreamParams.doNotWait = 0;
        
        NVENC_API_CALL(nvencFuncs->nvEncLockBitstream(m_nvencSession, &lockBitstreamParams), m_nvencSession);
        
        // Determine if this is a keyframe based on picture type
        bool isKeyframe = (lockBitstreamParams.pictureType == NV_ENC_PIC_TYPE_IDR);
        
        // Debug: Print bitstream size and picture type
        std::cout << "Frame " << frameNumber << " - Bitstream size: " << lockBitstreamParams.bitstreamSizeInBytes 
                  << " bytes, Picture type: " << lockBitstreamParams.pictureType 
                  << (isKeyframe ? " (KEYFRAME)" : "") << std::endl;
        
        if (lockBitstreamParams.bitstreamSizeInBytes > 0) {
            // Add frame to MP4 muxer
            m_mp4Muxer->addFrame(
                static_cast<const uint8_t*>(lockBitstreamParams.bitstreamBufferPtr),
                lockBitstreamParams.bitstreamSizeInBytes,
                isKeyframe
            );
        } else {
            std::cerr << "WARNING: Frame " << frameNumber << " produced zero bytes of output!" << std::endl;
        }
        
        NVENC_API_CALL(nvencFuncs->nvEncUnlockBitstream(m_nvencSession, m_outputBitstreams[m_currentInputBufferIndex]), m_nvencSession);
        
        // Move to next buffer
        m_currentInputBufferIndex = (m_currentInputBufferIndex + 1) % m_inputBuffers.size();
        m_frameCount++;
        
        if (frameNumber % 60 == 0) {
            std::cout << "Encoded frame " << frameNumber << " (total: " << m_frameCount << ")" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error encoding frame " << frameNumber << ": " << e.what() << std::endl;
    }
}

void VideoRecordingContextVLK::finalizeEncoding() {
    if (!m_encoderInitialized) {
        return;
    }
    
    NV_ENCODE_API_FUNCTION_LIST* nvencFuncs = (NV_ENCODE_API_FUNCTION_LIST*)m_nvencEncoder;
    
    std::cout << "Finalizing encoding - flushing remaining frames..." << std::endl;
    
    // Flush any remaining frames in the encoder pipeline
    // With synchronous encoding and triple buffering, there might be frames still being processed
    for (size_t i = 0; i < m_inputBuffers.size(); i++) {
        NV_ENC_PIC_PARAMS picParams = {};
        picParams.version = NV_ENC_PIC_PARAMS_VER;
        picParams.encodePicFlags = NV_ENC_PIC_FLAG_EOS;
        
        NVENCSTATUS status = nvencFuncs->nvEncEncodePicture(m_nvencSession, &picParams);
        if (status != NV_ENC_SUCCESS) {
            // EOS might fail if there are no more frames to flush, which is okay
            break;
        }
        
        // Try to retrieve any flushed frame
        NV_ENC_LOCK_BITSTREAM lockBitstreamParams = {};
        lockBitstreamParams.version = NV_ENC_LOCK_BITSTREAM_VER;
        lockBitstreamParams.outputBitstream = m_outputBitstreams[i];
        lockBitstreamParams.doNotWait = 0;
        
        status = nvencFuncs->nvEncLockBitstream(m_nvencSession, &lockBitstreamParams);
        if (status == NV_ENC_SUCCESS) {
            if (lockBitstreamParams.bitstreamSizeInBytes > 0) {
                bool isKeyframe = (lockBitstreamParams.pictureType == 3);
                std::cout << "Flushed frame " << i << " - " << lockBitstreamParams.bitstreamSizeInBytes << " bytes" 
                          << (isKeyframe ? " (KEYFRAME)" : "") << std::endl;

                if (m_mp4Muxer) {
                    m_mp4Muxer->addFrame(
                        static_cast<const uint8_t*>(lockBitstreamParams.bitstreamBufferPtr),
                        lockBitstreamParams.bitstreamSizeInBytes,
                        isKeyframe
                    );
                }
            }
            nvencFuncs->nvEncUnlockBitstream(m_nvencSession, m_outputBitstreams[i]);
        }
    }
    
    // Finalize MP4 - writes moov box with all sample information
    if (m_mp4Muxer) {
        m_mp4Muxer->finalize();
        m_mp4Muxer.reset();
    }
    
    std::cout << "Video encoding finalized. Total frames: " << m_frameCount << std::endl;
}

#else // !NVENC_AVAILABLE

// Stub implementations when NVENC is not available
void* VideoRecordingContextVLK::createCudaContext() {
    throw std::runtime_error("NVENC support not available");
}

void VideoRecordingContextVLK::destroyCudaContext() {
}

bool VideoRecordingContextVLK::loadNVENCFunctions() {
    return false;
}

void VideoRecordingContextVLK::initializeNVENC() {
    throw std::runtime_error("NVENC support not available");
}

void VideoRecordingContextVLK::cleanupNVENC() {
}

void VideoRecordingContextVLK::encodeFrame(int frameNumber, const void* pixelData) {
    std::cerr << "encodeFrame called but NVENC support not available" << std::endl;
}

void VideoRecordingContextVLK::finalizeEncoding() {
}

#endif // NVENC_AVAILABLE

