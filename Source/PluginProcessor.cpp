#include "PluginProcessor.h"
#include "PluginEditor.h"

BowieAudioProcessor::BowieAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "BOWIE_STATE", createParameterLayout())
{
    const bool loadedSuccessfully = samples.loadAll();
    jassert(loadedSuccessfully);
    juce::ignoreUnused(loadedSuccessfully);

    for (int index = 0; index < 12; ++index)
        synthesiser.addVoice(new BowieVoice(samples, parameters, sharedModWheelAmount,
                                             sharedVibratoCents));

    synthesiser.addSound(new BowieSound());
}

juce::AudioProcessorValueTreeState::ParameterLayout BowieAudioProcessor::createParameterLayout()
{
    using Parameter = juce::AudioParameterFloat;
    using Range = juce::NormalisableRange<float>;
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> result;

    result.push_back(std::make_unique<Parameter>("attack", "Attack",
        Range(0.0f, 4.0f, 0.001f, 0.38f), 0.046f, "s"));
    result.push_back(std::make_unique<Parameter>("decay", "Decay",
        Range(0.01f, 3.0f, 0.001f, 0.4f), 0.453f, "s"));
    result.push_back(std::make_unique<Parameter>("sustain", "Sustain",
        Range(0.0f, 1.0f, 0.001f), 0.455f));
    result.push_back(std::make_unique<Parameter>("release", "Release",
        Range(0.01f, 6.0f, 0.001f, 0.38f), 0.238f, "s"));
    result.push_back(std::make_unique<Parameter>("attack2", "Voice 2 Attack",
        Range(0.0f, 4.0f, 0.001f, 0.38f), 0.046f, "s"));
    result.push_back(std::make_unique<Parameter>("decay2", "Voice 2 Decay",
        Range(0.01f, 3.0f, 0.001f, 0.4f), 0.453f, "s"));
    result.push_back(std::make_unique<Parameter>("sustain2", "Voice 2 Sustain",
        Range(0.0f, 1.0f, 0.001f), 0.455f));
    result.push_back(std::make_unique<Parameter>("release2", "Voice 2 Release",
        Range(0.01f, 6.0f, 0.001f, 0.38f), 0.238f, "s"));
    result.push_back(std::make_unique<Parameter>("attack3", "Voice 3 Attack",
        Range(0.0f, 4.0f, 0.001f, 0.38f), 0.046f, "s"));
    result.push_back(std::make_unique<Parameter>("decay3", "Voice 3 Decay",
        Range(0.01f, 3.0f, 0.001f, 0.4f), 0.453f, "s"));
    result.push_back(std::make_unique<Parameter>("sustain3", "Voice 3 Sustain",
        Range(0.0f, 1.0f, 0.001f), 0.455f));
    result.push_back(std::make_unique<Parameter>("release3", "Voice 3 Release",
        Range(0.01f, 6.0f, 0.001f, 0.38f), 0.238f, "s"));
    result.push_back(std::make_unique<Parameter>("bowLevel", "Bow Level",
        Range(0.0f, 0.20f, 0.001f), 0.022f));
    result.push_back(std::make_unique<Parameter>("wobble", "Pitch Strain",
        Range(0.0f, 80.0f, 0.1f, 0.55f), 80.0f, "cents"));
    result.push_back(std::make_unique<juce::AudioParameterChoice>(
        "tone", "Voice 1 Body", BowieSampleSet::toneNames(), 3));
    result.push_back(std::make_unique<juce::AudioParameterChoice>(
        "tone2", "Voice 2 Body", BowieSampleSet::toneNames(), 5));
    result.push_back(std::make_unique<juce::AudioParameterChoice>(
        "tone3", "Voice 3 Body", BowieSampleSet::toneNames(), 9));
    result.push_back(std::make_unique<Parameter>("voice1Level", "Voice 1 Level",
        Range(0.0f, 1.0f, 0.001f), 1.0f));
    result.push_back(std::make_unique<Parameter>("voice2Level", "Voice 2 Level",
        Range(0.0f, 1.0f, 0.001f), 0.0f));
    result.push_back(std::make_unique<Parameter>("voice3Level", "Voice 3 Level",
        Range(0.0f, 1.0f, 0.001f), 0.0f));
    result.push_back(std::make_unique<Parameter>("masterLevel", "Master Level",
        Range(0.0f, 1.0f, 0.001f), 1.0f));

    Range cutoffRange(40.0f, 20000.0f, 1.0f);
    cutoffRange.setSkewForCentre(1200.0f);
    result.push_back(std::make_unique<Parameter>("cutoff", "Cutoff",
        cutoffRange, 20000.0f, "Hz"));
    result.push_back(std::make_unique<Parameter>("resonance", "Resonance",
        Range(0.0f, 1.0f, 0.001f), 0.08f));

    return { result.begin(), result.end() };
}

