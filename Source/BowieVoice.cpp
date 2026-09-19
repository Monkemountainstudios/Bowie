#include "BowieVoice.h"
#include <BinaryData.h>

namespace
{
float parameterValue(juce::AudioProcessorValueTreeState& state, const char* id)
{
    return state.getRawParameterValue(id)->load();
}

bool loadAnchor(BowieToneModel& model, int anchorIndex, int rootMidiNote,
                const void* data, size_t dataSize)
{
    model.anchors[static_cast<size_t>(anchorIndex)].rootMidiNote = rootMidiNote;
    return model.anchors[static_cast<size_t>(anchorIndex)].sample.load(data, dataSize);
}
}

bool BowieLoadedSample::load(const void* data, size_t dataSize)
{
    juce::AudioFormatManager formats;
    formats.registerBasicFormats();

    auto stream = std::make_unique<juce::MemoryInputStream>(data, dataSize, false);
    std::unique_ptr<juce::AudioFormatReader> reader(formats.createReaderFor(std::move(stream)));
    if (reader == nullptr)
        return false;

    sampleRate = reader->sampleRate;
    audio.setSize(static_cast<int>(reader->numChannels),
                  static_cast<int>(reader->lengthInSamples));
    if (!reader->read(&audio, 0, audio.getNumSamples(), 0, true, true))
        return false;

    float rms = 0.0f;
    for (int channel = 0; channel < audio.getNumChannels(); ++channel)
        rms += audio.getRMSLevel(channel, 0, audio.getNumSamples());
    rms /= static_cast<float>(juce::jmax(1, audio.getNumChannels()));
    toneNormalisationGain = juce::jlimit(0.55f, 2.5f, 0.11f / juce::jmax(0.0001f, rms));
    return true;
}

