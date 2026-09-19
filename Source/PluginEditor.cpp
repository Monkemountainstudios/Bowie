#include "PluginEditor.h"
#include <BinaryData.h>

namespace
{
const auto ivory = juce::Colour(0xfff0e3c7);
const auto gold = juce::Colour(0xffc08a2d);
const auto royalBlue = juce::Colour(0xff17367f);
const auto lacquerRed = juce::Colour(0xffb32620);
const auto nearBlack = juce::Colour(0xff080a0d);
}

void BowieFineWheelSlider::mouseWheelMove(const juce::MouseEvent& event,
                                           const juce::MouseWheelDetails& wheel)
{
    auto fineWheel = wheel;
    fineWheel.deltaX *= 0.08f;
    fineWheel.deltaY *= 0.08f;
    juce::Slider::mouseWheelMove(event, fineWheel);
}

void BowieFaderLookAndFeel::drawLabel(juce::Graphics& graphics, juce::Label& label)
{
    if (! static_cast<bool>(label.getProperties()["bowiePanelLabel"]))
    {
        LookAndFeel_V4::drawLabel(graphics, label);
        return;
    }

    graphics.fillAll(label.findColour(juce::Label::backgroundColourId));
    if (label.isBeingEdited())
        return;

    const auto alpha = label.isEnabled() ? 1.0f : 0.5f;
    const juce::Font font(getLabelFont(label));
    const auto area = getLabelBorderSize(label).subtractedFrom(label.getLocalBounds());
    const int lines = juce::jmax(1, juce::roundToInt(
        static_cast<float>(area.getHeight()) / font.getHeight()));
    const auto drawAt = [&](juce::Rectangle<int> target)
    {
        graphics.drawFittedText(label.getText(), target, label.getJustificationType(),
                                lines, label.getMinimumHorizontalScale());
    };

    graphics.setFont(font);
    graphics.setColour(nearBlack.withMultipliedAlpha(0.34f * alpha));
    drawAt(area.translated(1, 2));

    // A one-pixel ivory keyline acts as a restrained outline over the blue end
    // of the body, while disappearing naturally into its ivory upper region.
    graphics.setColour(ivory.withMultipliedAlpha(0.72f * alpha));
    drawAt(area.translated(-1, 0));
    drawAt(area.translated(1, 0));
    drawAt(area.translated(0, -1));
    drawAt(area.translated(0, 1));

    graphics.setColour(label.findColour(juce::Label::textColourId)
                              .withMultipliedAlpha(alpha));
    drawAt(area);
}

juce::Font BowieFaderLookAndFeel::getComboBoxFont(juce::ComboBox& box)
{
    const float height = juce::jmin(11.0f, static_cast<float>(box.getHeight()) * 0.52f);
    return panelTypeface != nullptr
        ? juce::Font(juce::FontOptions(panelTypeface).withHeight(height))
        : juce::LookAndFeel_V4::getComboBoxFont(box).withHeight(height);
}

juce::Font BowieFaderLookAndFeel::getPopupMenuFont()
{
    return panelTypeface != nullptr
        ? juce::Font(juce::FontOptions(panelTypeface).withHeight(12.0f))
        : juce::LookAndFeel_V4::getPopupMenuFont();
}

juce::Typeface::Ptr BowieFaderLookAndFeel::getTypefaceForFont(const juce::Font& font)
{
    return panelTypeface != nullptr
        ? panelTypeface
        : juce::LookAndFeel_V4::getTypefaceForFont(font);
}

