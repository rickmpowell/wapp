# WAPP - Windows Desktop Application Framework
## What it is
An objected-oriented C++ implementation of an application framework for targetting Microsoft Windows desktop applications. Includes sample applications, including a relatively functional chess program and engine. Functionality is only added as-needed when required by sample applications, so implementations are sometimes not fully fleshed out.  
## Requirements
WAPP itself is a Windows-only static link library and header files. Released as a Visual Studio solution (.sln) that contains multiple sample applications as sub-projects, along with the core WAPP library build.
Active development happens on Microsoft Windows 11 Pro 24H2 with Microsoft Visual Studio Community 2022. We primarily use the MSVC compiler, but have also tested with clang. We've only tested 64-bit builds, but only the Chess sample application requires 64-bit.
WAPP uses DirectX for its rendering and is 64-bit only.
## To create a new project
1. Install WAPP template to the "C:\Users\[YourUserName]\Documents\Visual Studio [Version]\Templates\ProjectTemplates" directory.
2. Add a New Project to the WAPP solution. Choose the WAPP template to create a bare-bones application.
1. Add WAPP include and library directories to compiler and linker options
## Overview of Systems
### WAPP application object
The main WAPP object has limited functionality and can be used as a basis to write non-graphical applications, although I don't recommend this. It provides access to application resources. 
### DC device contexts, Drawing, and Text
The device context is the root drawing surface, and is built on top of the Direct2D RenderTarget. It maintains a simple swapchain and renders through offscreen buffers. We provide basic drawing operations for things like rectangles, ellipses, polygons, bitmaps, and simple text.  Note that some drawing objects can be device-dependent, so we have a mechanism for invalidating cached objects you might keep.
### WN windows
The main object you'll be working with are WN objects. Applications on screen are built as a tree-structured system of WNs with parent-child relationships. The WN derives from the DC, so can be directly drawn on.
### Events
User input comes into the application through keyboard and mouse events. 
### Commands
We recommend events be used to create commands. For desktop applicaitons in particular, this allows for easy implementation of undo and redo. 
### Timers
### Controls
We include a library of WN classes that implement common user interface objects. 
### Dialogs
Controls can be gathered together into dialog boxes, which are another common UI concept. 
### Raw system access
Access to the underlying system objects, like HWNDs, RenderTargets, and 