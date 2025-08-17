# WAPP - Windows Desktop Application Framework
## What it is
An objected-oriented C++ implementation of an application framework for targetting Microsoft Windows desktop applications. Includes sample applications, including a relatively functional chess program and engine. Functionality to WAPP is only added as-needed when required by sample applications, so implementations are sometimes not fully fleshed out.
Graphical output uses modern DirectX graphics and should take advantage of modern graphics drivers and hardware. 
## Requirements
WAPP itself is a Windows-only static link library and header files. Released as a Visual Studio solution (.sln) that contains multiple sample applications as sub-projects, along with the core WAPP library build.
Active development happens on Microsoft Windows 11 Pro 24H2 with Microsoft Visual Studio Community 2022. We primarily use the MSVC compiler, but have also tested with clang. 
WAPP uses DirectX for its rendering and is 64-bit only.
## To create a new project
(WARNING! This does not currently work)
1. Install WAPP template to the "C:\Users\[YourUserName]\Documents\Visual Studio [Version]\Templates\ProjectTemplates" directory.
2. Add a New Project to the WAPP solution. Choose the WAPP template to create a bare-bones application.
3. Add WAPP include and library directories to compiler and linker options

## Documentation
Complete API and data structure documentation for WAPP is available at https://rickmpowell.github.io/wapp/
## Functionality
1. Window manager built on top of high speed DirectX graphics
2. Dialog manager with layout engine
3. Numerous standard controls
4. Command dispatch system with support for Undo/Redo
5. Numerous convenience wrapper classes 
6. Full UTF-8 string support
7. User input event dispatch system