void BowieFaderLookAndFeel::drawLinearSlider(juce::Graphics& graphics,
                                              int x, int y, int width, int height,
                                              float sliderPos, float, float,
                                              juce::Slider::SliderStyle style,
                                              juce::Slider& slider)
{
    if (style != juce::Slider::LinearVertical)
    {
        LookAndFeel_V4::drawLinearSlider(graphics, x, y, width, height,
                                         sliderPos, 0.0f, 0.0f, style, slider);
        return;
    }

    const float centreX = static_cast<float>(x + width / 2);
    const float top = slider.getPositionOfValue(slider.getMaximum());
    const float bottom = slider.getPositionOfValue(slider.getMinimum());

    const auto railColour = slider.findColour(juce::Slider::backgroundColourId);
    graphics.setColour(nearBlack.withAlpha(0.34f));
    graphics.fillRoundedRectangle(centreX - 2.0f, top + 2.0f, 7.0f, bottom - top, 2.5f);

    juce::ColourGradient railGradient(railColour.darker(0.38f), centreX - 3.0f, top,
                                      railColour.darker(0.38f), centreX + 3.0f, top, false);
    railGradient.addColour(0.5, railColour.brighter(0.30f));
    graphics.setGradientFill(railGradient);
    graphics.fillRoundedRectangle(centreX - 3.0f, top, 6.0f, bottom - top, 2.0f);

    const auto trackColour = slider.findColour(juce::Slider::trackColourId);
    juce::ColourGradient trackGradient(trackColour.brighter(0.18f), centreX, sliderPos,
                                       trackColour.darker(0.28f), centreX, bottom, false);
    graphics.setGradientFill(trackGradient);
    graphics.fillRoundedRectangle(centreX - 2.0f, sliderPos, 4.0f,
                                  juce::jmax(1.0f, bottom - sliderPos), 1.5f);

    // Ruler marks: bottom, centre and top are anchors; intermediate marks
    // alternate two lengths on both sides of the fader.
    graphics.setColour(ivory.withAlpha(0.62f));
    for (int tick = 0; tick <= 20; ++tick)
    {
        const bool anchor = tick == 0 || tick == 10 || tick == 20;
        const float length = anchor ? 11.0f : (tick % 2 == 0 ? 7.0f : 4.0f);
        const float tickY = bottom - (bottom - top) * static_cast<float>(tick) / 20.0f;
        graphics.drawLine(centreX - 7.0f - length, tickY,
                          centreX - 7.0f, tickY, anchor ? 1.35f : 0.9f);
        graphics.drawLine(centreX + 7.0f, tickY,
                          centreX + 7.0f + length, tickY, anchor ? 1.35f : 0.9f);
    }

    graphics.setColour(nearBlack.withAlpha(0.45f));
    graphics.fillRoundedRectangle(centreX - 13.0f, sliderPos - 4.0f, 26.0f, 13.0f, 3.5f);

    const auto thumbColour = slider.findColour(juce::Slider::thumbColourId);
    juce::ColourGradient thumbGradient(thumbColour.brighter(0.35f), centreX, sliderPos - 6.0f,
                                       thumbColour.darker(0.30f), centreX, sliderPos + 6.0f, false);
    thumbGradient.addColour(0.42, thumbColour.brighter(0.08f));
    graphics.setGradientFill(thumbGradient);
    graphics.fillRoundedRectangle(centreX - 13.0f, sliderPos - 6.0f, 26.0f, 12.0f, 3.0f);
    graphics.setColour(thumbColour.darker(0.48f));
    graphics.drawRoundedRectangle(centreX - 13.0f, sliderPos - 6.0f, 26.0f, 12.0f, 3.0f, 0.8f);
    graphics.setColour(nearBlack.withAlpha(0.55f));
    graphics.drawHorizontalLine(juce::roundToInt(sliderPos), centreX - 10.0f, centreX + 10.0f);

    graphics.setColour(ivory.withAlpha(0.62f));
    graphics.drawHorizontalLine(juce::roundToInt(sliderPos - 4.0f),
                                centreX - 9.0f, centreX + 9.0f);

    graphics.setFont(panelTypeface != nullptr
        ? juce::FontOptions(panelTypeface).withHeight(7.5f)
        : juce::FontOptions(7.5f));
    graphics.setColour(ivory.withAlpha(0.72f));
    graphics.drawText("1", x + width - 11, juce::roundToInt(top) - 4, 9, 10,
                      juce::Justification::centred);
    graphics.drawText("0", x + width - 11, juce::roundToInt(bottom) - 5, 9, 10,
                      juce::Justification::centred);
}

