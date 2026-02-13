# DrawingSoftware
A standalone software made in C++ with Qt

Setup
● Ensure Visual Studio (2022) is installed on the system (Development for C++
enabled)
● Install Qt for Developers from their official website
(https://www.qt.io/development/developers)
● Download or pull the files from GitHub
(https://github.com/doccdestroyer/StandaloneDrawingSoftware)
● Open Visual Studio and navigate to the extensions tab, then install Qt Visual Studio
Tools. Restart Visual Studio when prompted.
● Once restarted, navigate to Extensions -> Qt VS Tools -> Options -> Versions and
press “Add”. Then select where you have installed Qt and select the qmake.exe
(example: Qt -> 6.4.0 -> msvc2019_64 -> :bin -> qmake.exe | NOTE: Version
numbers and Dates are subject to change). Restart Visual Studio when prompted.
● Open DrawingSoftware.sln located within the DrawingSoftware Folder
● Once opened, if there is a prompt to retarget the Platform Toolset, select it. If there
are issues and the prompt has not appeared: go to Project -> Properties -> General
-> Platform Toolset, and ensure that v145 or v143 is selected (dependent on which
Visual Studio version you have)
● Run the file with the local windows debugger and the software should start.
