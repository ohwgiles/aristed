aristed is a project born out of my frustration with C++ IDEs.

My ideal C++ editor:
- high degree of semanticity (highlighting, completion and diagnostics)
- optionally completely keyboard navigable
- fast
- unobtrusive, minimal chrome
- you can start typing as soon as you launch the program
- git integration

Although this project is much more modest than the following, I am inspired by certain features of Eclipse CDT, XCode, vim, Notepad++, TextWrangler, aoeui

This project aims to stay small, adding functionality in a modular manner through the extensive use of external libraries. It should be easily hackable/extensible, features can be added as they are needed, or more realistically, when I have time.

Planned development angles:
- Extend semanticity. [Clang](http://clang.llvm.org/) is already used but there is much more to be gained
- Navigation keybindings, [aoeui](http://aoeui.sourceforge.net/) style modification bindings
- Git integration via [libgit2](http://libgit2.github.com/)

Current screenshot:

![Screenshot](https://raw.github.com/ohwgiles/aristed/master/res/screenshot.png)

