/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2022 - Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             AudioRecordingDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Records audio to a file.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_processors, juce_audio_utils, juce_core,
                   juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        AudioRecordingDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "DemoUtilities.h"
#include "AudioLiveScrollingDisplay.h"
#include <winsock2.h>
#include <windows.h>
#include "DanteAudioIODevice.h"

namespace juce {
    //==============================================================================
    class ChannelsListComponent: public Component,
        private ListBoxModel
    {
    public: 
        ChannelsListComponent() : mChannelNames() {};
        void setNames(std::vector<std::string> channelNames, bool isInput)
        {
            mChannelNames = channelNames;
            addAndMakeVisible(listBox);
            listBox.setTitle("Channel List");
            listBox.setModel(this);
            listBox.selectRow(0);
        }

        void resized() override
        {
            listBox.setBounds(getLocalBounds());
        }

        int getNumRows() override
        {
            return mChannelNames.size();
        }

        void paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected) override
        {
            if (rowNumber >= getNumRows())
                return;

            if (rowIsSelected)
                g.fillAll(Colour::contrasting(findColour(ListBox::textColourId),
                    findColour(ListBox::backgroundColourId)));

            g.setColour(findColour(ListBox::textColourId));
            g.setFont(14.0f);
            g.drawFittedText(getNameForRow(rowNumber), 8, 0, width - 10, height, Justification::centredLeft, 2);
        }

        String getNameForRow(int rowNumber) override
        {
            if (rowNumber < getNumRows())
                return mChannelNames[rowNumber];
            else
                return "";
             

            return {};
        }

        void selectedRowsChanged(int lastRowSelected) override
        {
            
        }

