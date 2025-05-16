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
## Documentation
Documentation of the public API is a Microsoft Windows document "WAPP Windows Application Framework.docx" in the doc directory of the project.