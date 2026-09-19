#include <JuceHeader.h>
#include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>

#if JucePlugin_Build_Standalone

namespace juce
{
class BowieStandaloneApp final : public JUCEApplication
{
public:
    BowieStandaloneApp()
    {
        PropertiesFile::Options options;
        options.applicationName = CharPointer_UTF8(JucePlugin_Name);
        options.filenameSuffix = ".settings";
        options.folderName = "";
        appProperties.setStorageParameters(options);
    }

    const String getApplicationName() override { return CharPointer_UTF8(JucePlugin_Name); }
    const String getApplicationVersion() override { return JucePlugin_VersionString; }
    bool moreThanOneInstanceAllowed() override { return false; }
    void anotherInstanceStarted(const String&) override
    {
        if (mainWindow != nullptr)
        {
            mainWindow->setVisible(true);
            mainWindow->toFront(true);
        }
    }

    void initialise(const String&) override
    {
        auto* settings = appProperties.getUserSettings();
        discardEmptyOutputState(settings);

        auto holder = std::make_unique<StandalonePluginHolder>(settings, false);
        recoverOutputDevice(*holder);

        mainWindow = std::make_unique<StandaloneFilterWindow>(
            getApplicationName(),
            LookAndFeel::getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId),
            std::move(holder));
        mainWindow->setVisible(true);
    }

    void shutdown() override
    {
        mainWindow = nullptr;
        appProperties.saveIfNeeded();
    }

    void systemRequestedQuit() override
    {
        if (mainWindow != nullptr)
            mainWindow->getPluginHolder()->savePluginState();

        if (ModalComponentManager::getInstance()->cancelAllModalComponents())
        {
            Timer::callAfterDelay(100, []
            {
                if (auto* app = JUCEApplicationBase::getInstance())
                    app->systemRequestedQuit();
            });
        }
        else
        {
            quit();
        }
    }

private:
    static void discardEmptyOutputState(PropertySet* settings)
    {
        if (settings == nullptr)
            return;

        if (auto saved = settings->getXmlValue("audioSetup"))
            if (saved->getStringAttribute("audioOutputDeviceName").trim().isEmpty())
                settings->removeValue("audioSetup");
    }

    static void recoverOutputDevice(StandalonePluginHolder& holder)
    {
        if (holder.deviceManager.getCurrentAudioDevice() != nullptr)
            return;

        // A disconnected interface can leave JUCE with a blank saved endpoint.
        // Prefer a native ASIO driver for an instrument, then fall back to the
        // normal shared Windows endpoint used by general desktop applications.
        const StringArray preferredTypes { "ASIO", "Windows Audio", "CoreAudio" };
        for (const auto& preferredType : preferredTypes)
        {
            for (auto* type : holder.deviceManager.getAvailableDeviceTypes())
            {
                if (type->getTypeName() != preferredType)
                    continue;

                type->scanForDevices();
                holder.deviceManager.setCurrentAudioDeviceType(type->getTypeName(), true);
                for (const auto& outputName : type->getDeviceNames(false))
                {
                    AudioDeviceManager::AudioDeviceSetup setup;
                    holder.deviceManager.getAudioDeviceSetup(setup);
                    setup.outputDeviceName = outputName;
                    setup.inputDeviceName.clear();
                    setup.useDefaultInputChannels = false;
                    setup.useDefaultOutputChannels = true;
                    setup.sampleRate = 0.0;
                    setup.bufferSize = preferredType == "ASIO" ? 256 : 0;

                    if (holder.deviceManager.setAudioDeviceSetup(setup, true).isEmpty())
                    {
                        holder.saveAudioDeviceState();
                        return;
                    }
                }
            }
        }
    }

    ApplicationProperties appProperties;
    std::unique_ptr<StandaloneFilterWindow> mainWindow;
};
}

JUCE_CREATE_APPLICATION_DEFINE(juce::BowieStandaloneApp)

#endif