bool BowieSampleSet::loadAll()
{
    tones[0].name = "Bassoon";
    tones[0].anchorCount = 3;
    tones[1].name = "Church";
    tones[1].anchorCount = 3;
    tones[2].name = "English Horn";
    tones[2].anchorCount = 2;
    tones[3].name = "Flute";
    tones[3].anchorCount = 3;
    tones[4].name = "Horn";
    tones[4].anchorCount = 3;
    tones[5].name = "Piano";
    tones[5].anchorCount = 3;
    tones[6].name = "Rhodes";
    tones[6].anchorCount = 3;
    tones[7].name = "Trumpet";
    tones[7].anchorCount = 3;
    tones[8].name = "Tuba";
    tones[8].anchorCount = 3;
    tones[9].name = "Vibra";
    tones[9].anchorCount = 3;

    const bool tonesLoaded =
        loadAnchor(tones[0], 0, 36, BinaryData::Bassoon_C2_wav, BinaryData::Bassoon_C2_wavSize)
        && loadAnchor(tones[0], 1, 48, BinaryData::Bassoon_C3_wav, BinaryData::Bassoon_C3_wavSize)
        && loadAnchor(tones[0], 2, 60, BinaryData::Bassoon_C4_wav, BinaryData::Bassoon_C4_wavSize)
        && loadAnchor(tones[1], 0, 36, BinaryData::Church_C2_wav, BinaryData::Church_C2_wavSize)
        && loadAnchor(tones[1], 1, 48, BinaryData::Church_C3_wav, BinaryData::Church_C3_wavSize)
        && loadAnchor(tones[1], 2, 60, BinaryData::Church_C4_wav, BinaryData::Church_C4_wavSize)
        && loadAnchor(tones[2], 0, 48, BinaryData::English_C3_wav, BinaryData::English_C3_wavSize)
        && loadAnchor(tones[2], 1, 60, BinaryData::English_C4_wav, BinaryData::English_C4_wavSize)
        && loadAnchor(tones[3], 0, 48, BinaryData::Flute_C3_wav, BinaryData::Flute_C3_wavSize)
        && loadAnchor(tones[3], 1, 60, BinaryData::Flute_C4_wav, BinaryData::Flute_C4_wavSize)
        && loadAnchor(tones[3], 2, 72, BinaryData::Flute_C5_wav, BinaryData::Flute_C5_wavSize)
        && loadAnchor(tones[4], 0, 36, BinaryData::Horn_C2_wav, BinaryData::Horn_C2_wavSize)
        && loadAnchor(tones[4], 1, 48, BinaryData::Horn_C3_wav, BinaryData::Horn_C3_wavSize)
        && loadAnchor(tones[4], 2, 60, BinaryData::Horn_C4_wav, BinaryData::Horn_C4_wavSize)
        && loadAnchor(tones[5], 0, 36, BinaryData::Piano_C2_wav, BinaryData::Piano_C2_wavSize)
        && loadAnchor(tones[5], 1, 48, BinaryData::Piano_C3_wav, BinaryData::Piano_C3_wavSize)
        && loadAnchor(tones[5], 2, 60, BinaryData::PIano_C4_wav, BinaryData::PIano_C4_wavSize)
        && loadAnchor(tones[6], 0, 36, BinaryData::Rhodes_C2_wav, BinaryData::Rhodes_C2_wavSize)
        && loadAnchor(tones[6], 1, 48, BinaryData::Rhodes_C3_wav, BinaryData::Rhodes_C3_wavSize)
        && loadAnchor(tones[6], 2, 60, BinaryData::Rhodes_C4_wav, BinaryData::Rhodes_C4_wavSize)
        && loadAnchor(tones[7], 0, 36, BinaryData::Trumpet_C2_wav, BinaryData::Trumpet_C2_wavSize)
        && loadAnchor(tones[7], 1, 48, BinaryData::Trumpet_C3_wav, BinaryData::Trumpet_C3_wavSize)
        && loadAnchor(tones[7], 2, 60, BinaryData::Trumpet_C4_wav, BinaryData::Trumpet_C4_wavSize)
        && loadAnchor(tones[8], 0, 37, BinaryData::Tuba_C2_wav, BinaryData::Tuba_C2_wavSize)
        && loadAnchor(tones[8], 1, 49, BinaryData::Tuba_C3_wav, BinaryData::Tuba_C3_wavSize)
        && loadAnchor(tones[8], 2, 61, BinaryData::Tuba_C4_wav, BinaryData::Tuba_C4_wavSize)
        && loadAnchor(tones[9], 0, 36, BinaryData::Vibra_C2_wav, BinaryData::Vibra_C2_wavSize)
        && loadAnchor(tones[9], 1, 48, BinaryData::Vibra_C3_wav, BinaryData::Vibra_C3_wavSize)
        && loadAnchor(tones[9], 2, 60, BinaryData::Vibra_C4_wav, BinaryData::Vibra_C4_wavSize);

    return tonesLoaded
        && bowSustain.load(BinaryData::scraping_bow_hp90_wav, BinaryData::scraping_bow_hp90_wavSize)
        && softAttack.load(BinaryData::long_soft_bow_hp250_wav, BinaryData::long_soft_bow_hp250_wavSize)
        && hardAttack.load(BinaryData::short_bow_noise_wav, BinaryData::short_bow_noise_wavSize);
}

bool BowieSampleSet::isReady() const
{
    for (const auto& tone : tones)
        for (int anchor = 0; anchor < tone.anchorCount; ++anchor)
            if (tone.anchors[static_cast<size_t>(anchor)].sample.audio.getNumSamples() == 0)
                return false;

    return bowSustain.audio.getNumSamples() > 0
        && softAttack.audio.getNumSamples() > 0
        && hardAttack.audio.getNumSamples() > 0;
}

const BowieToneModel& BowieSampleSet::toneForIndex(int index) const
{
    return tones[static_cast<size_t>(juce::jlimit(0, toneCount - 1, index))];
}

