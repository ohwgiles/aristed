aristed
=======

introduction
------------

aristed is a project born out of my frustration with C++ IDEs.

my ideal C++ editor
- has a high degree of semanticity (highlighting, completion and diagnostics)
- is completely keyboard navigable
- is fast and lightweight
- allows writing code immediately on launching, without setting up a project
- has git integration

although this project is much more modest than the following, I am inspired by certain features of Eclipse CDT, XCode, vim, Notepad++, TextWrangler and aoeui

this project aims to stay small, adding functionality in a modular manner through the extensive use of external libraries. it should be easily hackable/extensible, features can be added as they are needed. run-time configurability is a low priority but code should be structured in a way to make this simple

planned development angles:
- extend semanticity. [Clang](http://clang.llvm.org/) is already used but there is much more to be gained
- navigation keybindings, [aoeui](http://aoeui.sourceforge.net/) style modification bindings
- git integration via [libgit2](http://libgit2.github.com/)

screenshot:

![Screenshot](https://raw.github.com/ohwgiles/aristed/master/res/screenshot.png)


dependencies
------------
- Qt &gt;= 5
  - Arch Linux (qt5-base)
  - [search packages.ubuntu.com](http://packages.ubuntu.com/search?keywords=qtbase5-dev&searchon=names)
- clang &gt;= 3.1
  - Arch Linux (clang)
  - [search packages.ubuntu.com](http://packages.ubuntu.com/search?keywords=libclang-dev&searchon=names)

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
- alt-jkhl for movement &#10004;
- alt-b back a word &#10004;
- alt-w forward a word &#10004;
- ctl-a start of line &#10004;
- ctl-e end of line &#10004;
- ctl-t &lt;char&gt; will advance until &lt;char&gt; if it appears in that line &#10004;

file navigation
---------------
- ctrl-r will invoke Eclipse-style reverse-incremental filename search dialog
- f9 will show/hide file system browser &#10004;

editing
-------
- ctrl-d deletes selection
- ctrl-w deletes last word
- ctrl-u delete until start of line
- ctrl-k delete until end of line

search panel
------------
- ctrl-/ shows and focuses search panel &#10004;
- all matches are highlighted as you type &#10004;
- highlights will only be visible while panel is visible &#10004;
- enter will move the cursor to the first match after the cursor and focus the editor &#10004;
- esc will close the panel without moving the cursor (and disable the highlights) &#10004;
- alt-n will move to next result &#10004;
- alt-p will move to previous result &#10004;
- as with the navigation bindings, adding shift will select from cursor position until next match &#10004;
- searches are single-line regex, case insensitive by default &#10004;
- alt-i will toggle case sensitivity
- <del>alt-r will toggle regex search</del>

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
