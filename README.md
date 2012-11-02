aristed
=======

introduction
------------

aristed is a project born out of my frustration with C++ IDEs.

My ideal C++ editor:
- high degree of semanticity (highlighting, completion and diagnostics)
- optionally completely keyboard navigable
- fast and lightweight
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


dependencies
------------
- Qt >= 4.7
-- Known as libqt4-dev in debian-based distros
- clang >= 3.1
-- Known as libclang-dev in debian-based distros
-- Ubuntu 12.04 LTS only has version 3.0


compilation
-----------
    git clone https://github.com/ohwgiles/aristed.git aristed
    mkdir aristed/build
    cd aristed/build
    cmake ../
    make

design
======

these are ideas, some have been implemented, the rest are todos

navigation
----------
all navigation bindings can also use shift, to extend the current selection
- alt-jkhl for movement
- alt-b back a word
- alt-f forward a word
- ctl-a start of line
- ctl-e end of line
- ctl-f <char> will advance beyond <char> if it appears in that line
- ctl-t <char> will advance until <char> if it appears in that line
- f and t will go backwards with shift

file navigation
---------------
- ctrl-r will invoke Eclipse-style reverse-incremental filename search dialog
- f9 will show/hide file system browser

editing
-------
- ctrl-d deletes selection
- ctrl-w deletes last word
- ctrl-u delete until start of line
- ctrl-k delete until end of line

search panel
------------
- ctrl-/ shows and focuses search panel
- all matches are highlighted as you type
- highlights will only be visible while panel is visible
- enter will move the cursor to the first match after the cursor and focus the editor
- esc will close the panel without moving the cursor (and disable the highlights)
- alt-n will move to next result
- alt-p will move to previous result
- as with the navigation bindings, adding shift will select from cursor position until next match
- searches are single-line regex, case insensitive by default
- alt-i will toggle case sensitivity
- alt-r will toggle regex search

replace panel
-------------
the same as search panel, with the following changes
- ctrl-shift-/ shows and focuses panel
- enter will perform the replacement and move to the next match
- ctrl-enter will replace all in document

git
---
- line indicator is red when modified + unsaved
- line indicator is yellow when saved but unstaged
- line indicator is blue when staged but uncommitted
- ctrl-g will stage selected (or current) lines
- ctrl-shift-g will unstage selected (or current) lines
- f2 will show git status and allow committing
