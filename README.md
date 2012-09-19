aristed is a project born out of my frustration with C++ IDEs.

My ideal C++ editor:
- high degree of semanticity (highlighting, completion and diagnostics)
- keyboard navigable
- fast
- unobtrusive, minimal chrome
- you can start typing as soon as you launch the program
- git integration

I have taken some inspiration from well-known editors: Eclipse CDT, XCode, vim, Notepad++, aoeui

This project aims to stay small, adding functionality in a modular manner through the extensive use of external libraries. It should be easily hackable/extensible, features can be added as they are needed, or more realistically, when developers have time.

Planned development angles:
- Extend semanticity. [Clang](http://clang.llvm.org/) is already used but there is much more to be gained
- Basic C++ editing fixes: auto-indentation, brace completion
- Navigation keybindings, [aoeui](http://aoeui.sourceforge.net/) style modification bindings
- Git integration via [libgit2](http://libgit2.github.com/)

Current (working) screenshot:

![Screenshot](https://raw.github.com/ohwgiles/aristed/master/res/screenshot.png)

