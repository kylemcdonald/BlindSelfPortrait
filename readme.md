# BlindSelfPortrait

This repository stores work from the interactive installation "Blind Self Portrait" by Matt Mets and Kyle McDonald. It's built with [openFrameworks](http://www.openframeworks.cc/) and makes heavy use of the following addons:

* [ofxFaceTracker](https://github.com/kylemcdonald/ofxFaceTracker)
* [ofxCv](https://github.com/kylemcdonald/ofxCv)
* [ofxPathfinder](https://github.com/kylemcdonald/ofxPathfinder/)

And relies on the [s3g library](https://github.com/makerbot/s3g/) for communicating with a headless makerbot in realtime.

## Descriptions

* BlindSelfPortrait is the app running for the installation.
* ConnectedComponents demonstrates connected components analysis (from scratch) for labeling continuous edges.
* EraserLine uses the ofxPathfinder addon to find a path through points in an image, and sends them over OSC to the platform.
* CursiveRecorder and CursivePlayback were part of an earlier experiment involving automatic writing.