BowieAudioProcessorEditor::BowieAudioProcessorEditor(BowieAudioProcessor& owner)
    : AudioProcessorEditor(owner),
      processor(owner),
      keyboard(owner.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel(&faderLookAndFeel);

    int fontBytes = 0;
    if (const auto* fontData = BinaryData::getNamedResource("BrunoAceOfficial_ttf", fontBytes))
        labelTypeface = juce::Typeface::createSystemTypefaceFor(
            fontData, static_cast<size_t>(fontBytes));
    faderLookAndFeel.setPanelTypeface(labelTypeface);

    fontBytes = 0;
    if (const auto* fontData = BinaryData::getNamedResource("Fancytext_ttf", fontBytes))
        wordmarkTypeface = juce::Typeface::createSystemTypefaceFor(
            fontData, static_cast<size_t>(fontBytes));

    configureSlider(attack, attackLabel, "ATTACK", "attack", attackAttachment);
    configureSlider(decay, decayLabel, "DECAY", "decay", decayAttachment);
    configureSlider(sustain, sustainLabel, "SUSTAIN", "sustain", sustainAttachment);
    configureSlider(release, releaseLabel, "RELEASE", "release", releaseAttachment);
    configureSlider(bowLevel, bowLabel, "BOW", "bowLevel", bowAttachment);
    configureSlider(wobble, wobbleLabel, "STRAIN", "wobble", wobbleAttachment);
    configureSlider(cutoff, cutoffLabel, "CUTOFF", "cutoff", cutoffAttachment);
    configureSlider(resonance, resonanceLabel, "RESO", "resonance", resonanceAttachment);
    configureSlider(voice1Level, voice1LevelLabel, "VOL 1", "voice1Level", voice1LevelAttachment);
    configureSlider(voice2Level, voice2LevelLabel, "VOL 2", "voice2Level", voice2LevelAttachment);
    configureSlider(voice3Level, voice3LevelLabel, "VOL 3", "voice3Level", voice3LevelAttachment);
    configureSlider(masterLevel, masterLevelLabel, "MASTER", "masterLevel", masterLevelAttachment);

    int imageBytes = 0;
    if (const auto* imageData = BinaryData::getNamedResource("BowieIcon_png", imageBytes))
        brandImage = juce::ImageFileFormat::loadFrom(imageData, static_cast<size_t>(imageBytes));

    configureToneSelector(tone, toneLabel, "VOICE 1", "tone", toneAttachment);
    configureToneSelector(tone2, tone2Label, "VOICE 2", "tone2", tone2Attachment);
    configureToneSelector(tone3, tone3Label, "VOICE 3", "tone3", tone3Attachment);

    presetLabel.setText("PRESET", juce::dontSendNotification);
    presetLabel.getProperties().set("bowiePanelLabel", true);
    presetLabel.setJustificationType(juce::Justification::centredRight);
    presetLabel.setColour(juce::Label::textColourId, royalBlue.darker(0.62f));
    presetLabel.setFont(labelTypeface != nullptr
        ? juce::FontOptions(labelTypeface).withHeight(10.5f)
        : juce::FontOptions(10.5f).withStyle("Bold"));
    addAndMakeVisible(presetLabel);

    preset.addItemList(BowieAudioProcessor::factoryPresetNames(), 1);
    preset.setTextWhenNothingSelected("SELECT...");
    preset.setColour(juce::ComboBox::backgroundColourId, royalBlue.darker(0.36f));
    preset.setColour(juce::ComboBox::textColourId, ivory.withAlpha(0.88f));
    preset.setColour(juce::ComboBox::outlineColourId, gold.withAlpha(0.62f));
    preset.setColour(juce::ComboBox::arrowColourId, gold.withAlpha(0.74f));
    preset.onChange = [this]
    {
        if (preset.getSelectedId() > 0)
            processor.applyFactoryPreset(preset.getSelectedId() - 1);
    };
    addAndMakeVisible(preset);

    voice1Level.onValueChange = [this] { updateLayerMenuBrightness(); };
    voice2Level.onValueChange = [this] { updateLayerMenuBrightness(); };
    voice3Level.onValueChange = [this] { updateLayerMenuBrightness(); };
    updateLayerMenuBrightness();

    subtitle.setText("Bowed instrument", juce::dontSendNotification);
    subtitle.getProperties().set("bowiePanelLabel", true);
    subtitle.setJustificationType(juce::Justification::centred);
    subtitle.setColour(juce::Label::textColourId, gold.brighter(0.35f));
    subtitle.setFont(labelTypeface != nullptr
        ? juce::FontOptions(labelTypeface).withHeight(12.0f)
        : juce::FontOptions(12.0f).withStyle("Bold"));
    // The subtitle overlaps the preset row visually, so it must remain transparent
    // to pointer input or it prevents the preset ComboBox from opening.
    subtitle.setInterceptsMouseClicks(false, false);
    addAndMakeVisible(subtitle);

    keyboard.setAvailableRange(36, 96);
    keyboard.setKeyWidth(24.0f);
    keyboard.setColour(juce::MidiKeyboardComponent::whiteNoteColourId, ivory);
    keyboard.setColour(juce::MidiKeyboardComponent::blackNoteColourId, nearBlack);
    keyboard.setColour(juce::MidiKeyboardComponent::keySeparatorLineColourId, gold.darker(0.3f));
    keyboard.setColour(juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, royalBlue.withAlpha(0.45f));
    keyboard.setColour(juce::MidiKeyboardComponent::keyDownOverlayColourId, lacquerRed.withAlpha(0.65f));
    addAndMakeVisible(keyboard);

    setSize(920, 460);
}

