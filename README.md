# DanteJUCEDemo

Requirements:
- Windows 10 x64 or greater
- VS 2022
- DAL SDK Distribution (tested with DanteApplicationLibrary_cp-cpp11-1.2.0.6_windows.zip)
- VST3 plugin (tested with SPAN https://www.voxengo.com/product/span/)
- Dante Controller
- Dante Device Transmitter

Steps:
1. Clone this repo
2. Copy include\audinate folder from DAL distribution to AudioRecordingDemo\Audinate\audinate
- Should end up with the Types.hpp file on this path ..\DanteJuceDemo\AudioRecordingDemo\Audinate\audinate\dal\Types.hpp
- Edit the Types.hpp file and comment out the #include <winsock2.h> line as it is already included in JUCE
3. Copy your access_token.c file to AudioRecordingDemo\Audinate\access_token.c
4. Open HostPluginDemo/HostPluginDemo/Builds/VisualStudio2022/DanteJUCEDemo.sln in VS20225. Build solution -> *Will report missing dal.lib*
5. Copy all .lib files from lib\vs2015-dll\Debug\x64 in your DAL distrib to HostPluginDemo\HostPluginDemo\Builds\VisualStudio2022\x64\Debug\Shared Code
6. Copy dal.dll from bin/x64 in your DAL distib to HostPluginDemo\HostPluginDemo\Builds\VisualStudio2022\x64\Debug\Standalone Plugin
7. In AudioRecordingDemo\Source\DanteAudioIODevice.cpp line 146 and 147, change to point to your DAL distrib bin and logs 
8. Build DanteJUCEDemo project
9. Run DanteJUCEDemo.exe in HostPluginDemo\HostPluginDemo\Builds\VisualStudio2022\x64\Debug\Standalone Plugin or press F5 in VS2022
10. Allow programs to execute if asked.
11. Click on Options in top left, then Audio/Midi Settings
12. Uncheck Mute audio input
13. Under Audio Device Type select "Dante"
14. Under Device, Select "DanteJUCEDemo - Stereo"
15. Drag and drop a .vst3 plugin (eg. SPAN by Voxengo) into the list/table and follow instructions at bottom of App.

In the Dante Controller, route 2 channels from any Transmitter to the DanteJUCEDemo->Left/Right receivers.



