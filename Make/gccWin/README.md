# MSYS2 and MinGW setup
**MSYS2** is software distribution and a building platform for Windows. It provides a Unix-like environment, a command-line interface and a software repository making it easier to install, use, build and port software on Windows.  See https://github.com/msys2/msys2/wiki/MSYS2-introduction

**MinGW** provides a complete Open Source programming tool set which is suitable for the development of native MS-Windows applications, and which do not depend on any 3rd-party C-Runtime DLLs.  See http://www.mingw.org/

You are encouraged to do additional study on the MSYS2 and MinGW applications, and on Linux commands in general if you are new to the subject.  There are many wiki, FAQ and supporting discussion forums on all them.  Search on 'setup MSYS2 MinGW

## Installation of the MSYS2 and MinGW environments
Download the MSYS2 installation setup files from https://www.msys2.org/ (about 75MB)
We'll use `http://repo.msys2.org/distrib/x86_64/msys2-x86_64-20180531.exe` for this discussion.

- Run the setup and set your installation folder.  We'll use `C:\msys64`  
- Set the Windows Start Menu name, we'll use `MSYS2 64bit`  
- Finish with the box checked `Run MSYS2 64-bit now`

The MSYS2 terminal window opens to the `~` folder, this is the traditional Linux user home folder.  Your corresponding Windows folder for `~` is `C:\msys64\home\<user name>`

#### Update MSYS2
First update the new MSYS2 the package database and core system packages by running this command at the command prompt:  

`pacman -Syu`   *(remember Linux commands are cAsE SeNsiTiVe)*

In case you are curious, pacman ("package manager") help shows this for the commands:  

    usage:  pacman {-S --sync} [options] [package(s)]
    options:
      -u, --sysupgrade     upgrade installed packages (-uu enables downgrades)
      -y, --refresh        download fresh package databases from the server
          --needed         do not reinstall up to date packages

You will probably see this message:  

    warning: terminate MSYS2 without returning to shell and check for updates again  
    warning: for example close your terminal window instead of calling exit

Just close the window by clicking the red X or with Alt+F4, the typical Windows close window command.
A dialog stating `Processes are running in session, Close anyway?`` Click `OK`

Go to the `Windows Start Menu > All Programs > MSYS2 64bit > MSYS2 MinGW 64-bit`

At the command prompt run `pacman -Syu`  again.  (Hint: press the up arrow to retrieve the last command.)  This starts an update to the individual components and will take several minutes to complete.

Run `pacman -Syu`  again until it says there is nothing to do.

#### Install a compiler toolchain
a) for compiling 64-bit applications:  
`pacman -S --needed base-devel mingw-w64-x86_64-toolchain`

b) for compiling 32-bit applications:  
`pacman -S --needed base-devel mingw-w64-i686-toolchain`

This starts the retrieval and installation of the base development components (probably more than 100 small individual components) and will take several minutes to complete.

#### Set up $PATH
Update the path via 'Environment Variables' from the `Windows Control Panel > System > Advanced System Settings > Environment Variables > (User variables) Path > Edit`
At the end, add  

    ;C:\msys64\usr\bin;C:\msys64\mingw64\bin

#### Set MSYS2 to Run as administrator
Because openSeaChest needs to access the system hardware the Windows operating system will need your special permission to allow that to happen.  From the Windows Start menu open the MSYS2 64bit folder, right click on the environment you want, like MSYS2 MinGW 64-bit,  Select `Properties > Shortcut tab > Advanced > and check Run as administrator > OK > Apply > OK`

## openSeaChest downloads from GitHub
[GitHub](https://github.com/) is a Git repository hosting service, but it adds many of its own features. While Git is a command line tool, GitHub provides a Web-based graphical interface. Seagate maintains a [projects repository](https://github.com/seagate) on GitHub.

Make a project folder off of the home folder, a name something like `mygitstuff`, it is your choice.

     https://github.com/Seagate/openSeaChest
     sub-modules:
     https://github.com/Seagate/opensea-common
     https://github.com/Seagate/opensea-operations
     https://github.com/Seagate/opensea-transport

Either as ZIP downloads of the individual projects or git clones to the following directory structure (git will handle the folders for your automatically).

`cd mygitstuff`

        openSeaChest
          |_ Make
          |  |_ gccWin
          |_ opensea-common
          |  |_ Make
          |    |_ gccWin
          |_ opensea-operations
          |  |_ Make
          |    |_ gccWin
          |_ opensea-transport
             |_ Make
               |_ gccWin

## Git instructions
**Git** is a distributed version-control system for tracking changes in source code during software development. The openSeaChest projects are maintained on GitHb which follows traditional Git architecture. Git command line tools are available for MSYS2 which can greatly simplify the download process of openSeaChest, and open source projects in general.

Install git on MSYS2 with `pacman -S --needed git`

### Use git to pull the files from the Seagate openSeaChest repository on GitHub
Start by listing all of the available openSeaChest branches:  

    git ls-remote --heads https://github.com/Seagate/openSeaChest.git | sed 's?.*refs/heads/??'

A short list of branches similar to this is returned:  
- develop  *(<<-- our default branch)*
- feature/VMWare-Support
- feature/minGW_Support
- master

#### Clone the openSeaChest projects
**Examples:**
1. Clone the develop branch:  

       git clone --recurse-submodules --branch develop https://github.com/Seagate/openSeaChest.git openSeaChest-develop

2. Clone a feature branch:  

       git clone --recurse-submodules --branch feature/minGW_Support https://github.com/Seagate/openSeaChest.git openSeaChest-MinGW

3. Make sure the recursive sub-module projects (opensea-common etc) are completely cloned from the specific branch.  `cd` into the new branch folder (openSeaChest-develop or openSeaChest-MinGW from the examples above)
Run:  

       git submodule foreach --recursive 'git checkout develop'  
       or git submodule foreach --recursive 'git checkout feature/minGW_Support'

Later, if you want to pull the newest updates, then change directory to your top branch folder where you can **inspect and backup your current changes** (if you made any) with:  

    git status
    git submodule foreach 'git status'

    git stash
    git submodule foreach 'git stash'

Then get the latest openSeaChest files  

       git pull
       git pull --recurse-submodules

**Note about single-branch option**

If the `--single-branch` option is used during the initial clone, fetching submodules to the head of a specific branch as in step 3 (above) may fail until this has been "undone".
To undo a `--single-branch` pull for the submodules, perform the following:

    git submodule foreach 'git config remote.origin.fetch refs/heads/*:refs/remotes/origin/*'
    git submodule foreach 'git fetch'

At this point, `git branch -a` should show all available branches that can be pulled for a given repository, or in this case submodule. From here, you can now recursively pull the branch of interest like this:

    git submodule foreach --recursive 'git checkout branch-of-interest'


## Building openSeaChest with MinGW
Assuming you are in the `~` home directory, `cd` to `./openSeaChest-develop/Make/gccWin`
or `./openSeaChest-MinGW/Make/gccWin` whichever you used.  Run:

    make -f Makefile.gccWin
This should build all of the openSeaChest tools as defined by the variable *BUILD_ALL*.  To quickly see which tools are defined, you can run:  

    make -f Makefile.gccWin print-BUILD_ALL
You will find the newly compiled executable files in the `./openseachest_exes/ folder`.
