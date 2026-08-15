
<div align="center">
    
   <h1 align="center" style="font-size:2.5em; font-weight:bold; margin:1em 0;">    
   <img height="200" alt="sanaex logo" src="https://github.com/user-attachments/assets/53acf3cd-aa74-46cf-a40f-9915d5ac5415" />
   </h1>

   <p>
      <i>
         SANAex is a rework on SANA 8BIT, a simple and lightweight VST Chiptune Synthesizer that imitates the NES and Wavetable sounds.
      </i>
   </p>
   <p>
      <a href="https://github.com/Namizure/SANAex/blob/main/LICENSE"><img src="https://img.shields.io/static/v1?label=license&message=GNU&color=blue" alt="License:GNU"></a>
      &nbsp;
      <a href="https://github.com/Namizure/SANAex/releases/latest"><img src="https://img.shields.io/github/v/tag/Namizure/SANAex" alt="Latest Release"></a>
      &nbsp;
      <a href="https://github.com/Namizure/SANAex"><img src="https://img.shields.io/badge/status-BETA-red" alt="STATUS BETA"></a>
   </p>
   <img height="550" alt="image" src="https://github.com/user-attachments/assets/e2b5328e-484b-4d74-ab22-c3bbbc12ce0d" />

</div>






## Description
This is VSTi project made using JUCE 8.0.12 Library. "SANAex" is name of the Synthesizer. It has an oscillator, pitch sweep function, vibrato function, cycles function, arpeggiator function and you can write own waveform on the waveform component.

This is a windows 32bit/64bit Application. It builded as VST3 format. And you can install it in your DAW easily.

## New Features
- Fixed the way cycles worked, added 16 more nodes to it, and separated it in it's own menu.
- Added a new menu button for arpeggios, this menu lets you configure how you want your arpeggio to sound using a graph that ranges from -24 to +24.
- Removed the usage of the Color Envelope.
- In both Cycles and Arps, if you press right click you can set-up the end note or size of the current iteration.
- Pitch bend range has been increased from 1-13 to 1-48.
- Waveforms have increased from 1 to 20.
- Three new buttons added for adding more waveforms or iterating through them.
- A total rework of the UI is currently in the works.
- New font added.
- Noise is now affected by pitch bend, sweep, and arpeggios.
- Oscilloscope has been upped to 240hz.
- Instruments are now handled by using its index number instead of strings.

## Install
Drop SANAex.vst3 files in to your DAW's Plugin folder.

## How to Use
Coming soon!

## Environment
Windows 32bit/64bit
MacOS VST/AU

I tested the following DAWs.
* Studio One 2, 3, 4
* REAPER 5.94
* Cubase8.5, 9.5
* SONAR X2
* FL Studio 12, 20
* Live 10 Suite
* Ableton Live 9
* Waveform 8

## Build Instructions on Windows
1. Download Juce (http://www.juce.com/)
2. Download the VST SDK (http://www.steinberg.net/en/company/developers.html)
3. Run "The Projucer" executable included in Juce.
4. Open juce project file "xxx.jucer"
   1. Make any changes to the configure and build options.
   1. Save juce project if modified
5. Hit "Save Project and Open in Visual Studio". I use Visual Studio 2017.
6. Select the build: "Release - x64" and set platform to x64(64bit). Otherwise, "Release - Win32" and set platform to x86(32bit).
7. Build and deploy to plugin folder.

## Licence
[GPL3.0](./LICENSE)

## Author
[Masaki Mori](https://github.com/m-masaki72)

## Contributors

[COx2](https://github.com/COx2)  

[Namizure](https://github.com/Namizure)

[Original Repogitory:https://github.com/COx2/JUCE_JAPAN_DEMO](https://github.com/COx2/JUCE_JAPAN_DEMO)


## Technologies Used
* C++ for the language
* JUCE for the framework/library
* Steinberg VST SDK
* Visual Studio for the IDE
