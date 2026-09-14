// Example: How to use VideoRecordingContext for recording gameplay footage
//
// This is a simple example demonstrating how to record video from your render view.
// This file is for documentation purposes only and is not compiled.

#include "../../../../gapi/IDevice.h"
#include "../../MapSceneParams.h"
#include <iostream>
#include <memory>

// Example 1: Basic video recording
void basicVideoRecording(std::shared_ptr<IRenderView> renderView) {
    // Create a video recording context
    // Output will be a raw H.264 file that can be converted to MP4 with FFmpeg
    auto recordingContext = renderView->createVideoRecordingContext(
        1920,                          // Width in pixels
        1080,                          // Height in pixels  
        "gameplay_recording.h264"      // Output filename
    );
    
    if (!recordingContext) {
        std::cerr << "Failed to create video recording context. " 
                  << "Make sure NVENC support is available." << std::endl;
        return;
    }
    
    std::cout << "Started recording to gameplay_recording.h264" << std::endl;
    
    // Your render loop
    int frameNumber = 0;
    bool isRecording = true;
    
    while (isRecording) {
        // ... your normal rendering code here ...
        
        // Feed the rendered frame to the encoder
        renderView->feedFrameToVideoRecording(recordingContext, frameNumber);
        
        frameNumber++;
        
        // Stop recording after 600 frames (10 seconds at 60fps)
        if (frameNumber >= 600) {
            isRecording = false;
        }
    }
    
    // Finalize the recording
    recordingContext.reset();
    
    std::cout << "Recording finished. Total frames: " << frameNumber << std::endl;
    std::cout << "Convert to MP4 with: ffmpeg -r 60 -i gameplay_recording.h264 -c:v copy output.mp4" << std::endl;
}

