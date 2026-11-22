/*
  ==============================================================================

	MyInterface.cpp
	Created with Maneki UI Designer.

	This file was generated from a Maneki design. You can edit it freely, but
	exporting the design again will overwrite it.

  ==============================================================================
*/

#include "MyInterface.h"

MyInterface::MyInterface()
{
	SaveBtn.setButtonText("SAVE");
	ManekiText::set(SaveBtn).font("Barlow Condensed").bold();
	SaveBtn.setLookAndFeel(&myButton4LookAndFeel);
	addAndMakeVisible(SaveBtn);

	LoadPackBtn.setButtonText("LOAD");
	ManekiText::set(LoadPackBtn).font("Barlow Condensed").bold();
	LoadPackBtn.setLookAndFeel(&myButton4LookAndFeel);
	addAndMakeVisible(LoadPackBtn);

	drumModeButton.setButtonText("DRUM");
	drumModeButton.setClickingTogglesState(true);
	ManekiText::set(drumModeButton).font("Barlow Condensed").bold();
	drumModeButton.setLookAndFeel(&myButton4LookAndFeel);
	addAndMakeVisible(drumModeButton);

	Prev.setButtonText("");
	Prev.setLookAndFeel(&myButton5LookAndFeel);
	addAndMakeVisible(Prev);

	Next.setButtonText("");
	Next.setLookAndFeel(&myButton6LookAndFeel);
	addAndMakeVisible(Next);

	presetList.addItem("Init 1", 1);
	presetList.addItem("Init 2", 2);
	presetList.addItem("Init 3", 3);
	presetList.addItem("Init 4", 4);
	presetList.addItem("Init 5", 5);
	presetList.addItem("Init 6", 6);
	presetList.addItem("Init 7", 7);
	presetList.addItem("Init 8", 8);
	presetList.addItem("Init 9", 9);
	presetList.addItem("Init 10", 10);
	presetList.addItem("Init 11", 11);
	presetList.addItem("Init 12", 12);
	presetList.addItem("Init 13", 13);
	presetList.addItem("Init 14", 14);
	presetList.addItem("Init 15", 15);
	presetList.addItem("Init 16", 16);
	presetList.addItem("Init 17", 17);
	presetList.addItem("Init 18", 18);
	presetList.addItem("Init 19", 19);
	presetList.addItem("Init 20", 20);
	presetList.addItem("Init 21", 21);
	presetList.addItem("Init 22", 22);
	presetList.addItem("Init 23", 23);
	presetList.addItem("Init 24", 24);
	presetList.addItem("Init 25", 25);
	presetList.addItem("Init 26", 26);
	presetList.addItem("Init 27", 27);
	presetList.addItem("Init 28", 28);
	presetList.addItem("Init 29", 29);
	presetList.addItem("Init 30", 30);
	presetList.addItem("Init 31", 31);
	presetList.addItem("Init 32", 32);
	presetList.addItem("Init 33", 33);
	presetList.addItem("Init 34", 34);
	presetList.addItem("Init 35", 35);
	presetList.addItem("Init 36", 36);
	presetList.addItem("Init 37", 37);
	presetList.addItem("Init 38", 38);
	presetList.addItem("Init 39", 39);
	presetList.addItem("Init 40", 40);
	presetList.addItem("Init 41", 41);
	presetList.addItem("Init 42", 42);
	presetList.addItem("Init 43", 43);
	presetList.addItem("Init 44", 44);
	presetList.addItem("Init 45", 45);
	presetList.addItem("Init 46", 46);
	presetList.addItem("Init 47", 47);
	presetList.addItem("Init 48", 48);
	presetList.addItem("Init 49", 49);
	presetList.addItem("Init 50", 50);
	presetList.addItem("Init 51", 51);
	presetList.addItem("Init 52", 52);
	presetList.addItem("Init 53", 53);
	presetList.addItem("Init 54", 54);
	presetList.addItem("Init 55", 55);
	presetList.addItem("Init 56", 56);
	presetList.addItem("Init 57", 57);
	presetList.addItem("Init 58", 58);
	presetList.addItem("Init 59", 59);
	presetList.addItem("Init 60", 60);
	presetList.addItem("Init 61", 61);
	presetList.addItem("Init 62", 62);
	presetList.addItem("Init 63", 63);
	presetList.addItem("Init 64", 64);
	presetList.setSelectedItemIndex(0, juce::dontSendNotification);
	presetList.setJustificationType(juce::Justification::centred);
	ManekiText::set(presetList).font("Barlow");
	presetList.setLookAndFeel(&myComboBox2LookAndFeel);
	addAndMakeVisible(presetList);

	setSize(960, 620);
}

MyInterface::~MyInterface()
{
	SaveBtn.setLookAndFeel(nullptr);
	LoadPackBtn.setLookAndFeel(nullptr);
	drumModeButton.setLookAndFeel(nullptr);
	Prev.setLookAndFeel(nullptr);
	Next.setLookAndFeel(nullptr);
	presetList.setLookAndFeel(nullptr);
}

void MyInterface::paint(juce::Graphics& g)
{
	g.fillAll(juce::Colour(0xff2a2d31));
}

void MyInterface::resized()
{
	SaveBtn.setBounds(432, 6, 55, 26);
	LoadPackBtn.setBounds(484, 6, 55, 26);
	drumModeButton.setBounds(536, 6, 55, 26);
	Prev.setBounds(591, 6, 26, 26);
	Next.setBounds(791, 6, 26, 26);
	presetList.setBounds(613, 7, 182, 24);
}