BowieAudioProcessorEditor::~BowieAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void BowieAudioProcessorEditor::configureSlider(juce::Slider& slider,
                                                 juce::Label& label,
                                                 const juce::String& text,
                                                 const juce::String& parameterId,
                                                 std::unique_ptr<Attachment>& attachment)
{
    slider.setSliderStyle(juce::Slider::LinearVertical);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    slider.setColour(juce::Slider::trackColourId, lacquerRed);
    slider.setColour(juce::Slider::backgroundColourId, royalBlue.darker(0.45f));
    slider.setColour(juce::Slider::thumbColourId, gold.brighter(0.25f));
    slider.setColour(juce::Slider::textBoxTextColourId, ivory);
    slider.setColour(juce::Slider::textBoxBackgroundColourId, nearBlack.brighter(0.14f));
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(slider);

    label.setText(text, juce::dontSendNotification);
    label.getProperties().set("bowiePanelLabel", true);
    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, royalBlue.darker(0.62f));
    label.setFont(labelTypeface != nullptr
        ? juce::FontOptions(labelTypeface).withHeight(12.0f)
        : juce::FontOptions(12.0f).withStyle("Bold"));
    addAndMakeVisible(label);

    attachment = std::make_unique<Attachment>(processor.parameters, parameterId, slider);
}

