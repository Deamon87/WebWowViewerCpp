![github actions state](https://github.com/Deamon87/WebWowViewerCpp/actions/workflows/main.yml/badge.svg?branch=master)

# WebWowViewerCpp

The aim of this project is to create an opensource viewer of models and maps for World of Warcraft.

There are two frontends built using this code: standalone map viewer and webgl library built for web.

Webgl version of model viewer runs on https://wow.tools/mv/ page

[![Donate to support developer](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.com/donate/?hosted_button_id=AFUUEL3ZQVVNW)

## How to build

*The following text is intended only for developers. Usual users can download ready-to-use binaries from [releases] page*

### Prerequirements:

- Git
- CMake >3.13
- Python 3
- Any latest compiler: MSVC/Clang/GCC

### Getting sources

- git clone https://github.com/Deamon87/WebWowViewerCpp.git
- git submodule --init --recursive

### Building

Configure and build as usually you would do with CMake project. Lately I had issues with `makefiles`, so I recommend using `ninja` generator

Otherwise, you can look at the github actions script in `.github/workflows/` folder to look at how project is built there

## How to use

- Grab an app from [releases] page
- Run app
- Open your Local Casc (File -> Open Casc Storage)
- Update the database (File -> Update database).
- Open Map Selection dialog (File -> Open Map Selection)
- Select a map and adjust the zoom to find the point on minimap. Select a point and click GO

Happy exploring :)

[releases]:https://github.com/Deamon87/WebWowViewerCpp/releases/
