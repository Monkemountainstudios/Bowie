#pragma once

#include <JuceHeader.h>
#include "BowieVoice.h"

class BowieAudioProcessor final : public juce::AudioProcessor
{
public:
    BowieAudioProcessor();
    ~BowieAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 5.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    static juce::StringArray factoryPresetNames();
    void applyFactoryPreset(int presetIndex);

    juce::AudioProcessorValueTreeState parameters;
    juce::MidiKeyboardState keyboardState;
    bool samplesAreReady() const { return samples.isReady(); }
    float currentModWheelAmount() const { return sharedModWheelAmount.load(); }

private:
    BowieSampleSet samples;
    juce::Synthesiser synthesiser;
    std::atomic<float> sharedModWheelAmount { 0.0f };
    juce::AudioBuffer<float> sharedVibratoCents;
    double sharedVibratoPhase = 0.0;
    double currentSampleRate = 44100.0;
    juce::dsp::StateVariableTPTFilter<float> outputFilter;
    juce::Reverb roomReverb;
    juce::SmoothedValue<float> masterGain;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BowieAudioProcessor)
};