juce::StringArray BowieSampleSet::toneNames()
{
    return { "Bassoon", "Church", "English Horn", "Flute", "Horn",
             "Piano", "Rhodes", "Trumpet", "Tuba", "Vibra" };
}

void DualHeadLoopPlayer::start(const BowieLoadedSample* newSample,
                               double newStartPosition,
                               double newLoopStart,
                               double newLoopEnd,
                               double crossfadeSeconds,
                               double outputSampleRate,
                               bool useEqualPower)
{
    sample = newSample;
    mainPosition = newStartPosition;
    loopStart = juce::jmax(0.0, newLoopStart);
    loopEnd = juce::jmin(newLoopEnd,
                         static_cast<double>(sample->audio.getNumSamples() - 2));
    loopEnd = juce::jmax(loopStart + 2.0, loopEnd);
    incomingPosition = loopStart;
    crossfadeLength = juce::jmax(1, juce::roundToInt(crossfadeSeconds * outputSampleRate));
    crossfadeProgress = 0;
    crossfading = false;
    equalPower = useEqualPower;
}

float DualHeadLoopPlayer::interpolatedSample(int channel, double position) const
{
    if (sample == nullptr || sample->audio.getNumSamples() < 2)
        return 0.0f;

    const int sourceChannel = juce::jmin(channel, sample->audio.getNumChannels() - 1);
    const int index = juce::jlimit(0, sample->audio.getNumSamples() - 2,
                                   static_cast<int>(position));
    const float fraction = static_cast<float>(position - static_cast<double>(index));
    const float a = sample->audio.getSample(sourceChannel, index);
    const float b = sample->audio.getSample(sourceChannel, index + 1);
    return a + fraction * (b - a);
}

float DualHeadLoopPlayer::sampleForChannel(int channel) const
{
    const float outgoing = interpolatedSample(channel, mainPosition);
    if (!crossfading)
        return outgoing;

    const float progress = juce::jlimit(0.0f, 1.0f,
        static_cast<float>(crossfadeProgress) / static_cast<float>(crossfadeLength));
    const float incoming = interpolatedSample(channel, incomingPosition);

    if (equalPower)
    {
        const float outGain = std::cos(progress * juce::MathConstants<float>::halfPi);
        const float inGain = std::sin(progress * juce::MathConstants<float>::halfPi);
        return outgoing * outGain + incoming * inGain;
    }

    return outgoing * (1.0f - progress) + incoming * progress;
}

void DualHeadLoopPlayer::advance(double step)
{
    if (sample == nullptr)
        return;

    mainPosition += step;

    if (crossfading)
    {
        incomingPosition += step;
        ++crossfadeProgress;

        if (crossfadeProgress >= crossfadeLength)
        {
            mainPosition = incomingPosition;
            crossfading = false;
            crossfadeProgress = 0;
        }
        return;
    }

    const double sourceFadeLength = step * static_cast<double>(crossfadeLength);
    if (mainPosition >= loopEnd - sourceFadeLength)
    {
        crossfading = true;
        crossfadeProgress = 0;
        const double latestRestart = juce::jmax(loopStart,
            loopEnd - sourceFadeLength - 2.0);
        incomingPosition = loopStart
            + random.nextDouble() * (latestRestart - loopStart);
    }
}

BowieVoice::BowieVoice(BowieSampleSet& samples,
                       juce::AudioProcessorValueTreeState& parameters,
                       std::atomic<float>& modWheel,
                       const juce::AudioBuffer<float>& vibratoCents)
    : sampleSet(samples), apvts(parameters), sharedModWheelAmount(modWheel),
      sharedVibratoCents(vibratoCents)
{
}

bool BowieVoice::canPlaySound(juce::SynthesiserSound* sound)
{
    return dynamic_cast<BowieSound*>(sound) != nullptr;
}