// Example 2: Recording with error handling
void robustVideoRecording(std::shared_ptr<IRenderView> renderView) {
    std::shared_ptr<IVideoRecordingContext> recordingContext;
    
    try {
        recordingContext = renderView->createVideoRecordingContext(
            3840,  // 4K resolution
            2160,
            "4k_recording.h264"
        );
        
        if (!recordingContext) {
            throw std::runtime_error("Failed to create recording context");
        }
        
        std::cout << "Recording started at 4K resolution" << std::endl;
        
        int frameNumber = 0;
        bool shouldContinue = true;
        
        while (shouldContinue) {
            try {
                // Render your scene
                // ... rendering code ...
                
                // Encode the frame
                renderView->feedFrameToVideoRecording(recordingContext, frameNumber);
                
                // Print progress every second (60 frames)
                if (frameNumber % 60 == 0) {
                    std::cout << "Recorded " << (frameNumber / 60) << " seconds" << std::endl;
                }
                
                frameNumber++;
                
                // Check stop condition
                shouldContinue = checkIfShouldContinueRecording();
                
            } catch (const std::exception& e) {
                std::cerr << "Error during frame " << frameNumber << ": " << e.what() << std::endl;
                // Decide whether to continue or abort
                shouldContinue = false;
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Recording error: " << e.what() << std::endl;
    }
    
    // Cleanup (destructor handles finalization)
    recordingContext.reset();
    std::cout << "Recording finalized" << std::endl;
}

// Example 3: Recording multiple clips
class VideoRecorder {
public:
    VideoRecorder(std::shared_ptr<IRenderView> renderView)
        : m_renderView(renderView), m_isRecording(false), m_currentFrame(0) {}
    
    bool startRecording(const std::string& filename, uint32_t width, uint32_t height) {
        if (m_isRecording) {
            std::cerr << "Already recording!" << std::endl;
            return false;
        }
        
        m_recordingContext = m_renderView->createVideoRecordingContext(width, height, filename);
        if (!m_recordingContext) {
            std::cerr << "Failed to create recording context" << std::endl;
            return false;
        }
        
        m_isRecording = true;
        m_currentFrame = 0;
        m_currentFilename = filename;
        
        std::cout << "Started recording to " << filename << std::endl;
        return true;
    }
    
    void recordFrame(int frameNumber) {
        if (!m_isRecording) {
            return;
        }
        
        m_renderView->feedFrameToVideoRecording(m_recordingContext, frameNumber);
        m_currentFrame++;
    }
    
    void stopRecording() {
        if (!m_isRecording) {
            return;
        }
        
        m_recordingContext.reset();
        m_isRecording = false;
        
        std::cout << "Stopped recording " << m_currentFilename 
                  << " (" << m_currentFrame << " frames)" << std::endl;
    }
    
    bool isRecording() const { return m_isRecording; }
    int getCurrentFrame() const { return m_currentFrame; }
    
private:
    std::shared_ptr<IRenderView> m_renderView;
    std::shared_ptr<IVideoRecordingContext> m_recordingContext;
    bool m_isRecording;
    int m_currentFrame;
    std::string m_currentFilename;
};

// Usage of VideoRecorder class
void recordMultipleClips(std::shared_ptr<IRenderView> renderView) {
    VideoRecorder recorder(renderView);
    
    // Record first clip
    recorder.startRecording("clip1.h264", 1920, 1080);
    for (int i = 0; i < 300; i++) {  // 5 seconds at 60fps
        // ... render ...
        recorder.recordFrame(i);
    }
    recorder.stopRecording();
    
    // Do something else without recording
    // ... 
    
    // Record second clip
    recorder.startRecording("clip2.h264", 1920, 1080);
    for (int i = 0; i < 600; i++) {  // 10 seconds at 60fps
        // ... render ...
        recorder.recordFrame(i);
    }
    recorder.stopRecording();
}

// Example 4: Conditional recording (record only when action happens)
void conditionalRecording(std::shared_ptr<IRenderView> renderView) {
    std::shared_ptr<IVideoRecordingContext> recordingContext;
    bool wasRecording = false;
    int frameNumber = 0;
    int clipNumber = 0;
    
    while (true) {  // Main game loop
        // ... render scene ...
        
        bool shouldRecord = isInterestingActionHappening();  // Your game logic
        
        if (shouldRecord && !wasRecording) {
            // Start recording
            std::string filename = "highlight_" + std::to_string(clipNumber++) + ".h264";
            recordingContext = renderView->createVideoRecordingContext(1920, 1080, filename);
            if (recordingContext) {
                std::cout << "Started recording highlight" << std::endl;
                wasRecording = true;
            }
        }
        
        if (wasRecording) {
            if (shouldRecord) {
                // Continue recording
                renderView->feedFrameToVideoRecording(recordingContext, frameNumber);
            } else {
                // Stop recording
                recordingContext.reset();
                wasRecording = false;
                std::cout << "Stopped recording highlight" << std::endl;
            }
        }
        
        frameNumber++;
    }
}

// Example 5: Recording with custom resolution
void recordCustomResolution(std::shared_ptr<IRenderView> renderView) {
    struct Resolution {
        uint32_t width;
        uint32_t height;
        std::string name;
    };
    
    std::vector<Resolution> resolutions = {
        {1280, 720, "720p"},
        {1920, 1080, "1080p"},
        {2560, 1440, "1440p"},
        {3840, 2160, "4K"}
    };
    
    // Let user select resolution
    int choice = 1;  // Default to 1080p
    Resolution selected = resolutions[choice];
    
    auto recordingContext = renderView->createVideoRecordingContext(
        selected.width,
        selected.height,
        "recording_" + selected.name + ".h264"
    );
    
    if (!recordingContext) {
        std::cerr << "Failed to create " << selected.name << " recording context" << std::endl;
        return;
    }
    
    std::cout << "Recording at " << selected.name 
              << " (" << selected.width << "x" << selected.height << ")" << std::endl;
    
    // ... recording loop ...
}

// Dummy functions used in examples
bool checkIfShouldContinueRecording() {
    // Your logic to determine if recording should continue
    return true;
}

bool isInterestingActionHappening() {
    // Your game logic to detect interesting events
    return false;
}

/*
 * Notes on converting the output:
 * 
 * The output is a raw H.264 bitstream. To create a standard video file:
 * 
 * 1. Simple MP4 conversion (no audio):
 *    ffmpeg -r 60 -i recording.h264 -c:v copy output.mp4
 * 
 * 2. MP4 with silent audio track:
 *    ffmpeg -r 60 -i recording.h264 -c:v copy -f lavfi -i anullsrc -c:a aac -shortest output.mp4
 * 
 * 3. Re-encode with different quality:
 *    ffmpeg -r 60 -i recording.h264 -c:v libx264 -crf 18 -preset medium output.mp4
 * 
 * 4. Convert to WebM:
 *    ffmpeg -r 60 -i recording.h264 -c:v libvpx-vp9 -b:v 2M output.webm
 * 
 * 5. Create GIF (for short clips):
 *    ffmpeg -r 60 -i recording.h264 -vf "fps=30,scale=640:-1:flags=lanczos" -c:v gif output.gif
 */

