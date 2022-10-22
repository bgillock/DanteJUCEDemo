#include "juce_audio_devices/juce_audio_devices.h"

namespace juce {
    class JUCE_API DanteAudioIODeviceType : public AudioIODeviceType {
    public:
        DanteAudioIODeviceType();
        virtual void scanForDevices() override;
        virtual StringArray getDeviceNames(bool) const override;
        virtual int getDefaultDeviceIndex(bool) const override;
        virtual int getIndexOfDevice(AudioIODevice* d, bool) const override;
        virtual bool hasSeparateInputsAndOutputs() const override;
        virtual AudioIODevice* createDevice(const String& outputDeviceName,
            const String& inputDeviceName) override;
    };

    class ASIOAudioIODevice : public AudioIODevice
    {
    public:
        //==============================================================================
        String open(const BigInteger&, const BigInteger&, double, int) override;
        void close() override;

        void start(AudioIODeviceCallback*) override;
        void stop() override;

        Array<double> getAvailableSampleRates() override;
        Array<int> getAvailableBufferSizes() override;

        bool setAudioPreprocessingEnabled(bool) override;

        //==============================================================================
        bool isPlaying() override;
        bool isOpen() override;
        String getLastError() override;

        //==============================================================================
        StringArray getOutputChannelNames() override;
        StringArray getInputChannelNames() override;

        int getDefaultBufferSize() override;
        int getCurrentBufferSizeSamples() override;

        double getCurrentSampleRate() override;

        int getCurrentBitDepth() override;

        BigInteger getActiveOutputChannels() const override;
        BigInteger getActiveInputChannels() const override;

        int getOutputLatencyInSamples() override;
        int getInputLatencyInSamples() override;

        int getXRunCount() const noexcept override;

        //==============================================================================
        void setMidiMessageCollector(MidiMessageCollector*);
        AudioPlayHead* getAudioPlayHead() const;

        //==============================================================================
        bool isInterAppAudioConnected() const;
#if JUCE_MODULE_AVAILABLE_juce_graphics
        Image getIcon(int size);
#endif
        void switchApplication();

    private:
        //==============================================================================
        ASIOAudioIODevice(DanteAudioIODeviceType*, const String&, const String&);

        //==============================================================================
        friend class DanteAudioIODeviceType;
        friend struct AudioSessionHolder;

        struct Pimpl;
        std::unique_ptr<Pimpl> pimpl;

        JUCE_DECLARE_NON_COPYABLE(ASIOAudioIODevice)
    };
};