void BowieVoice::updateEnvelopeParameters()
{
    juce::ADSR::Parameters settings;
    settings.attack = parameterValue(apvts, "attack");
    settings.decay = parameterValue(apvts, "decay");
    settings.sustain = parameterValue(apvts, "sustain");
    settings.release = parameterValue(apvts, "release");
    envelope.setParameters(settings);
}

void BowieVoice::startNote(int midiNoteNumber, float velocity,
                           juce::SynthesiserSound*, int pitchWheelPosition)
{
    outputRate = getSampleRate();
    envelope.setSampleRate(outputRate);
    constexpr double envelopeGlideSeconds = 0.012;
    envelopeGlideCoefficient = static_cast<float>(1.0
        - std::exp(-1.0 / (outputRate * envelopeGlideSeconds)));
    smoothedEnvelope = 0.0f;
    updateEnvelopeParameters();
    envelope.reset();
    envelope.noteOn();

    noteVelocity = juce::jlimit(0.0f, 1.0f, velocity);
    noteAgeSeconds = 0.0;
    softAttackPosition = 0.0;
    hardAttackPosition = 0.0;
    wobblePhaseA = random.nextFloat() * juce::MathConstants<float>::twoPi;
    wobblePhaseB = random.nextFloat() * juce::MathConstants<float>::twoPi;
    strainRateAHz = 5.35 + 1.25 * random.nextDouble();
    strainRateBHz = 8.25 + 1.75 * random.nextDouble();
    strainDecaySeconds = 0.20 + 0.10 * random.nextDouble();
    strainDepthVariation = 0.86 + 0.28 * random.nextDouble();
    strainInitialDrop = 0.24 + 0.16 * random.nextDouble();
    humanVibratoPhase = random.nextFloat() * juce::MathConstants<float>::twoPi;
    pitchWheelMoved(pitchWheelPosition);

    // Equal-power handover centred at roughly two thirds of the velocity range.
    const float crossover = juce::jlimit(0.0f, 1.0f,
        (noteVelocity - 0.52f) / (0.82f - 0.52f));
    const float velocityHardMix = std::sin(crossover * juce::MathConstants<float>::halfPi);
    hardAttackMix = velocityHardMix;
    softAttackMix = std::sqrt(juce::jmax(0.0f, 1.0f - hardAttackMix * hardAttackMix));

    const float attackSeconds = parameterValue(apvts, "attack");
    constexpr const char* toneParameterIds[] = { "tone", "tone2", "tone3" };
    constexpr const char* levelParameterIds[] = {
        "voice1Level", "voice2Level", "voice3Level"
    };

    for (int layer = 0; layer < bodyLayerCount; ++layer)
    {
        const int toneIndex = juce::roundToInt(parameterValue(apvts, toneParameterIds[layer]));
        layerToneIndices[static_cast<size_t>(layer)] = toneIndex;
        layerLevels[static_cast<size_t>(layer)].reset(outputRate, 0.020);
        layerLevels[static_cast<size_t>(layer)].setCurrentAndTargetValue(
            parameterValue(apvts, levelParameterIds[layer]));

        const auto& tone = sampleSet.toneForIndex(toneIndex);
        int lowerAnchor = 0;
        int upperAnchor = 0;
        float anchorBlend = 0.0f;
        if (midiNoteNumber >= tone.anchors[static_cast<size_t>(tone.anchorCount - 1)].rootMidiNote)
        {
            lowerAnchor = upperAnchor = tone.anchorCount - 1;
        }
        else if (midiNoteNumber > tone.anchors[0].rootMidiNote)
        {
            for (int anchorIndex = 0; anchorIndex < tone.anchorCount - 1; ++anchorIndex)
            {
                const int lowerRoot = tone.anchors[static_cast<size_t>(anchorIndex)].rootMidiNote;
                const int upperRoot = tone.anchors[static_cast<size_t>(anchorIndex + 1)].rootMidiNote;
                if (midiNoteNumber <= upperRoot)
                {
                    lowerAnchor = anchorIndex;
                    upperAnchor = anchorIndex + 1;
                    anchorBlend = static_cast<float>(midiNoteNumber - lowerRoot)
                                / static_cast<float>(upperRoot - lowerRoot);
                    break;
                }
            }
        }

        auto& gains = toneGains[static_cast<size_t>(layer)];
        gains[0] = std::cos(anchorBlend * juce::MathConstants<float>::halfPi);
        gains[1] = lowerAnchor == upperAnchor
            ? 0.0f : std::sin(anchorBlend * juce::MathConstants<float>::halfPi);

        const int selectedAnchors[] = { lowerAnchor, upperAnchor };
        for (int player = 0; player < 2; ++player)
        {
            const auto& anchor = tone.anchors[static_cast<size_t>(selectedAnchors[player])];
            const auto& sample = anchor.sample;
            baseToneRatios[static_cast<size_t>(layer)][static_cast<size_t>(player)]
                = (sample.sampleRate / outputRate)
                * std::pow(2.0, static_cast<double>(midiNoteNumber - anchor.rootMidiNote) / 12.0);

            const double length = static_cast<double>(sample.audio.getNumSamples());
            double loopStart = 0.0;
            double loopEnd = 0.0;
            double loopCrossfadeSeconds = 0.22;
            const bool isPianoFamily = toneIndex == 5 || toneIndex == 6;
            if (isPianoFamily)
            {
                loopEnd = length - sample.sampleRate * (0.18 + 0.12 * random.nextDouble());
                const double loopDuration = sample.sampleRate * (1.45 + 0.45 * random.nextDouble());
                loopStart = juce::jmax(sample.sampleRate * 1.0, loopEnd - loopDuration);
                loopCrossfadeSeconds = 0.30;
            }
            else
            {
                loopStart = juce::jmin(length - sample.sampleRate * 1.0,
                    sample.sampleRate * (0.72 + 0.34 * random.nextDouble()));
                loopEnd = juce::jmax(loopStart + sample.sampleRate * 1.0,
                    length - sample.sampleRate * (0.38 + 0.24 * random.nextDouble()));
            }

            const bool isLowVibraAnchor = toneIndex == 9 && anchor.rootMidiNote == 36;
            const double vibraHammerSkipSeconds = isLowVibraAnchor
                ? 0.11 * (1.0 - std::exp(-static_cast<double>(attackSeconds) / 0.030))
                : 0.0;
            tonePlayers[static_cast<size_t>(layer)][static_cast<size_t>(player)].start(
                &sample,
                sample.sampleRate * (0.12 + 0.08 * random.nextDouble()
                                   + vibraHammerSkipSeconds),
                loopStart,
                loopEnd,
                loopCrossfadeSeconds,
                outputRate,
                false);
            gains[static_cast<size_t>(player)] *= sample.toneNormalisationGain;
        }
    }

    const double bowLength = static_cast<double>(sampleSet.bowSustain.audio.getNumSamples());
    const double bowLoopStart = sampleSet.bowSustain.sampleRate
        * (0.28 + 0.32 * random.nextDouble());
    const double bowLoopEnd = juce::jmin(bowLength - 2.0,
        sampleSet.bowSustain.sampleRate * (5.15 + 0.55 * random.nextDouble()));
    bowPlayer.start(&sampleSet.bowSustain,
                    0.0,
                    bowLoopStart,
                    bowLoopEnd,
                    0.48,
                    outputRate,
                    true);
}

