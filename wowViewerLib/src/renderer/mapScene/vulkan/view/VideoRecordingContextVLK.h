//
// Created for video recording with Nvidia Video Codec SDK
//

#ifndef AWEBWOWVIEWERCPP_VIDEORECORDINGCONTEXTVLK_H
#define AWEBWOWVIEWERCPP_VIDEORECORDINGCONTEXTVLK_H

#include "../../../../gapi/vulkan/GDeviceVulkan.h"
#include "../../MapSceneParams.h"
#include <string>
#include <memory>
#include <vector>
#include <fstream>

// Forward declare MP4 muxer
class SimpleMp4Muxer;

// Forward declarations for NVENC types
//struct NV_ENC_INITIALIZE_PARAMS;
//struct NV_ENC_CONFIG;
//struct NV_ENC_REGISTERED_PTR;
//struct NV_ENC_INPUT_PTR;
//struct NV_ENC_OUTPUT_PTR;

class VideoRecordingContextVLK : public IVideoRecordingContext {
public:
    VideoRecordingContextVLK(uint32_t framebufferWidth, uint32_t framebufferHeight, 
                             uint32_t outputWidth, uint32_t outputHeight,
                             const std::string &outputFilename, std::shared_ptr<IRenderView> renderView);
    ~VideoRecordingContextVLK() override;

    // Get the video output dimensions (what gets encoded to file)
    uint32_t getWidth() const override { return m_outputWidth; }
    uint32_t getHeight() const override { return m_outputHeight; }
    // Get the framebuffer dimensions (what gets rendered)
    uint32_t getFramebufferWidth() const override { return m_framebufferWidth; }
    uint32_t getFramebufferHeight() const override { return m_framebufferHeight; }
    std::shared_ptr<IRenderView> getRenderView() const override { return m_renderView; }
    const std::string& getOutputFilename() const { return m_outputFilename; }

    // Feed a frame to the encoder
    // frameNumber is the current frame number
    // pixelData should be RGBA data with size framebufferWidth * framebufferHeight * 4 bytes
    void encodeFrame(int frameNumber, const void* pixelData);
    
    // Finalize encoding and write remaining frames
    void finalizeEncoding();

private:
    uint32_t m_framebufferWidth;   // Size of input framebuffer
    uint32_t m_framebufferHeight;
    uint32_t m_outputWidth;        // Size of output video
    uint32_t m_outputHeight;
    std::string m_outputFilename;
    std::shared_ptr<IRenderView> m_renderView;

    // Nvidia Video Codec SDK specific members
    void* m_nvencEncoder = nullptr;          // NV_ENCODE_API_FUNCTION_LIST*
    void* m_nvencSession = nullptr;          // void* (encoder session)
    void* m_cuContext = nullptr;             // CUcontext
    void* m_registeredResource = nullptr;    // NV_ENC_REGISTERED_PTR
    std::vector<void*> m_inputBuffers;       // Vector of NV_ENC_INPUT_PTR
    std::vector<void*> m_outputBitstreams;   // Vector of NV_ENC_OUTPUT_PTR
    int m_cudaDevice = -1;                   // CUdevice (-1 means not initialized)
    bool m_usingPrimaryContext = false;      // Track if we're using primary context
    
    // CUDA device memory for image resizing
    void* m_cudaFramebufferBuffer = nullptr; // CUdeviceptr for input framebuffer data
    void* m_cudaResizedBuffer = nullptr;     // CUdeviceptr for resized output data
    void* m_nppStreamContext = nullptr;      // NppStreamContext for NPP operations
    
    // Output file
    std::ofstream m_outputFile;
    std::unique_ptr<SimpleMp4Muxer> m_mp4Muxer;
    bool m_encoderInitialized = false;
    int m_currentInputBufferIndex = 0;
    
    // Frame tracking
    uint32_t m_frameCount = 0;
    
    // Keyframe interval (in frames) - 1 second at 60fps
    static constexpr uint32_t KEYFRAME_INTERVAL = 60;
    
    void initializeNVENC();
    void cleanupNVENC();
    bool loadNVENCFunctions();
    void* createCudaContext();
    void destroyCudaContext();
};

typedef std::shared_ptr<VideoRecordingContextVLK> HVideoRecordingContextVLK;

#endif //AWEBWOWVIEWERCPP_VIDEORECORDINGCONTEXTVLK_H

