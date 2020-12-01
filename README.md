**Note!**

 Since most of the updates are integrated from the upstream branch at sourceforge, the master branch is occasionally rebased.

# Double commander ![logo](https://doublecmd.sourceforge.io/site/images/logo.png "Double commander")  

**A twin panel (side by side) cross platform open source file manager.**  

Double commander on github is fork of the [original double commander project at source forge](http://doublecmd.sourceforge.net/).  
It is inspired by [Total Commander](http://www.ghisler.com/) and features some new ideas.  

The goal of this fork is to relief this project from the svn constraints by using github as a platform to achieve a better collaboration and attract developers to improve this software to be the best it can.  

Key Features
------------

* Unicode support.  
* Background file operations.  
* Multi-rename tool.  
* Tabbed interface.  
* Configurable file panel columns.  
* Internal file viewer and text editor.  
* Folder like bahviour of many archive file types.  
* Advanced search (in files).  
* Configurable button bar to start external programs or internal menu commands.  
* Total Commander plugins compatibility WCX, WDX and WLX plug-ins support.  
* File operations logging.  
* Fully configurable key mappings.  

Build
---------
**Building the project on windows.**  

1. Download [Lazarus](http://www.lazarus-ide.org/) version 1.6 and install to c:\lazarus  
2. Download [MinGW](https://sourceforge.net/projects/mingw/files/) and install to c:\MinGW.  
   in the MinGW installation manager, install the GNU-C and the GNU-C++ compilers.
3. Clone this repository using "git clone https://github.com/TheNicker/doublecmd.git" or fork the project and clone your repository.  
4. Run build.bat  


Work flow
---------
This repository has two main branches 'svn-clone' and 'master'.  
**'master'** is the standard main branch.  
**'svn-clone'** is the up to date clone of the [trunk branch in source forge](https://sourceforge.net/p/doublecmd/code/HEAD/tree/trunk/).     
Since currently changes are slim, the 'master' branch is being rebased every week on top of the svn-clone.    
Once a fair amount of developers will use this repository, a rebase will not occur, but a normal 
merge from svn-clone as depicted in [this diagram](http://thenicker.github.io/doublecmd/).
