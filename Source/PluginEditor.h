#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class BowieFaderLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    void setPanelTypeface(juce::Typeface::Ptr typeface) { panelTypeface = std::move(typeface); }
    void drawLinearSlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          juce::Slider::SliderStyle, juce::Slider&) override;
    void drawLabel(juce::Graphics&, juce::Label&) override;
    juce::Font getComboBoxFont(juce::ComboBox&) override;
    juce::Font getPopupMenuFont() override;
    juce::Typeface::Ptr getTypefaceForFont(const juce::Font&) override;

private:
    juce::Typeface::Ptr panelTypeface;
};

class BowieFineWheelSlider final : public juce::Slider
{
public:
    void mouseWheelMove(const juce::MouseEvent&,
                        const juce::MouseWheelDetails&) override;
};

class BowieAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit BowieAudioProcessorEditor(BowieAudioProcessor&);
    ~BowieAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    void configureSlider(juce::Slider& slider, juce::Label& label,
                         const juce::String& text, const juce::String& parameterId,
                         std::unique_ptr<Attachment>& attachment);
    void configureToneSelector(juce::ComboBox&, juce::Label&, const juce::String& text,
                               const juce::String& parameterId,
                               std::unique_ptr<ComboAttachment>& attachment);
    void selectEnvelope(int voiceIndex);
    void updateLayerMenuBrightness();

    BowieAudioProcessor& processor;
    BowieFaderLookAndFeel faderLookAndFeel;
    juce::MidiKeyboardComponent keyboard;
    juce::Slider attack, decay, sustain, release, bowLevel, wobble, resonance;
    juce::Slider voice1Level, voice2Level, voice3Level, masterLevel;
    BowieFineWheelSlider cutoff;
    juce::Label attackLabel, decayLabel, sustainLabel, releaseLabel, bowLabel, wobbleLabel;
    juce::Label cutoffLabel, resonanceLabel;
    juce::Label voice1LevelLabel, voice2LevelLabel, voice3LevelLabel, masterLevelLabel;
    juce::ComboBox tone, tone2, tone3;
    juce::Label toneLabel, tone2Label, tone3Label, subtitle;
    juce::ComboBox preset;
    juce::Label presetLabel;
    juce::Label envelopeVoiceLabel;
    std::array<juce::TextButton, 3> envelopeVoiceButtons;
    juce::Image brandImage;
    juce::Typeface::Ptr labelTypeface;
    juce::Typeface::Ptr wordmarkTypeface;
    std::unique_ptr<Attachment> attackAttachment, decayAttachment, sustainAttachment;
    std::unique_ptr<Attachment> releaseAttachment, bowAttachment, wobbleAttachment;
    std::unique_ptr<Attachment> cutoffAttachment, resonanceAttachment;
    std::unique_ptr<Attachment> voice1LevelAttachment, voice2LevelAttachment;
    std::unique_ptr<Attachment> voice3LevelAttachment, masterLevelAttachment;
    std::unique_ptr<ComboAttachment> toneAttachment, tone2Attachment, tone3Attachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BowieAudioProcessorEditor)
};
