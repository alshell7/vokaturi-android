## Vokaturi - Android Library
[![](http://img.shields.io/badge/build-passing-blue.svg)]()
[![API](https://img.shields.io/badge/API-15%2B-orange.svg?style=flat)](https://android-arsenal.com/api?level=15)
[![](https://jitpack.io/v/alshell7/VokaturiAndroid.svg)](https://jitpack.io/#alshell7/VokaturiAndroid)
[![](http://img.shields.io/badge/Vokaturi-2--1d-red.svg)](http://developers.vokaturi.com/getting-started/overview)
[![Android Arsenal](https://img.shields.io/badge/Android%20Arsenal-VokaturiAndroid-lightgrey.svg?style=flat)](https://android-arsenal.com/details/1/6036)


Android port of the Vokaturi emotion recognition API.

## Overview
[Vokaturi](https://vokaturi.com) is an emotion recognition software, that can **understand the emotions in a speaker's voice**.
Currently Vokaturi is available for **iOS, Windows, MacOS**. This project adds up to support for [Android](https://www.android.com) platform as well.
Vokaturi maintains three separate versions of its software library for recognizing emotions. The android library provided in this project is implemented using [JNI](http://docs.oracle.com/javase/7/docs/technotes/guides/jni/spec/functions.html) framework and built up on the _OpenVokaturi_ that is [distributed](https://developers.vokaturi.com/getting-started/overview) under General Public License (GPL).

Currently the community version of the Vokaturi is able to detect five different types of emotions.
* Neutrality
* Happiness
* Sadness
* Anger
* Fear

### :sparkles: Demo

To have a check on the library, download the [demo apk](https://github.com/alshell7/VokaturiAndroid/blob/master/app-debug.apk)

<p align="left"><img src="https://github.com/alshell7/VokaturiAndroid/blob/master/demo_snapshot.png"></p>

> Demo application shows the following emoji's based on the results detected from voice.

![emoji_neutral](https://github.com/alshell7/VokaturiAndroid/blob/master/app/src/main/res/drawable/emoji_neutral.png)
![emoji_happiness](https://github.com/alshell7/VokaturiAndroid/blob/master/app/src/main/res/drawable/emoji_happiness.png)
![emoji_sadness](https://github.com/alshell7/VokaturiAndroid/blob/master/app/src/main/res/drawable/emoji_sadness.png)
![emoji_anger](https://github.com/alshell7/VokaturiAndroid/blob/master/app/src/main/res/drawable/emoji_anger.png)
![emoji_fear](https://github.com/alshell7/VokaturiAndroid/blob/master/app/src/main/res/drawable/emoji_fear.png)

## :wrench: Installation 
To add emotion recognition by speech functionality in your app, Add the library in your `Project build.gradle` :

```groovy
allprojects {
	repositories {
		...
		maven { url 'https://jitpack.io' }
	}
}
```

Add the dependency in the `build.gradle (Module: app)` :

```groovy
dependencies {
	compile 'com.github.alshell7:VokaturiAndroid:{version-number}'
}
```
> Check out the latest release or from the badge above for the version number.

## :bulb: Usage

### Permissions
* To be able to record and analyze for emotions from a given audio file, your app must tell the user that it will access the device's audio input and read from external storage. You must include these permission tags in the app's manifest file :

```xml
<uses-permission android:name="android.permission.RECORD_AUDIO" />
<uses-permission android:name="android.permission.WRITE_EXTERNAL_STORAGE"/>
```

### Java
* To instantiate the library and get access to its functions, retrieve the singleton instance of the [Vokaturi](https://github.com/alshell7/VokaturiAndroid/blob/master/vokaturi/src/main/java/com/projects/alshell/vokaturi/Vokaturi.java) class :

```java
Vokaturi vokaturiApi;

try {            
      vokaturiApi = Vokaturi.getInstance(getApplicationContext());
 } catch (VokaturiException e) {
      e.printStackTrace();
}
```

* **Start recording** the voice from the device :

```java
vokaturiApi.startListeningForSpeech();
```

* **Stop recording** and analyze for emotions :

```java
EmotionProbabilities emotionProbabilities = vokaturiApi.stopListeningAndAnalyze();
```

Recording of voice from the audio input hardware of the device is automatically handled by the library, so you need not to worry about using [AudioRecord](https://developer.android.com/reference/android/media/AudioRecord.html) and [MediaRecord](https://developer.android.com/reference/android/media/MediaRecorder.html) classes.

* **Access the metrics of emotions :**

```java
logD("Neutrality: " + emotionProbabilities.Neutrality);
logD("Happiness: " + emotionProbabilities.Happiness);
logD("Sadness: " + emotionProbabilities.Sadness);
logD("Anger: " + emotionProbabilities.Anger);
logD("Fear: " + emotionProbabilities.Fear);
```

Since the method `stopListeningAndAnalyze()` returns back highly accurate metrics of emotions analyzed from the speech data captured, you can trim the length of values in [EmotionProbabilities](https://github.com/alshell7/VokaturiAndroid/blob/master/vokaturi/src/main/java/com/projects/alshell/vokaturi/EmotionProbabilities.java) by calling the method :

```java
EmotionProbabilities.scaledValues(int scale);
```

> For example, the value `0.0780483331548E-15`, after calling `scaledValues(5)`, value will change to `0.07805`

* Detect the exact dominating emotion from the metrics :mag_right: :

```java
Emotion capturedEmotion = Vokaturi.extractEmotion(EmotionProbabilities emotionProbabilities)
```
 The returned value can be any of the below encapsulated values :
 
 ```java
public enum Emotion
{
    Neutral,
    Happy,
    Sad,
    Angry,
    Feared
}
 ```
 
### Analyze asynchronously

You can use async method to analyze for emotions on a background thread. But only if you wish to handle voice recording by yourself, or want to voluntarily process any audio file to extract emotions.

```java
vokaturiApi.AnalyzeForEmotionsAsync(MainActivity.this, fileName, new VokaturiAsyncResult()
            {
                @Override
                public void onSuccess(EmotionProbabilities emotionProbabilities)
                {
                    //If there were no exceptions thrown by the native code
                }
    
                @Override
                public void onError(VokaturiException e)
                {
                   //If there was some problem that occurred while processing
                }
            });
```
* **Important Note :** Vokaturi currently supports audio file of [WAV](https://en.wikipedia.org/wiki/WAV) format only. Trying to process audio file of any other format will result in receiving an exception :ghost:.

### :speech_balloon: Error codes

The exception [class]((https://github.com/alshell7/VokaturiAndroid/blob/master/vokaturi/src/main/java/com/projects/alshell/vokaturi/VokaturiException.java)) thrown by the library is common for both the exceptions triggered from the [native](https://github.com/alshell7/VokaturiAndroid/blob/master/vokaturi/src/main/cpp/VokaturiException.h) and java implementation as well. The exception comes along with the *error codes*, so that you can take actions based on the type of the exception.

The error codes associated with the exception are as follows :

* `VOKATURI_ERROR` - Generic error.

* `VOKATURI_NOT_ENOUGH_SONORANCY` - If the speech sound that is produced is not continuous, and has turbulent audio.

* `VOKATURI_DENIED_PERMISSIONS` - File Read Write/Recording audio permissions not granted for the application.

* `VOKATURI_INVALID_WAV_FILE` - The given file is not of a valid WAV format.

### :zzz: Few Tips 

* You can use in built [recorder](https://github.com/alshell7/VokaturiAndroid/blob/master/vokaturi/src/main/java/com/projects/alshell/vokaturi/WavAudioRecorder.java) to record audio in WAV format, in case you wish to handle the voice recording by yourself.

* Need some more info?, you can [download](https://github.com/alshell7/VokaturiAndroid/blob/master/javadocs.zip) the java documentation for the library.

* Found some problems in the library?, feel easy to report it in the [issues](https://github.com/alshell7/VokaturiAndroid/issues).

## How to Contribute?

1. Fork it :trollface:
2. Create your feature branch (git checkout -b my-new-feature)
3. Commit your changes (git commit -am 'Add some feature')
4. Push to the branch (git push origin my-new-feature)
5. Create new Pull Request :hear_no_evil:


## Licenses

```
Copyright 2017 alshell7

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
```

```
 Copyright (C) 2016,2017 Paul Boersma, Johnny Ip, Toni Gojani
 version 2.0, 2017-01-02
 
 OpenVokaturi is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or (at
 your option) any later version.
 
 OpenVokaturi is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 See the GNU General Public License for more details.
```
