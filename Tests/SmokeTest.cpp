#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <iostream>

int main(int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI juceInitialiser;

    if (argc > 1 && juce::String(argv[1]) == "--render-ui")
    {
        BowieAudioProcessor uiProcessor;
        std::unique_ptr<juce::AudioProcessorEditor> editor(uiProcessor.createEditor());
        editor->setVisible(true);
        auto* hit = editor->getComponentAt(812, 25);
        bool presetReceivesClicks = false;
        for (auto* component = hit; component != nullptr; component = component->getParentComponent())
            presetReceivesClicks = presetReceivesClicks
                || dynamic_cast<juce::ComboBox*>(component) != nullptr;
        auto preview = editor->createComponentSnapshot(editor->getLocalBounds(), true, 1.0f);

        const auto target = juce::File::getCurrentWorkingDirectory()
            .getChildFile("build-ninja")
            .getChildFile("BowieEditorPreview-0.4.5.png");
        target.deleteFile();
        juce::FileOutputStream stream(target);
        const bool written = stream.openedOk()
                          && juce::PNGImageFormat().writeImageToStream(preview, stream);
        std::cout << "uiPreview=" << target.getFullPathName()
                  << " written=" << (written ? "yes" : "no")
                  << " presetHit=" << (presetReceivesClicks ? "pass" : "fail")
                  << std::endl;
        return written && presetReceivesClicks ? 0 : 3;
    }

    if (argc > 1 && juce::String(argv[1]) == "--audio-device-diagnostic")
    {
        juce::AudioDeviceManager deviceManager;
        const auto error = deviceManager.initialiseWithDefaultDevices(0, 2);
        std::cout << "defaultDeviceError=" << error << std::endl;
        for (auto* type : deviceManager.getAvailableDeviceTypes())
        {
            type->scanForDevices();
            std::cout << "driverType=" << type->getTypeName() << std::endl;
            for (const auto& name : type->getDeviceNames(false))
            {
                deviceManager.closeAudioDevice();
                deviceManager.setCurrentAudioDeviceType(type->getTypeName(), true);
                juce::AudioDeviceManager::AudioDeviceSetup setup;
                deviceManager.getAudioDeviceSetup(setup);
                setup.outputDeviceName = name;
                setup.inputDeviceName.clear();
                setup.useDefaultInputChannels = false;
                setup.useDefaultOutputChannels = true;
                setup.sampleRate = 0.0;
                setup.bufferSize = type->getTypeName() == "ASIO" ? 256 : 0;
                const auto attempt = deviceManager.setAudioDeviceSetup(setup, true);
                std::cout << "  output=" << name
                          << " result=" << (attempt.isEmpty() ? "opened" : attempt)
                          << std::endl;
            }
        }
        const bool openedAnyDevice = deviceManager.getCurrentAudioDevice() != nullptr;
        if (auto* device = deviceManager.getCurrentAudioDevice())
            std::cout << "openedDevice=" << device->getName()
                      << " rate=" << device->getCurrentSampleRate()
                      << " block=" << device->getCurrentBufferSizeSamples()
                      << " outputs=" << device->getActiveOutputChannels().countNumberOfSetBits()
                      << std::endl;
        return openedAnyDevice ? 0 : 2;
    }

    BowieAudioProcessor processor;
    std::cout << "samplesReady=" << (processor.samplesAreReady() ? "yes" : "no") << std::endl;
    constexpr double sampleRate = 44100.0;
    constexpr int blockSize = 512;
    processor.setPlayConfigDetails(0, 2, sampleRate, blockSize);
    processor.prepareToPlay(sampleRate, blockSize);

    juce::AudioBuffer<float> audio(2, blockSize);
    double energy = 0.0;
    float peak = 0.0f;
    int nonSilentBlocks = 0;

    // Hold E5 long enough to force both dual-head loopers through boundaries.
    for (int block = 0; block < 900; ++block)
    {
        juce::MidiBuffer midi;
        if (block == 0)
            midi.addEvent(juce::MidiMessage::noteOn(1, 76, static_cast<juce::uint8>(96)), 0);
        if (block == 240)
            midi.addEvent(juce::MidiMessage::controllerEvent(1, 1, 64), 0);
        if (block == 480)
            midi.addEvent(juce::MidiMessage::controllerEvent(1, 1, 127), 0);
        if (block == 800)
            midi.addEvent(juce::MidiMessage::noteOff(1, 76), 0);

        audio.clear();
        processor.processBlock(audio, midi);

        float blockPeak = 0.0f;
        for (int channel = 0; channel < audio.getNumChannels(); ++channel)
        {
            const auto* samples = audio.getReadPointer(channel);
            for (int index = 0; index < audio.getNumSamples(); ++index)
            {
                const float value = samples[index];
                energy += static_cast<double>(value) * value;
                blockPeak = juce::jmax(blockPeak, std::abs(value));
            }
        }

        peak = juce::jmax(peak, blockPeak);
        if (blockPeak > 1.0e-5f)
            ++nonSilentBlocks;
    }

    const double rms = std::sqrt(energy / (900.0 * blockSize * 2.0));
    std::cout << "peak=" << peak << " rms=" << rms
              << " nonSilentBlocks=" << nonSilentBlocks << std::endl;

    BowieAudioProcessor shortNoteProcessor;
    shortNoteProcessor.setPlayConfigDetails(0, 2, sampleRate, blockSize);
    shortNoteProcessor.prepareToPlay(sampleRate, blockSize);
    if (auto* attack = shortNoteProcessor.parameters.getParameter("attack"))
        attack->setValueNotifyingHost(attack->convertTo0to1(0.0f));

    float firstRenderedSample = 1.0f;
    float previousSample = 0.0f;
    float noteOffBoundaryJump = 0.0f;
    for (int block = 0; block < 16; ++block)
    {
        juce::MidiBuffer midi;
        if (block == 0)
            midi.addEvent(juce::MidiMessage::noteOn(1, 76, static_cast<juce::uint8>(110)), 0);
        if (block == 4)
            midi.addEvent(juce::MidiMessage::noteOff(1, 76), 0);

        audio.clear();
        shortNoteProcessor.processBlock(audio, midi);
        if (block == 0)
            firstRenderedSample = audio.getSample(0, 0);
        if (block == 4)
            noteOffBoundaryJump = std::abs(audio.getSample(0, 0) - previousSample);
        previousSample = audio.getSample(0, blockSize - 1);
    }

    std::cout << "zeroAttackFirstSample=" << firstRenderedSample
              << " shortNoteOffJump=" << noteOffBoundaryJump << std::endl;

    BowieAudioProcessor libraryProcessor;
    libraryProcessor.setPlayConfigDetails(0, 2, sampleRate, blockSize);
    libraryProcessor.prepareToPlay(sampleRate, blockSize);
    auto* toneParameter = libraryProcessor.parameters.getParameter("tone");
    bool allToneModelsAudible = toneParameter != nullptr;
    for (int toneIndex = 0; toneIndex < BowieSampleSet::toneCount && allToneModelsAudible; ++toneIndex)
    {
        toneParameter->setValueNotifyingHost(toneParameter->convertTo0to1(static_cast<float>(toneIndex)));
        float modelPeak = 0.0f;
        // F#3 sits between the supplied C3/C4 anchors and therefore exercises
        // both sides of every equal-power register blend, including English Horn.
        for (int block = 0; block < 32; ++block)
        {
            juce::MidiBuffer midi;
            if (block == 0)
                midi.addEvent(juce::MidiMessage::noteOn(1, 54, static_cast<juce::uint8>(100)), 0);
            if (block == 22)
                midi.addEvent(juce::MidiMessage::noteOff(1, 54), 0);

            audio.clear();
            libraryProcessor.processBlock(audio, midi);
            for (int channel = 0; channel < audio.getNumChannels(); ++channel)
                modelPeak = juce::jmax(modelPeak,
                    audio.getMagnitude(channel, 0, audio.getNumSamples()));
        }

        std::cout << "tone[" << toneIndex << "] peak=" << modelPeak << std::endl;
        allToneModelsAudible = std::isfinite(modelPeak)
                            && modelPeak > 1.0e-4f
                            && modelPeak < 1.0f;
    }

    auto* cutoffParameter = libraryProcessor.parameters.getParameter("cutoff");
    auto* resonanceParameter = libraryProcessor.parameters.getParameter("resonance");
    const bool effectParametersPresent = cutoffParameter != nullptr
                                      && resonanceParameter != nullptr;
    float effectPeak = 0.0f;
    if (effectParametersPresent)
    {
        cutoffParameter->setValueNotifyingHost(cutoffParameter->convertTo0to1(650.0f));
        resonanceParameter->setValueNotifyingHost(resonanceParameter->convertTo0to1(0.70f));
        for (int block = 0; block < 44; ++block)
        {
            juce::MidiBuffer midi;
            if (block == 0)
                midi.addEvent(juce::MidiMessage::noteOn(1, 78, static_cast<juce::uint8>(108)), 0);
            if (block == 22)
                midi.addEvent(juce::MidiMessage::noteOff(1, 78), 0);

            audio.clear();
            libraryProcessor.processBlock(audio, midi);
            for (int channel = 0; channel < audio.getNumChannels(); ++channel)
                effectPeak = juce::jmax(effectPeak,
                    audio.getMagnitude(channel, 0, audio.getNumSamples()));
        }
    }
    std::cout << "roomFilterPeak=" << effectPeak << std::endl;
    const bool effectsStable = effectParametersPresent
                            && std::isfinite(effectPeak)
                            && effectPeak > 1.0e-5f
                            && effectPeak < 1.0f;

    BowieAudioProcessor layerProcessor;
    layerProcessor.setPlayConfigDetails(0, 2, sampleRate, blockSize);
    layerProcessor.prepareToPlay(sampleRate, blockSize);
    auto* voice1Level = layerProcessor.parameters.getParameter("voice1Level");
    auto* voice2Level = layerProcessor.parameters.getParameter("voice2Level");
    auto* voice3Level = layerProcessor.parameters.getParameter("voice3Level");
    auto* tone2 = layerProcessor.parameters.getParameter("tone2");
    auto* tone3 = layerProcessor.parameters.getParameter("tone3");
    auto* masterLevel = layerProcessor.parameters.getParameter("masterLevel");
    const bool layerParametersPresent = voice1Level != nullptr && voice2Level != nullptr
                                     && voice3Level != nullptr && tone2 != nullptr
                                     && tone3 != nullptr && masterLevel != nullptr;
    float layeredPeak = 0.0f;
    if (layerParametersPresent)
    {
        voice1Level->setValueNotifyingHost(voice1Level->convertTo0to1(0.0f));
        voice2Level->setValueNotifyingHost(voice2Level->convertTo0to1(0.62f));
        voice3Level->setValueNotifyingHost(voice3Level->convertTo0to1(0.48f));
        tone2->setValueNotifyingHost(tone2->convertTo0to1(5.0f));
        tone3->setValueNotifyingHost(tone3->convertTo0to1(9.0f));
        for (int block = 0; block < 36; ++block)
        {
            juce::MidiBuffer midi;
            if (block == 0)
                midi.addEvent(juce::MidiMessage::noteOn(1, 57, static_cast<juce::uint8>(102)), 0);
            if (block == 28)
                midi.addEvent(juce::MidiMessage::noteOff(1, 57), 0);
            audio.clear();
            layerProcessor.processBlock(audio, midi);
            for (int channel = 0; channel < audio.getNumChannels(); ++channel)
                layeredPeak = juce::jmax(layeredPeak,
                    audio.getMagnitude(channel, 0, audio.getNumSamples()));
        }
    }
    const bool layeringStable = layerParametersPresent
                             && std::isfinite(layeredPeak)
                             && layeredPeak > 1.0e-4f
                             && layeredPeak < 1.0f;
    std::cout << "layeringPeak=" << layeredPeak
              << " layering=" << (layeringStable ? "pass" : "fail") << std::endl;

    BowieAudioProcessor presetProcessor;
    const bool presetNamesStable =
        BowieAudioProcessor::factoryPresetNames().joinIntoString("|")
        == "Flute|Into the Forest|Saloon|Fanfare|Dirty Train|3 Flutes|Tape E-Piano";
    presetProcessor.applyFactoryPreset(1);
    const auto closeTo = [](float actual, float expected)
    {
        return std::abs(actual - expected) < 0.0015f;
    };
    const bool forestPresetStable =
        closeTo(presetProcessor.parameters.getRawParameterValue("tone")->load(), 8.0f)
        && closeTo(presetProcessor.parameters.getRawParameterValue("tone2")->load(), 9.0f)
        && closeTo(presetProcessor.parameters.getRawParameterValue("tone3")->load(), 0.0f)
        && closeTo(presetProcessor.parameters.getRawParameterValue("attack")->load(), 0.468f)
        && closeTo(presetProcessor.parameters.getRawParameterValue("bowLevel")->load(), 0.043f)
        && closeTo(presetProcessor.parameters.getRawParameterValue("resonance")->load(), 0.328f)
        && closeTo(presetProcessor.parameters.getRawParameterValue("voice1Level")->load(), 0.443f)
        && closeTo(presetProcessor.parameters.getRawParameterValue("voice2Level")->load(), 0.459f)
        && closeTo(presetProcessor.parameters.getRawParameterValue("voice3Level")->load(), 0.590f);
    presetProcessor.applyFactoryPreset(0);
    const bool initPresetStable =
        closeTo(presetProcessor.parameters.getRawParameterValue("tone")->load(), 3.0f)
        && closeTo(presetProcessor.parameters.getRawParameterValue("voice1Level")->load(), 1.0f)
        && closeTo(presetProcessor.parameters.getRawParameterValue("voice2Level")->load(), 0.0f)
        && closeTo(presetProcessor.parameters.getRawParameterValue("voice3Level")->load(), 0.0f);
    presetProcessor.applyFactoryPreset(2);
    const bool saloonPresetStable =
        closeTo(presetProcessor.parameters.getRawParameterValue("tone")->load(), 6.0f)
        && closeTo(presetProcessor.parameters.getRawParameterValue("tone2")->load(), 5.0f)
        && closeTo(presetProcessor.parameters.getRawParameterValue("tone3")->load(), 1.0f)
        && closeTo(presetProcessor.parameters.getRawParameterValue("attack")->load(), 0.0f)
        && closeTo(presetProcessor.parameters.getRawParameterValue("voice2Level")->load(), 0.180f);
    presetProcessor.applyFactoryPreset(3);
    const bool fanfarePresetStable =
        closeTo(presetProcessor.parameters.getRawParameterValue("tone")->load(), 0.0f)
        && closeTo(presetProcessor.parameters.getRawParameterValue("tone2")->load(), 2.0f)
        && closeTo(presetProcessor.parameters.getRawParameterValue("tone3")->load(), 8.0f)
        && closeTo(presetProcessor.parameters.getRawParameterValue("cutoff")->load(), 2900.0f)
        && closeTo(presetProcessor.parameters.getRawParameterValue("voice2Level")->load(), 0.623f);
    presetProcessor.applyFactoryPreset(4);
    const bool dirtyTrainPresetStable =
        closeTo(presetProcessor.parameters.getRawParameterValue("tone")->load(), 3.0f)
        && closeTo(presetProcessor.parameters.getRawParameterValue("tone2")->load(), 2.0f)
        && closeTo(presetProcessor.parameters.getRawParameterValue("tone3")->load(), 9.0f)
        && closeTo(presetProcessor.parameters.getRawParameterValue("attack")->load(), 2.014f)
        && closeTo(presetProcessor.parameters.getRawParameterValue("wobble")->load(), 24.8f)
        && closeTo(presetProcessor.parameters.getRawParameterValue("resonance")->load(), 0.0f);
    presetProcessor.applyFactoryPreset(5);
    const bool threeFlutesPresetStable =
        closeTo(presetProcessor.parameters.getRawParameterValue("tone")->load(), 3.0f)
        && closeTo(presetProcessor.parameters.getRawParameterValue("tone2")->load(), 3.0f)
        && closeTo(presetProcessor.parameters.getRawParameterValue("tone3")->load(), 3.0f)
        && closeTo(presetProcessor.parameters.getRawParameterValue("voice1Level")->load(), 0.770f)
        && closeTo(presetProcessor.parameters.getRawParameterValue("voice3Level")->load(), 0.721f);
    presetProcessor.applyFactoryPreset(6);
    const bool tapeEPianoPresetStable =
        closeTo(presetProcessor.parameters.getRawParameterValue("tone")->load(), 5.0f)
        && closeTo(presetProcessor.parameters.getRawParameterValue("tone2")->load(), 6.0f)
        && closeTo(presetProcessor.parameters.getRawParameterValue("tone3")->load(), 9.0f)
        && closeTo(presetProcessor.parameters.getRawParameterValue("cutoff")->load(), 1124.0f)
        && closeTo(presetProcessor.parameters.getRawParameterValue("voice2Level")->load(), 0.607f);
    const bool presetsStable = presetNamesStable && forestPresetStable && initPresetStable
                            && saloonPresetStable && fanfarePresetStable
                            && dirtyTrainPresetStable && threeFlutesPresetStable
                            && tapeEPianoPresetStable;
    std::cout << "factoryPresets=" << (presetsStable ? "pass" : "fail") << std::endl;

    BowieAudioProcessor controllerProcessor;
    controllerProcessor.setPlayConfigDetails(0, 2, sampleRate, blockSize);
    controllerProcessor.prepareToPlay(sampleRate, blockSize);
    juce::MidiBuffer wheelUp;
    wheelUp.addEvent(juce::MidiMessage::controllerEvent(1, 1, 127), 0);
    audio.clear();
    controllerProcessor.processBlock(audio, wheelUp);
    const bool idleWheelRemembered = controllerProcessor.currentModWheelAmount() > 0.999f;

    juce::MidiBuffer newNote;
    newNote.addEvent(juce::MidiMessage::noteOn(1, 64, static_cast<juce::uint8>(100)), 0);
    audio.clear();
    controllerProcessor.processBlock(audio, newNote);
    const bool newVoiceInheritedWheel = controllerProcessor.currentModWheelAmount() > 0.999f;

    juce::MidiBuffer wheelDown;
    wheelDown.addEvent(juce::MidiMessage::controllerEvent(1, 1, 0), 0);
    audio.clear();
    controllerProcessor.processBlock(audio, wheelDown);
    const bool wheelZeroReachedAllVoices = controllerProcessor.currentModWheelAmount() < 0.001f;
    const bool controllerStateStable = idleWheelRemembered
                                    && newVoiceInheritedWheel
                                    && wheelZeroReachedAllVoices;
    std::cout << "sharedCC1=" << (controllerStateStable ? "pass" : "fail") << std::endl;

    return peak > 1.0e-4f
        && nonSilentBlocks > 700
        && std::abs(firstRenderedSample) < 1.0e-6f
        && noteOffBoundaryJump < 0.05f
        && allToneModelsAudible
        && effectsStable
        && layeringStable
        && presetsStable
        && controllerStateStable ? 0 : 1;
}