    private:
        ListBox listBox;
        std::vector<std::string> mChannelNames;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChannelsListComponent)
    };

    //==============================================================================
    /** A simple class that acts as an AudioIODeviceCallback and writes the
        incoming audio data to a WAV file.
    */
    class AudioRecorder : public AudioIODeviceCallback
    {
    public:
        AudioRecorder(AudioThumbnail& thumbnailToUpdate)
            : thumbnail(thumbnailToUpdate)
        {
            backgroundThread.startThread();
        }

        ~AudioRecorder() override
        {
            stop();
        }

        //==============================================================================
        void startRecording(const File& file)
        {
            stop();

            if (sampleRate > 0)
            {
                // Create an OutputStream to write to our destination file...
                file.deleteFile();

                if (auto fileStream = std::unique_ptr<FileOutputStream>(file.createOutputStream()))
                {
                    // Now create a WAV writer object that writes to our output stream...
                    WavAudioFormat wavFormat;

                    if (auto writer = wavFormat.createWriterFor(fileStream.get(), sampleRate, 1, 16, {}, 0))
                    {
                        fileStream.release(); // (passes responsibility for deleting the stream to the writer object that is now using it)

                        // Now we'll create one of these helper objects which will act as a FIFO buffer, and will
                        // write the data to disk on our background thread.
                        threadedWriter.reset(new AudioFormatWriter::ThreadedWriter(writer, backgroundThread, 32768));

                        // Reset our recording thumbnail
                        thumbnail.reset(writer->getNumChannels(), writer->getSampleRate());
                        nextSampleNum = 0;

                        // And now, swap over our active writer pointer so that the audio callback will start using it..
                        const ScopedLock sl(writerLock);
                        activeWriter = threadedWriter.get();
                    }
                }
            }
        }

        void stop()
        {
            // First, clear this pointer to stop the audio callback from using our writer object..
            {
                const ScopedLock sl(writerLock);
                activeWriter = nullptr;
            }

            // Now we can delete the writer object. It's done in this order because the deletion could
            // take a little time while remaining data gets flushed to disk, so it's best to avoid blocking
            // the audio callback while this happens.
            threadedWriter.reset();
        }

        bool isRecording() const
        {
            return activeWriter.load() != nullptr;
        }

        //==============================================================================
        void audioDeviceAboutToStart(AudioIODevice* device) override
        {
            sampleRate = device->getCurrentSampleRate();
        }

        void audioDeviceStopped() override
        {
            sampleRate = 0;
        }

        void audioDeviceIOCallback(const float** inputChannelData, int numInputChannels,
            float** outputChannelData, int numOutputChannels,
            int numSamples) override
        {
            const ScopedLock sl(writerLock);

            if (activeWriter.load() != nullptr && numInputChannels >= thumbnail.getNumChannels())
            {
                activeWriter.load()->write(inputChannelData, numSamples);

                // Create an AudioBuffer to wrap our incoming data, note that this does no allocations or copies, it simply references our input data
                AudioBuffer<float> buffer(const_cast<float**> (inputChannelData), thumbnail.getNumChannels(), numSamples);
                thumbnail.addBlock(nextSampleNum, buffer, 0, numSamples);
                nextSampleNum += numSamples;
            }

            // We need to clear the output buffers, in case they're full of junk..
            for (int i = 0; i < numOutputChannels; ++i)
                if (outputChannelData[i] != nullptr)
                    FloatVectorOperations::clear(outputChannelData[i], numSamples);
        }

    private:
        AudioThumbnail& thumbnail;
        TimeSliceThread backgroundThread{ "Audio Recorder Thread" }; // the thread that will write our audio data to disk
        std::unique_ptr<AudioFormatWriter::ThreadedWriter> threadedWriter; // the FIFO used to buffer the incoming data
        double sampleRate = 0.0;
        int64 nextSampleNum = 0;

        CriticalSection writerLock;
        std::atomic<AudioFormatWriter::ThreadedWriter*> activeWriter{ nullptr };
    };

    //==============================================================================
    class RecordingThumbnail : public Component,
        private ChangeListener
    {
    public:
        RecordingThumbnail()
        {
            formatManager.registerBasicFormats();
            thumbnail.addChangeListener(this);
        }

        ~RecordingThumbnail() override
        {
            thumbnail.removeChangeListener(this);
        }

        AudioThumbnail& getAudioThumbnail() { return thumbnail; }

        void setDisplayFullThumbnail(bool displayFull)
        {
            displayFullThumb = displayFull;
            repaint();
        }

        void paint(Graphics& g) override
        {
            g.fillAll(Colours::darkgrey);
            g.setColour(Colours::lightgrey);

            if (thumbnail.getTotalLength() > 0.0)
            {
                auto endTime = displayFullThumb ? thumbnail.getTotalLength()
                    : jmax(30.0, thumbnail.getTotalLength());

                auto thumbArea = getLocalBounds();
                thumbnail.drawChannels(g, thumbArea.reduced(2), 0.0, endTime, 1.0f);
            }
            else
            {
                g.setFont(14.0f);
                g.drawFittedText("(No file recorded)", getLocalBounds(), Justification::centred, 2);
            }
        }

    private:
        AudioFormatManager formatManager;
        AudioThumbnailCache thumbnailCache{ 10 };
        AudioThumbnail thumbnail{ 512, formatManager, thumbnailCache };

        bool displayFullThumb = false;

        void changeListenerCallback(ChangeBroadcaster* source) override
        {
            if (source == &thumbnail)
                repaint();
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RecordingThumbnail)
    };
   
    //==============================================================================
    class AudioRecordingDemo : public Component
    {
    public:
        AudioRecordingDemo()
        {
            setOpaque(true);

            deviceType = this->createAudioIODeviceType();
            if (audioDeviceManager.getAvailableDeviceTypes().indexOf(deviceType) == -1)
                audioDeviceManager.addAudioDeviceType(std::unique_ptr<AudioIODeviceType>(deviceType));
            auto audioDeviceSelectorComponent = new AudioDeviceSelectorComponent(audioDeviceManager,
                0, 64, 0, 64, false, false, true, false);
            audioSetupComp.reset(audioDeviceSelectorComponent);
            addAndMakeVisible(audioSetupComp.get());
            
            addAndMakeVisible(explanationLabel);
            explanationLabel.setFont(Font(15.0f, Font::plain));
            explanationLabel.setJustificationType(Justification::topLeft);
            explanationLabel.setEditable(false, false, false);
            explanationLabel.setColour(TextEditor::textColourId, Colours::white);

#ifndef JUCE_DEMO_RUNNER
            RuntimePermissions::request(RuntimePermissions::recordAudio,
                [this](bool granted)
                {
                    int numInputChannels = granted ? 2 : 0;
                    audioDeviceManager.initialise(numInputChannels, 2, nullptr, true, {}, nullptr);
                });
#endif

            setSize(410, 520);
        }

        ~AudioRecordingDemo() override
        {
        }

        void paint(Graphics& g) override
        {
            g.fillAll(getUIColourIfAvailable(LookAndFeel_V4::ColourScheme::UIColour::windowBackground));
        }

        void resized() override
        {
            auto area = getLocalBounds();

            if (audioSetupComp != nullptr) 
                audioSetupComp->setBounds(Rectangle<int>(10, 10, 400, 400));

            explanationLabel.setText("",NotificationType::dontSendNotification);
            explanationLabel.setBounds(Rectangle<int>(10, 490, 400, 20));
           
        }

    private:
        // if this PIP is running inside the demo runner, we'll use the shared device manager instead
#ifndef JUCE_DEMO_RUNNER
        AudioDeviceManager audioDeviceManager;
#else
        AudioDeviceManager& audioDeviceManager{ getSharedAudioDeviceManager(1, 0) };
#endif
        std::unique_ptr<AudioDeviceSelectorComponent> audioSetupComp;

        AudioIODeviceType* deviceType;
        Label explanationLabel{ {}, "Initializing Dante...\n"
        };

        std::unique_ptr<ComboBox> outputDeviceDropDown, inputDeviceDropDown;
        std::unique_ptr<Label> outputDeviceLabel, inputDeviceLabel;
        ChannelsListComponent outputChannelsList, inputChannelsList;
        AudioIODeviceType* createAudioIODeviceType() { return new DanteAudioIODeviceType(); }
        void setChannelsList(String device, bool isInput)
        {
           
        }
        void addNamesToDeviceBox(ComboBox& combo, bool isInputs)
        {
            const StringArray devs(deviceType->getDeviceNames(isInputs));

            combo.clear(dontSendNotification);

            for (int i = 0; i < devs.size(); ++i)
                combo.addItem(devs[i], i + 1);

            combo.setSelectedId(0, dontSendNotification);
        }
        void updateOutputsComboBox()
        {
            if (outputDeviceDropDown == nullptr)
            {
                outputDeviceDropDown.reset(new ComboBox());
                outputDeviceDropDown->onChange = [this] {setChannelsList(outputDeviceDropDown->getItemText(outputDeviceDropDown->getSelectedItemIndex()), false);  };

                addAndMakeVisible(outputDeviceDropDown.get());

                outputDeviceLabel.reset(new Label({}, TRANS("Output:")));
                outputDeviceLabel->attachToComponent(outputDeviceDropDown.get(), true);
            }

            addNamesToDeviceBox(*outputDeviceDropDown, false);
        }

        void updateInputsComboBox()
        {
            if (inputDeviceDropDown == nullptr)
            {
                inputDeviceDropDown.reset(new ComboBox());
                inputDeviceDropDown->onChange = [this] { setChannelsList(inputDeviceDropDown->getItemText(inputDeviceDropDown->getSelectedItemIndex()), true); };
                addAndMakeVisible(inputDeviceDropDown.get());

                inputDeviceLabel.reset(new Label({}, TRANS("Input:")));
                inputDeviceLabel->attachToComponent(inputDeviceDropDown.get(), true);

                //inputLevelMeter.reset(new SimpleDeviceManagerInputLevelMeter(*setup.manager));
                //addAndMakeVisible(inputLevelMeter.get());
            }

            addNamesToDeviceBox(*inputDeviceDropDown, true);
        }
        void startRecording()
        {
            if (!RuntimePermissions::isGranted(RuntimePermissions::writeExternalStorage))
            {
                SafePointer<AudioRecordingDemo> safeThis(this);

                RuntimePermissions::request(RuntimePermissions::writeExternalStorage,
                    [safeThis](bool granted) mutable
                    {
                        if (granted)
                            safeThis->startRecording();
                    });
                return;
            }

#if (JUCE_ANDROID || JUCE_IOS)
            auto parentDir = File::getSpecialLocation(File::tempDirectory);
#else
            auto parentDir = File::getSpecialLocation(File::userDocumentsDirectory);
#endif

            //lastRecording = parentDir.getNonexistentChildFile("JUCE Demo Audio Recording", ".wav");

            //recorder.startRecording(lastRecording);

            //recordButton.setButtonText("Stop");
            //recordingThumbnail.setDisplayFullThumbnail(false);
        }

        void stopRecording()
        {
            //recorder.stop();

#if JUCE_CONTENT_SHARING
            SafePointer<AudioRecordingDemo> safeThis(this);
            File fileToShare = lastRecording;

            ContentSharer::getInstance()->shareFiles(Array<URL>({ URL(fileToShare) }),
                [safeThis, fileToShare](bool success, const String& error)
                {
                    if (fileToShare.existsAsFile())
                        fileToShare.deleteFile();

                    if (!success && error.isNotEmpty())
                        NativeMessageBox::showAsync(MessageBoxOptions()
                            .withIconType(MessageBoxIconType::WarningIcon)
                            .withTitle("Sharing Error")
                            .withMessage(error),
                            nullptr);
                });
#endif

            //lastRecording = File();
            //recordButton.setButtonText("Record");
            //recordingThumbnail.setDisplayFullThumbnail(true);
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioRecordingDemo)
    };
}