void BowieVoice::stopNote(float, bool allowTailOff)
{
    if (allowTailOff)
        envelope.noteOff();
    else
    {
        envelope.reset();
        clearCurrentNote();
    }
}

void BowieVoice::pitchWheelMoved(int newPitchWheelValue)
{
    pitchWheelSemitones = 2.0f * (static_cast<float>(newPitchWheelValue) - 8192.0f) / 8192.0f;
}

void BowieVoice::controllerMoved(int controllerNumber, int newControllerValue)
{
    if (controllerNumber == 1)
        sharedModWheelAmount.store(
            juce::jlimit(0.0f, 1.0f,
                static_cast<float>(newControllerValue) / 127.0f),
            std::memory_order_relaxed);
}

float BowieVoice::oneShotSample(const BowieLoadedSample& source,
                                int channel, double position) const
{
    if (position >= static_cast<double>(source.audio.getNumSamples() - 1))
        return 0.0f;

    const int sourceChannel = juce::jmin(channel, source.audio.getNumChannels() - 1);
    const int index = juce::jmax(0, static_cast<int>(position));
    const float fraction = static_cast<float>(position - static_cast<double>(index));
    const float a = source.audio.getSample(sourceChannel, index);
    const float b = source.audio.getSample(sourceChannel, index + 1);
    const float sampleValue = a + fraction * (b - a);
    const double startFadeSamples = source.sampleRate * 0.003;
    const double endFadeSamples = source.sampleRate * 0.018;
    const double remaining = static_cast<double>(source.audio.getNumSamples() - 1) - position;
    const float edgeGain = static_cast<float>(juce::jlimit(0.0, 1.0,
        juce::jmin(position / startFadeSamples, remaining / endFadeSamples)));
    return sampleValue * edgeGain;
}

void BowieVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
                                 int startSample, int numSamples)
{
    if (!isVoiceActive())
        return;

    updateEnvelopeParameters();
    const float bowLevel = parameterValue(apvts, "bowLevel");
    const float bowMixGain = bowLevel * 0.42f;
    const float wobbleMaximumCents = parameterValue(apvts, "wobble");
    constexpr const char* levelParameterIds[] = {
        "voice1Level", "voice2Level", "voice3Level"
    };
    bool hasAudibleLayer = false;
    bool everyAudibleLayerDecays = true;
    double bowBodyDecaySeconds = 3.3;
    for (int layer = 0; layer < bodyLayerCount; ++layer)
    {
        const float level = parameterValue(apvts, levelParameterIds[layer]);
        layerLevels[static_cast<size_t>(layer)].setTargetValue(level);
        if (level <= 0.001f)
            continue;

        hasAudibleLayer = true;
        const int toneIndex = layerToneIndices[static_cast<size_t>(layer)];
        const bool decayingBody = toneIndex == 5 || toneIndex == 6 || toneIndex == 9;
        everyAudibleLayerDecays = everyAudibleLayerDecays && decayingBody;
        if (toneIndex == 6)
            bowBodyDecaySeconds = juce::jmax(bowBodyDecaySeconds, 4.2);
        else if (toneIndex == 5)
            bowBodyDecaySeconds = juce::jmax(bowBodyDecaySeconds, 3.6);
    }
    const bool fadeBowWithBody = hasAudibleLayer && everyAudibleLayerDecays;
    // Tone and bow must feel like one physical gesture, so both receive the
    // exact same bounded velocity gain after their internal blend is formed.
    const float gestureVelocityGain = 0.24f + 0.76f * noteVelocity;
    const double softStep = sampleSet.softAttack.sampleRate / outputRate;
    const double hardStep = sampleSet.hardAttack.sampleRate / outputRate;
    const double bowStep = sampleSet.bowSustain.sampleRate / outputRate;

    for (int frame = 0; frame < numSamples; ++frame)
    {
        const double time = noteAgeSeconds;
        const double strainEnvelope = std::exp(-time / strainDecaySeconds);
        const double irregular = 0.62 * std::sin(juce::MathConstants<double>::twoPi * strainRateAHz * time + wobblePhaseA)
                               + 0.38 * std::sin(juce::MathConstants<double>::twoPi * strainRateBHz * time + wobblePhaseB)
                               - strainInitialDrop * std::exp(-time / 0.055);
        const double cents = wobbleMaximumCents
                           * std::pow(static_cast<double>(noteVelocity), 1.35)
                           * strainDepthVariation * strainEnvelope * irregular;

        // Percussive bodies should not leave an exposed, indefinitely bowed tail.
        // Keep a small residue so the texture remains audible, but let it settle
        // with Piano, Rhodes and Vibra. Future layered bodies can use this same
        // trait and keep the bow sustained whenever any sustained body is present.
        const float bowBodyGain = fadeBowWithBody
            ? static_cast<float>(0.12 + 0.88 * std::exp(-time / bowBodyDecaySeconds))
            : 1.0f;

        // Keep the almost subliminal per-note human drift independent. The played
        // CC1 vibrato is generated once by the processor so every overlapping voice
        // follows the same phase and can neither beat nor cancel during legato.
        const double humanVibratoCents = 1.65 * std::sin(humanVibratoPhase);
        const double modVibratoCents = sharedVibratoCents.getSample(0, startSample + frame);
        const double bendRatio = std::pow(2.0,
            (static_cast<double>(pitchWheelSemitones)
             + (cents + humanVibratoCents + modVibratoCents) / 100.0) / 12.0);
        std::array<float, bodyLayerCount> layerGains;
        for (int layer = 0; layer < bodyLayerCount; ++layer)
            layerGains[static_cast<size_t>(layer)]
                = layerLevels[static_cast<size_t>(layer)].getNextValue();
        const float rawEnvelopeGain = envelope.getNextSample();
        smoothedEnvelope += envelopeGlideCoefficient
                          * (rawEnvelopeGain - smoothedEnvelope);
        const float envelopeGain = smoothedEnvelope;
        const float deClickGain = static_cast<float>(juce::jlimit(0.0, 1.0,
            noteAgeSeconds / 0.006));

        for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
        {
            float tone = 0.0f;
            for (int layer = 0; layer < bodyLayerCount; ++layer)
            {
                const auto layerIndex = static_cast<size_t>(layer);
                tone += (tonePlayers[layerIndex][0].sampleForChannel(channel)
                            * toneGains[layerIndex][0]
                       + tonePlayers[layerIndex][1].sampleForChannel(channel)
                            * toneGains[layerIndex][1])
                      * layerGains[layerIndex];
            }
            tone *= 0.50f;
            const float sustainBow = bowPlayer.sampleForChannel(channel)
                                   * 0.32f * bowBodyGain;
            const float gentleCatch = oneShotSample(sampleSet.softAttack, channel, softAttackPosition)
                                    * softAttackMix * 0.70f;
            const float hardCatch = oneShotSample(sampleSet.hardAttack, channel, hardAttackPosition)
                                  * hardAttackMix * 0.60f;
            const float mixed = (tone + bowMixGain * (sustainBow + gentleCatch + hardCatch))
                              * gestureVelocityGain * envelopeGain * deClickGain;
            outputBuffer.addSample(channel, startSample + frame, std::tanh(mixed));
        }

        for (int layer = 0; layer < bodyLayerCount; ++layer)
        {
            const auto layerIndex = static_cast<size_t>(layer);
            tonePlayers[layerIndex][0].advance(
                baseToneRatios[layerIndex][0] * bendRatio);
            if (toneGains[layerIndex][1] > 0.0f)
                tonePlayers[layerIndex][1].advance(
                    baseToneRatios[layerIndex][1] * bendRatio);
        }
        bowPlayer.advance(bowStep);
        softAttackPosition += softStep;
        hardAttackPosition += hardStep;
        humanVibratoPhase += static_cast<float>(juce::MathConstants<double>::twoPi * 0.46 / outputRate);
        if (humanVibratoPhase >= juce::MathConstants<float>::twoPi)
            humanVibratoPhase -= juce::MathConstants<float>::twoPi;
        noteAgeSeconds += 1.0 / outputRate;

        if (!envelope.isActive() && smoothedEnvelope < 1.0e-5f)
        {
            clearCurrentNote();
            break;
        }
    }
}
