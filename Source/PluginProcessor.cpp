/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

const std::string NoiseGeneratorAudioProcessor::paramsNames[] = { "Volume" };

//==============================================================================
NoiseGeneratorAudioProcessor::NoiseGeneratorAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
	volumeParameter = apvts.getRawParameterValue(paramsNames[0]);

	buttonAParameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("ButtonA"));
	buttonBParameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("ButtonB"));
	buttonCParameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("ButtonC"));
	buttonDParameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("ButtonD"));
}

NoiseGeneratorAudioProcessor::~NoiseGeneratorAudioProcessor()
{
}

//==============================================================================
const juce::String NoiseGeneratorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NoiseGeneratorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool NoiseGeneratorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool NoiseGeneratorAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double NoiseGeneratorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NoiseGeneratorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int NoiseGeneratorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NoiseGeneratorAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String NoiseGeneratorAudioProcessor::getProgramName (int index)
{
    return {};
}

void NoiseGeneratorAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void NoiseGeneratorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{	
	const int channels = getTotalNumOutputChannels();
	if (channels <= 1)
		return;

	// Offset right channel
	for (int i = 0; i < 48000; i++)
	{
		m_whiteNoiseGenerator[1].process();
	}
}

void NoiseGeneratorAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NoiseGeneratorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void NoiseGeneratorAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Buttons
	const auto buttonA = buttonAParameter->get();
	const auto buttonB = buttonBParameter->get();
	const auto buttonC = buttonCParameter->get();
	const auto buttonD = buttonDParameter->get();

	// Get params
	float volume = 0.0f;
	if (buttonA)
		volume = juce::Decibels::decibelsToGain(volumeParameter->load()) * juce::Decibels::decibelsToGain(-42.0f);
	else if (buttonB)
		volume = juce::Decibels::decibelsToGain(volumeParameter->load()) * juce::Decibels::decibelsToGain(-43.0f);
	else if (buttonC)
		volume = juce::Decibels::decibelsToGain(volumeParameter->load()) * juce::Decibels::decibelsToGain(-47.0f);
	else if (buttonD)	
		volume = juce::Decibels::decibelsToGain(volumeParameter->load()) * juce::Decibels::decibelsToGain(-31.0f);

	// Mics constants
	const int channels = getTotalNumOutputChannels();
	const int samples = buffer.getNumSamples();		

	for (int channel = 0; channel < channels; ++channel)
	{
		auto* channelBuffer = buffer.getWritePointer(channel);
		auto& whiteNoiseGenerator = m_whiteNoiseGenerator[channel];

		if (buttonA)
			whiteNoiseGenerator.setDistributionType(WhiteNoiseGenerator::DistributionType::Uniform);
		else if (buttonB)
			whiteNoiseGenerator.setDistributionType(WhiteNoiseGenerator::DistributionType::Normal);
		else if (buttonC)
			whiteNoiseGenerator.setDistributionType(WhiteNoiseGenerator::DistributionType::Bernoulli);
		else if (buttonD)	
			whiteNoiseGenerator.setDistributionType(WhiteNoiseGenerator::DistributionType::PieceWise);

		for (int sample = 0; sample < samples; ++sample)
		{
			channelBuffer[sample] = volume * whiteNoiseGenerator.process();
		}
	}
}

//==============================================================================
bool NoiseGeneratorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* NoiseGeneratorAudioProcessor::createEditor()
{
    return new NoiseGeneratorAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void NoiseGeneratorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void NoiseGeneratorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout NoiseGeneratorAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>(-24.0f, 24.0f,   0.1f, 1.0f), 0.0f));

	layout.add(std::make_unique<juce::AudioParameterBool>("ButtonA", "ButtonA", true));
	layout.add(std::make_unique<juce::AudioParameterBool>("ButtonB", "ButtonB", false));
	layout.add(std::make_unique<juce::AudioParameterBool>("ButtonC", "ButtonC", false));
	layout.add(std::make_unique<juce::AudioParameterBool>("ButtonD", "ButtonC", false));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NoiseGeneratorAudioProcessor();
}