juce::StringArray BowieAudioProcessor::factoryPresetNames()
{
    return { "Flute", "Into the Forest", "Saloon", "Fanfare",
             "Dirty Train", "3 Flutes", "Tape E-Piano" };
}

void BowieAudioProcessor::applyFactoryPreset(int presetIndex)
{
    struct Preset
    {
        int tones[3];
        float attack, decay, sustain, release;
        float bow, strain, cutoff, resonance;
        float levels[3];
        float master;
    };

    static constexpr Preset factoryPresets[] = {
        { { 3, 5, 9 }, 0.046f, 0.453f, 0.455f, 0.238f,
          0.022f, 80.0f, 20000.0f, 0.080f, { 1.000f, 0.000f, 0.000f }, 1.0f },
        { { 8, 9, 0 }, 0.468f, 0.686f, 0.665f, 0.782f,
          0.043f, 80.0f, 20000.0f, 0.328f, { 0.443f, 0.459f, 0.590f }, 1.0f },
        { { 6, 5, 1 }, 0.000f, 0.476f, 0.410f, 0.218f,
          0.066f, 80.0f, 20000.0f, 0.328f, { 0.443f, 0.180f, 0.213f }, 1.0f },
        { { 13, 12, 7 }, 0.002f, 0.862f, 0.658f, 0.213f,
          0.066f, 80.0f, 2900.0f, 0.328f, { 0.443f, 0.623f, 0.393f }, 1.0f },
        { { 3, 2, 9 }, 2.014f, 0.867f, 0.393f, 1.019f,
          0.108f, 24.8f, 4354.0f, 0.000f, { 0.770f, 0.230f, 0.246f }, 1.0f },
        { { 3, 3, 3 }, 0.073f, 0.867f, 0.656f, 0.218f,
          0.052f, 80.0f, 4354.0f, 0.475f, { 0.770f, 0.246f, 0.721f }, 1.0f },
        { { 5, 6, 9 }, 0.000f, 0.867f, 0.656f, 0.218f,
          0.052f, 80.0f, 1124.0f, 0.475f, { 0.197f, 0.607f, 0.230f }, 1.0f }
    };

    const auto setParameter = [this](const char* id, float value)
    {
        if (auto* parameter = parameters.getParameter(id))
            parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
    };

    const auto& preset = factoryPresets[static_cast<size_t>(
        juce::jlimit(0, static_cast<int>(std::size(factoryPresets)) - 1, presetIndex))];
    setParameter("tone", static_cast<float>(preset.tones[0]));
    setParameter("tone2", static_cast<float>(preset.tones[1]));
    setParameter("tone3", static_cast<float>(preset.tones[2]));
    setParameter("attack", preset.attack);
    setParameter("decay", preset.decay);
    setParameter("sustain", preset.sustain);
    setParameter("release", preset.release);
    setParameter("attack2", preset.attack);
    setParameter("decay2", preset.decay);
    setParameter("sustain2", preset.sustain);
    setParameter("release2", preset.release);
    setParameter("attack3", preset.attack);
    setParameter("decay3", preset.decay);
    setParameter("sustain3", preset.sustain);
    setParameter("release3", preset.release);
    if (presetIndex == 3)
    {
        // Fanfare uses the three independent envelopes captured from the
        // Farfisa / Oboe / Trumpet panel settings.
        setParameter("attack2", 0.739f);
        setParameter("decay2", 0.439f);
        setParameter("sustain2", 0.461f);
        setParameter("release2", 0.239f);
        setParameter("attack3", 0.645f);
        setParameter("decay3", 0.773f);
        setParameter("sustain3", 0.355f);
        setParameter("release3", 0.239f);
    }
    setParameter("bowLevel", preset.bow);
    setParameter("wobble", preset.strain);
    setParameter("cutoff", preset.cutoff);
    setParameter("resonance", preset.resonance);
    setParameter("voice1Level", preset.levels[0]);
    setParameter("voice2Level", preset.levels[1]);
    setParameter("voice3Level", preset.levels[2]);
    setParameter("masterLevel", preset.master);
}

void BowieAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    sharedVibratoPhase = 0.0;
    sharedVibratoCents.setSize(1, juce::jmax(1, samplesPerBlock), false, true, false);
    sharedVibratoCents.clear();
    synthesiser.setCurrentPlaybackSampleRate(sampleRate);
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = 2;
    outputFilter.prepare(spec);
    outputFilter.reset();
    outputFilter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    roomReverb.setSampleRate(sampleRate);
    roomReverb.reset();
    masterGain.reset(sampleRate, 0.025);
    masterGain.setCurrentAndTargetValue(
        parameters.getRawParameterValue("masterLevel")->load());
}

void BowieAudioProcessor::releaseResources()
{
    outputFilter.reset();
    roomReverb.reset();
}

bool BowieAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void BowieAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                       juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    keyboardState.processNextMidiBuffer(midiMessages, 0, buffer.getNumSamples(), true);

    if (sharedVibratoCents.getNumSamples() < buffer.getNumSamples())
        sharedVibratoCents.setSize(1, buffer.getNumSamples(), false, false, true);

    float modWheelAmount = sharedModWheelAmount.load(std::memory_order_relaxed);
    auto midiIterator = midiMessages.cbegin();
    const auto midiEnd = midiMessages.cend();
    for (int frame = 0; frame < buffer.getNumSamples(); ++frame)
    {
        while (midiIterator != midiEnd && (*midiIterator).samplePosition <= frame)
        {
            const auto message = (*midiIterator).getMessage();
            if (message.isController() && message.getControllerNumber() == 1)
            {
                modWheelAmount = juce::jlimit(0.0f, 1.0f,
                    static_cast<float>(message.getControllerValue()) / 127.0f);
                sharedModWheelAmount.store(modWheelAmount, std::memory_order_relaxed);
            }
            ++midiIterator;
        }

        const double upperHalf = juce::jmax(0.0f, (modWheelAmount - 0.5f) * 2.0f);
        const double vibratoHz = 4.1 + 0.8 * juce::jmin(1.0f, modWheelAmount * 2.0f)
                               + 5.3 * upperHalf * upperHalf;
        const float cents = static_cast<float>((20.0 * modWheelAmount
                                               + 6.0 * modWheelAmount * modWheelAmount)
                                              * std::sin(sharedVibratoPhase));
        sharedVibratoCents.setSample(0, frame, cents);
        sharedVibratoPhase += juce::MathConstants<double>::twoPi * vibratoHz
                            / currentSampleRate;
        if (sharedVibratoPhase >= juce::MathConstants<double>::twoPi)
            sharedVibratoPhase -= juce::MathConstants<double>::twoPi;
    }
    synthesiser.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

    outputFilter.setCutoffFrequency(parameters.getRawParameterValue("cutoff")->load());
    const float resonance = parameters.getRawParameterValue("resonance")->load();
    outputFilter.setResonance(0.65f + resonance * 2.35f);
    juce::dsp::AudioBlock<float> audioBlock(buffer);
    juce::dsp::ProcessContextReplacing<float> filterContext(audioBlock);
    outputFilter.process(filterContext);

    constexpr float room = 0.30f;
    juce::Reverb::Parameters roomSettings;
    roomSettings.roomSize = 0.18f + 0.35f * room;
    roomSettings.damping = 0.72f;
    roomSettings.wetLevel = 0.28f * room;
    roomSettings.dryLevel = 1.0f - 0.08f * room;
    roomSettings.width = 0.78f;
    roomSettings.freezeMode = 0.0f;
    roomReverb.setParameters(roomSettings);
    roomReverb.processStereo(buffer.getWritePointer(0), buffer.getWritePointer(1),
                             buffer.getNumSamples());

    masterGain.setTargetValue(parameters.getRawParameterValue("masterLevel")->load());
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        const float gain = masterGain.getNextValue();
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            buffer.getWritePointer(channel)[sample] *= gain;
    }
}

juce::AudioProcessorEditor* BowieAudioProcessor::createEditor()
{
    return new BowieAudioProcessorEditor(*this);
}

void BowieAudioProcessor::getStateInformation(juce::MemoryBlock& destination)
{
    if (auto xml = parameters.copyState().createXml())
        copyXmlToBinary(*xml, destination);
}

void BowieAudioProcessor::setStateInformation(const void* data, int size)
{
    if (auto xml = getXmlFromBinary(data, size))
        parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BowieAudioProcessor();
}
