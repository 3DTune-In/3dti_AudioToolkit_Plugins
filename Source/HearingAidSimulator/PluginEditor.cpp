/**
 * \class HASPluginAudioProcessorEditor
 *
 * \brief Declaration of HASPluginAudioProcessorEditor interface.
 * \date  November 2020
 *
 * \authors Reactify Music LLP: R. Hrafnkelsson ||
 * Coordinated by , A. Reyes-Lecuona (University of Malaga) and L.Picinali (Imperial College London) ||
 * \b Contact: areyes@uma.es and l.picinali@imperial.ac.uk
 *
 * \b Project: 3DTI (3D-games for TUNing and lEarnINg about hearing aids) ||
 * \b Website: http://3d-tune-in.eu/
 *
 * \b Copyright: University of Malaga and Imperial College London - 2020
 *
 * \b Licence: This copy of the 3D Tune-In Toolkit Plugin is licensed to you under the terms described in the LICENSE.md file included in this distribution.
 *
 * \b Acknowledgement: This project has received funding from the European Union's Horizon 2020 research and innovation programme under grant agreements No 644051 and 726765.
 */

#include "Common/Constants.h"
#include "Presets.h"
#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
HASPluginAudioProcessorEditor::HASPluginAudioProcessorEditor (HASPluginAudioProcessor& p)
  : AudioProcessorEditor(&p),
    processor(p),
    channelSettingsComponent (p),
    dynamicEQComponent (p)
{
    aboutText.setMultiLine(true);
    aboutText.setFont(Font(16.0f, Font::plain));
    aboutText.setText(String::fromUTF8(BinaryData::About_HAS_txt, BinaryData::About_HAS_txtSize));
    aboutText.setReadOnly(true);
    aboutText.setAlpha(0.9f);
    aboutText.setEnabled (false);
    aboutText.addMouseListener (this, true);
    aboutText.setVisible (false);
    
    aboutBanner.button.onClick = [this] { aboutText.setVisible(!aboutText.isVisible()); };
    
    audiogramComponent.reset (new AudiogramComponent (p.getBandFrequenciesAudiometry(), 2));
    audiogramComponent->addListener (this);
    audiogramComponent->linkToggle.onClick = [this] {
        *processor.hearingLossLink = audiogramComponent->linkToggle.getToggleState();
    };
    
    addAndMakeVisible (channelSettingsComponent);
    addAndMakeVisible (audiogramComponent.get());
    addAndMakeVisible (dynamicEQComponent);
    addAndMakeVisible (aboutBanner);
    addChildComponent (aboutText);
    
    setSize (800, 960);
    
    startTimer (30);
}

HASPluginAudioProcessorEditor::~HASPluginAudioProcessorEditor() {
    stopTimer();
}

void HASPluginAudioProcessorEditor::timerCallback()
{
    channelSettingsComponent.updateGUIState();
    dynamicEQComponent.updateGUIState();
    
    bool channelLink = processor.channelLink->get();
    
    bool hearingLossLink = processor.hearingLossLink->get();
    
    audiogramComponent->linkToggle.setToggleState (hearingLossLink, dontSendNotification);
    
    for (int ch = 0; ch < processor.getTotalNumOutputChannels(); ch++)
    {
        int channel = (! channelLink) * ch;
        
        bool channelEnabled = processor.channelEnabled[channel]->get();
        audiogramComponent->setChannelEnabled (ch, channelEnabled);
        dynamicEQComponent.setChannelEnabled (ch, channelEnabled);
        
        channel = (! hearingLossLink) * ch;
        
        for (int i = 0; i < Constants::NUM_BANDS; i++)
            audiogramComponent->setSliderValue (ch, i, (double)processor.hearingLoss[channel][i]->get());
        
        int selectedPreset = selectedPresets().audiometryPreset[channel];
        audiogramComponent->presetSelectors[ch].setSelectedItemIndex (selectedPreset, dontSendNotification);
    }
}

//==============================================================================
void HASPluginAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (Colour (24, 31, 34));
}
                                    
void HASPluginAudioProcessorEditor::resized()
{
    auto r = getLocalBounds();

    aboutBanner.setBounds (r.removeFromTop (50));
    aboutText.setBounds (r);

    channelSettingsComponent.setBounds (r.removeFromTop (r.proportionOfHeight (0.14f)));
    audiogramComponent->setBounds (r.removeFromTop (r.proportionOfHeight (0.4f)));
    dynamicEQComponent.setBounds (r);
}

void HASPluginAudioProcessorEditor::audiogramPresetSelected (int channel, int index)
{
    for (int i = 0; i < Constants::NUM_BANDS; i++)
        *processor.hearingLoss[channel][i] = hearingLossPresetValues[index][i];
    
    selectedPresets().audiometryPreset[channel] = index;
}

void HASPluginAudioProcessorEditor::audiogramSliderValueChanged (int channel, int band, double value)
{
    *processor.hearingLoss[channel][band] = (float)value;
}
