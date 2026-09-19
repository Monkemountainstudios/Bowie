#pragma once

#include <JuceHeader.h>
#include <array>
#include <atomic>

struct BowieLoadedSample
{
    bool load(const void* data, size_t dataSize);

    juce::AudioBuffer<float> audio;
    double sampleRate = 44100.0;
    float toneNormalisationGain = 1.0f;
};

struct BowieToneAnchor
{
    BowieLoadedSample sample;
    int rootMidiNote = 60;
};

struct BowieToneModel
{
    juce::String name;
    std::array<BowieToneAnchor, 3> anchors;
    int anchorCount = 0;
};

struct BowieSampleSet
{
    static constexpr int toneCount = 10;

    bool loadAll();
    bool isReady() const;
    const BowieToneModel& toneForIndex(int index) const;
    static juce::StringArray toneNames();

    std::array<BowieToneModel, toneCount> tones;
    BowieLoadedSample bowSustain;
    BowieLoadedSample softAttack;
    BowieLoadedSample hardAttack;
};

class BowieSound final : public juce::SynthesiserSound
{
public:
    bool appliesToNote(int) override       { return true; }
    bool appliesToChannel(int) override    { return true; }
};

class DualHeadLoopPlayer
{
public:
    void start(const BowieLoadedSample* newSample,
               double newStartPosition,
               double newLoopStart,
               double newLoopEnd,
               double crossfadeSeconds,
               double outputSampleRate,
               bool useEqualPower);

    float sampleForChannel(int channel) const;
    void advance(double sourceSamplesPerOutputSample);

private:
    float interpolatedSample(int channel, double position) const;

    const BowieLoadedSample* sample = nullptr;
    double mainPosition = 0.0;
    double incomingPosition = 0.0;
    double loopStart = 0.0;
    double loopEnd = 1.0;
    int crossfadeLength = 1;
    int crossfadeProgress = 0;
    bool crossfading = false;
    bool equalPower = true;
    juce::Random random;
};

class BowieVoice final : public juce::SynthesiserVoice
{
public:
    static constexpr int bodyLayerCount = 3;

    BowieVoice(BowieSampleSet& samples,
               juce::AudioProcessorValueTreeState& parameters,
               std::atomic<float>& sharedModWheelAmount,
               const juce::AudioBuffer<float>& sharedVibratoCents);

    bool canPlaySound(juce::SynthesiserSound* sound) override;
    void startNote(int midiNoteNumber, float velocity,
                   juce::SynthesiserSound*, int pitchWheelPosition) override;
    void stopNote(float velocity, bool allowTailOff) override;
    void pitchWheelMoved(int newPitchWheelValue) override;
    void controllerMoved(int controllerNumber, int newControllerValue) override;
    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
                         int startSample, int numSamples) override;

private:
    float oneShotSample(const BowieLoadedSample& source,
                        int channel, double position) const;
    void updateEnvelopeParameters();

    BowieSampleSet& sampleSet;
    juce::AudioProcessorValueTreeState& apvts;
    std::atomic<float>& sharedModWheelAmount;
    const juce::AudioBuffer<float>& sharedVibratoCents;
    juce::ADSR envelope;
    float smoothedEnvelope = 0.0f;
    float envelopeGlideCoefficient = 1.0f;
    std::array<std::array<DualHeadLoopPlayer, 2>, bodyLayerCount> tonePlayers;
    std::array<std::array<double, 2>, bodyLayerCount> baseToneRatios
        {{{ 1.0, 1.0 }, { 1.0, 1.0 }, { 1.0, 1.0 }}};
    std::array<std::array<float, 2>, bodyLayerCount> toneGains
        {{{ 1.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 0.0f }}};
    std::array<int, bodyLayerCount> layerToneIndices { 3, 5, 9 };
    std::array<juce::SmoothedValue<float>, bodyLayerCount> layerLevels;
    DualHeadLoopPlayer bowPlayer;
    juce::Random random;

    double outputRate = 44100.0;
    double softAttackPosition = 0.0;
    double hardAttackPosition = 0.0;
    double noteAgeSeconds = 0.0;
    float noteVelocity = 0.0f;
    float softAttackMix = 1.0f;
    float hardAttackMix = 0.0f;
    float pitchWheelSemitones = 0.0f;
    float wobblePhaseA = 0.0f;
    float wobblePhaseB = 0.0f;
    double strainRateAHz = 6.0;
    double strainRateBHz = 9.2;
    double strainDecaySeconds = 0.23;
    double strainDepthVariation = 1.0;
    double strainInitialDrop = 0.32;
    float humanVibratoPhase = 0.0f;
};