void BowieAudioProcessorEditor::configureToneSelector(
    juce::ComboBox& selector, juce::Label& label, const juce::String& text,
    const juce::String& parameterId, std::unique_ptr<ComboAttachment>& attachment)
{
    label.setText(text, juce::dontSendNotification);
    label.getProperties().set("bowiePanelLabel", true);
    label.setJustificationType(juce::Justification::centredRight);
    label.setColour(juce::Label::textColourId, royalBlue.darker(0.62f));
    label.setFont(labelTypeface != nullptr
        ? juce::FontOptions(labelTypeface).withHeight(11.0f)
        : juce::FontOptions(11.0f).withStyle("Bold"));
    addAndMakeVisible(label);

    selector.addItemList(BowieSampleSet::toneNames(), 1);
    selector.setColour(juce::ComboBox::backgroundColourId, royalBlue.darker(0.36f));
    selector.setColour(juce::ComboBox::textColourId, ivory.withAlpha(0.88f));
    selector.setColour(juce::ComboBox::outlineColourId, gold.withAlpha(0.62f));
    selector.setColour(juce::ComboBox::arrowColourId, gold.withAlpha(0.74f));
    addAndMakeVisible(selector);
    attachment = std::make_unique<ComboAttachment>(processor.parameters,
                                                    parameterId, selector);
}

void BowieAudioProcessorEditor::updateLayerMenuBrightness()
{
    const auto update = [](juce::ComboBox& selector, juce::Label& label, double level)
    {
        const float opacity = level <= 0.0005 ? 0.40f : 1.0f;
        selector.setAlpha(opacity);
        label.setAlpha(opacity);
    };

    update(tone, toneLabel, voice1Level.getValue());
    update(tone2, tone2Label, voice2Level.getValue());
    update(tone3, tone3Label, voice3Level.getValue());
}

void BowieAudioProcessorEditor::paint(juce::Graphics& graphics)
{
    juce::ColourGradient background(ivory.brighter(0.08f), 0.0f, 0.0f,
                                    royalBlue.darker(0.48f), 0.0f,
                                    static_cast<float>(getHeight()), false);
    background.addColour(0.22, ivory.darker(0.06f));
    background.addColour(0.58, royalBlue.darker(0.08f));
    graphics.setGradientFill(background);
    graphics.fillAll();

    // Broad, low-contrast highlights and shadows make the body read as a
    // shallow moulded shell without disturbing the restrained palette.
    juce::ColourGradient bodySheen(juce::Colours::white.withAlpha(0.16f), 20.0f, 12.0f,
                                   juce::Colours::transparentWhite, 650.0f, 275.0f, true);
    graphics.setGradientFill(bodySheen);
    graphics.fillRoundedRectangle(getLocalBounds().toFloat().reduced(11.0f), 10.0f);

    if (brandImage.isValid())
        graphics.drawImageWithin(brandImage, 24, 24, 76, 76,
            juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);

    graphics.setColour(royalBlue.darker(0.66f));
    graphics.setFont(wordmarkTypeface != nullptr
        ? juce::FontOptions(wordmarkTypeface).withHeight(44.0f)
        : juce::FontOptions("Segoe Script", "Bold", 44.0f));
    graphics.drawText("Bowie", 0, 14, getWidth(), 58, juce::Justification::centred);

    graphics.setColour(gold.darker(0.2f));
    juce::Path steppedDivider;
    steppedDivider.startNewSubPath(18.0f, 282.0f);
    steppedDivider.lineTo(270.0f, 282.0f);
    steppedDivider.lineTo(270.0f, 130.0f);
    steppedDivider.lineTo(584.0f, 130.0f);
    steppedDivider.lineTo(584.0f, 282.0f);
    steppedDivider.lineTo(902.0f, 282.0f);
    graphics.setColour(nearBlack.withAlpha(0.30f));
    graphics.strokePath(steppedDivider,
        juce::PathStrokeType(2.6f, juce::PathStrokeType::curved,
                             juce::PathStrokeType::rounded),
        juce::AffineTransform::translation(0.0f, 2.0f));
    graphics.setColour(gold.darker(0.2f));
    graphics.strokePath(steppedDivider,
        juce::PathStrokeType(1.35f, juce::PathStrokeType::curved,
                             juce::PathStrokeType::rounded));

    const auto keyRecess = keyboard.getBounds().toFloat().expanded(7.0f, 6.0f);
    graphics.setColour(nearBlack.withAlpha(0.46f));
    graphics.fillRoundedRectangle(keyRecess.translated(0.0f, 2.0f), 4.0f);
    juce::ColourGradient recessGradient(royalBlue.darker(0.72f), keyRecess.getX(), keyRecess.getY(),
                                        royalBlue.darker(0.35f), keyRecess.getX(), keyRecess.getBottom(), false);
    graphics.setGradientFill(recessGradient);
    graphics.fillRoundedRectangle(keyRecess, 4.0f);
    graphics.setColour(ivory.withAlpha(0.32f));
    graphics.drawHorizontalLine(juce::roundToInt(keyRecess.getY() + 1.0f),
                                keyRecess.getX() + 3.0f, keyRecess.getRight() - 3.0f);

    const auto frame = getLocalBounds().toFloat().reduced(9.0f);
    graphics.setColour(nearBlack.withAlpha(0.42f));
    graphics.drawRoundedRectangle(frame.translated(0.0f, 2.0f), 10.0f, 2.2f);
    graphics.setColour(gold.darker(0.22f));
    graphics.drawRoundedRectangle(frame, 10.0f, 1.3f);
    graphics.setColour(ivory.withAlpha(0.58f));
    graphics.drawLine(frame.getX() + 10.0f, frame.getY() + 1.0f,
                      frame.getRight() - 10.0f, frame.getY() + 1.0f, 1.0f);
}

