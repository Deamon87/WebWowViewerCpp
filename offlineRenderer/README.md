# Offline Renderer

A standalone command-line tool for rendering WoW scenes to PNG images offline, without requiring a GUI or interactive session.

## Building

The offlineRenderer is automatically included when building the main project. It will be built alongside other targets.

```bash
mkdir build
cd build
cmake ..
cmake --build . --target offlineRenderer
```

## Usage

The offlineRenderer accepts command-line arguments to configure the rendering:

### Basic Options

- `-casc <path>` - Path to the CASC filesystem (required, or set `CASC_PATH` environment variable)
- `-output <path>` - Output PNG file path (default: `output.png`)
- `-size <width> <height>` - Output image dimensions (default: 1024x1024)

### Scene Options

Render a map:
```
-map <mapId> <wdtFileId>
```

Render a WMO:
```
-wmo <wmoFileId>
```

### Camera Options

- `-camera <x> <y> <z>` - Camera position (default: 0 0 0)
- `-lookat <x> <y> <z>` - Camera look-at point (default: 0 0 0)
- `-time <time>` - Time override for animations (default: 0)

### Graphics API Options

- `-gapi <name>` - Graphics API name (default: `vulkan`)
- `-bindless` - Enable bindless support
- `-validation` - Enable validation layers

## Examples

### Render a map scene:

```bash
./offlineRenderer -casc /path/to/wow -map 530 828395 \
  -camera 9305.81 -7442.45 236.1 \
  -lookat 9304.83 -7442.5 235.909 \
  -output result.png
```

### Render a WMO scene:

```bash
./offlineRenderer -casc /path/to/wow -wmo 1576792 \
  -camera -14.3089 1.57191 3.31259 \
  -lookat -13.3302 1.47767 3.13035 \
  -output wmo_result.png
```

### Custom output size:

```bash
./offlineRenderer -casc /path/to/wow -map 0 775971 \
  -camera -4933.28 -950.835 512.3 \
  -lookat -4932.4 -951.284 512.11 \
  -size 2048 2048 \
  -output highres.png
```

## How It Works

The offlineRenderer follows the same setup and rendering process as the test framework:

1. **Setup**: Initializes the rendering system, creates the CASC filesystem connection, database, graphics device, scene composer, and scene window.

2. **Scene Loading**: Opens the specified map or WMO scene with the given camera parameters.

3. **Frame Processing**: Continuously processes frames until all data is loaded. The `isFrameLoaded()` function checks if:
   - All request processor jobs are completed
   - The render plan exists
   - All WMO arrays are loaded
   - All M2 geometry and main arrays are loaded

4. **Screenshot Capture**: Once all frame data is loaded, captures a screenshot to the specified output file.

5. **Cleanup**: Waits for all GPU work to complete and cleans up resources.

## Notes

- The renderer will wait for all assets to load before capturing the screenshot, ensuring complete scenes.
- The output directory will be created automatically if it doesn't exist.
- The CASC path can be provided via command-line or the `CASC_PATH` environment variable.