void BowieAudioProcessorEditor::resized()
{
    subtitle.setBounds(0, 66, getWidth(), 24);
    constexpr int menuLabelX = 692;
    constexpr int menuX = 750;
    constexpr int menuWidth = 132;
    constexpr int menuHeight = 19;
    presetLabel.setBounds(menuLabelX, 16, 54, menuHeight);
    preset.setBounds(menuX, 16, menuWidth, menuHeight);

    juce::ComboBox* selectors[] = { &tone, &tone2, &tone3 };
    juce::Label* selectorLabels[] = { &toneLabel, &tone2Label, &tone3Label };
    for (int index = 0; index < 3; ++index)
    {
        const int y = 42 + index * 26;
        selectorLabels[index]->setBounds(menuLabelX, y, 54, menuHeight);
        selectors[index]->setBounds(menuX, y, menuWidth, menuHeight);
    }

    juce::Slider* envelopeSliders[] = { &attack, &decay, &sustain, &release };
    juce::Label* envelopeLabels[] = {
        &attackLabel, &decayLabel, &sustainLabel, &releaseLabel
    };
    for (int index = 0; index < 4; ++index)
    {
        const int x = 22 + index * 60;
        envelopeLabels[index]->setBounds(x, 147, 58, 18);
        envelopeSliders[index]->setBounds(x + 3, 165, 52, 100);
    }

    juce::Slider* shapingSliders[] = { &bowLevel, &wobble, &cutoff, &resonance };
    juce::Label* shapingLabels[] = { &bowLabel, &wobbleLabel, &cutoffLabel, &resonanceLabel };
    for (int index = 0; index < 4; ++index)
    {
        const int x = 282 + index * 72;
        shapingLabels[index]->setBounds(x, 147, 66, 18);
        shapingSliders[index]->setBounds(x + 4, 165, 58, 100);
    }

    juce::Slider* mixSliders[] = {
        &voice1Level, &voice2Level, &voice3Level, &masterLevel
    };
    juce::Label* mixLabels[] = {
        &voice1LevelLabel, &voice2LevelLabel, &voice3LevelLabel, &masterLevelLabel
    };
    for (int index = 0; index < 4; ++index)
    {
        const int x = 590 + index * 77;
        mixLabels[index]->setBounds(x, 147, 73, 18);
        mixSliders[index]->setBounds(x + 5, 165, 63, 100);
    }

    keyboard.setBounds(24, 316, getWidth() - 48, 112);
}
