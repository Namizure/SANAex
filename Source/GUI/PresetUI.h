#pragma once
#include <JuceHeader.h>
#include "PresetManager.h"
#include "ScopeComponent.hpp"
#include "ProjectFonts.h"
#include "LNF/MyButton4LookAndFeel.h"
#include "LNF/MyButton5LookAndFeel.h"
#include "LNF/MyButton6LookAndFeel.h"
#include "LNF/MyComboBox2LookAndFeel.h"

namespace GUI {
	class PresetPanel : public Component,
		public Button::Listener,
		public ComboBox::Listener,
		public MouseListener,
		public ValueTree::Listener,
		public juce::AudioProcessorParameter::Listener
	{
	public:
		PresetPanel(Service::PresetManager& pm, juce::AudioProcessorValueTreeState& apvts)
			: presetManager(pm), valueTreeState(apvts)
		{
			configureButton(SaveBtn, "SAVE");
			ManekiText::set(SaveBtn).font(ProjectFonts::getBarlowBoldTypeface()).size(18.0f);
			SaveBtn.setLookAndFeel(&myButton4LookAndFeel);

			configureButton(LoadPackBtn, "LOAD");
			ManekiText::set(LoadPackBtn).font(ProjectFonts::getBarlowBoldTypeface()).size(18.0f);
			LoadPackBtn.setLookAndFeel(&myButton4LookAndFeel);

			drumModeButton.setButtonText("DRUM");
			ManekiText::set(drumModeButton).font(ProjectFonts::getBarlowBoldTypeface()).size(18.0f);
			drumModeButton.setLookAndFeel(&myButton4LookAndFeel);

			configureButton(Prev, "");
			Prev.setLookAndFeel(&myButton5LookAndFeel);

			configureButton(Next, "");
			Next.setLookAndFeel(&myButton6LookAndFeel);

			presetList.setJustificationType(juce::Justification::centred);
			ManekiText::set(presetList).font(ProjectFonts::getBarlowBoldTypeface()).size(18.0f);
			presetList.setLookAndFeel(&myComboBox2LookAndFeel);

			addAndMakeVisible(SaveBtn);
			addAndMakeVisible(LoadPackBtn);
			addAndMakeVisible(drumModeButton);
			addAndMakeVisible(Prev);
			addAndMakeVisible(Next);
			addAndMakeVisible(presetList);

			presetList.addListener(this);
			presetList.addMouseListener(this, true);
			valueTreeState.state.addListener(this);

			if (auto* param = getProgramIndexParam())
				param->addListener(this);

			loadPresetList();
		}

		~PresetPanel() override {
			SaveBtn.setLookAndFeel(nullptr);
			LoadPackBtn.setLookAndFeel(nullptr);
			drumModeButton.setLookAndFeel(nullptr);
			Prev.setLookAndFeel(nullptr);
			Next.setLookAndFeel(nullptr);
			presetList.setLookAndFeel(nullptr);

			SaveBtn.removeListener(this);
			LoadPackBtn.removeListener(this);
			Prev.removeListener(this);
			Next.removeListener(this);
			presetList.removeListener(this);

			presetList.removeMouseListener(this);
			valueTreeState.state.removeListener(this);

			if (auto* param = getProgramIndexParam())
				param->removeListener(this);
		}

		juce::TextButton drumModeButton{ "Drum" };
		std::function<void()> onPresetsChanged;
		std::function<void(int)> onPresetSelected;

		void loadPresetList() {
			presetList.clear(dontSendNotification);
			const auto allPresets = presetManager.getAllPresetsInPack();
			presetList.addItemList(allPresets, 1);
			presetList.setSelectedItemIndex(presetManager.getCurrentPresetIndex(), juce::dontSendNotification);

			if (onPresetsChanged)
				onPresetsChanged();
		}

		void parameterValueChanged(int parameterIndex, float newValue) override
		{
			if (parameterIndex == getProgramIndexParamIndex() && parameterIndex >= 0)
			{
				if (auto* param = dynamic_cast<juce::AudioParameterInt*>(getProgramIndexParam()))
				{
					int newIndex = juce::roundToInt(param->getNormalisableRange().convertFrom0to1(newValue));
					if (presetList.getSelectedItemIndex() != newIndex)
					{
						juce::MessageManager::callAsync([safeThis = juce::Component::SafePointer<PresetPanel>(this), newIndex]() {
							if (safeThis != nullptr) {
								safeThis->presetList.setSelectedItemIndex(newIndex, juce::dontSendNotification);
								if (safeThis->onPresetSelected)
									safeThis->onPresetSelected(newIndex);
							}
							});
					}
				}
			}
		}

		void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override {}

		void resized() override {
			auto bh = 30;

			SaveBtn.setBounds(3, 3, 55, bh);
			LoadPackBtn.setBounds(56, 3, 55, bh);
			drumModeButton.setBounds(109, 3, 55, bh);
			Prev.setBounds(170 - 9, 3, 26, bh);
			presetList.setBounds(194 - 9, 4, 200, bh - 2);
			Next.setBounds(392 - 9, 3, 26, bh);
		}

	private:
		int getProgramIndexParamIndex() const
		{
			const auto& params = presetManager.processor.getParameters();
			for (int i = 0; i < params.size(); ++i) {
				if (auto* paramWithID = dynamic_cast<juce::AudioProcessorParameterWithID*>(params[i])) {
					if (paramWithID->paramID == "PROGRAM_INDEX")
						return i;
				}
			}
			return -1;
		}

		juce::AudioProcessorParameter* getProgramIndexParam() const
		{
			const int index = getProgramIndexParamIndex();
			return index >= 0 ? presetManager.processor.getParameters().getUnchecked(index) : nullptr;
		}

		void mouseDown(const MouseEvent& e) override
		{
			if (e.originalComponent == &presetList || e.originalComponent->getParentComponent() == &presetList)
			{
				if (e.mods.isPopupMenu())
				{
					juce::PopupMenu menu;
					menu.addItem(1, "Rename Preset");
					menu.addItem(2, "Delete Preset");

					menu.showMenuAsync(juce::PopupMenu::Options{}.withTargetComponent(&presetList),
						[safeThis = juce::Component::SafePointer<PresetPanel>(this)](int result) {
							if (safeThis == nullptr)
								return;

							const int currentIndex = safeThis->presetManager.getCurrentPresetIndex();

							if (result == 1)
								safeThis->showRenameDialog(currentIndex);
							else if (result == 2)
							{
								safeThis->presetManager.deletePreset(currentIndex);
								safeThis->loadPresetList();
							}
						});
				}
			}
		}

		void showRenameDialog(int index)
		{
			auto* alert = new AlertWindow("Rename Preset", "", AlertWindow::NoIcon);
			alert->addTextEditor("presetName", presetList.getText());
			alert->addButton("OK", 1, KeyPress(KeyPress::returnKey));
			alert->addButton("Cancel", 0, KeyPress(KeyPress::escapeKey));

			alert->enterModalState(true, ModalCallbackFunction::create(
				[safeThis = juce::Component::SafePointer<PresetPanel>(this), index, alert](int result) {
					if (safeThis != nullptr && result == 1) {
						String newName = alert->getTextEditorContents("presetName");
						safeThis->presetManager.savePreset(index, newName);
						safeThis->loadPresetList();
					}
					delete alert;
				}));
		}

		void savePackFileIfNeeded()
		{
			if (presetManager.hasActivePackFile())
			{
				presetManager.savePack();
			}
			else
			{
				fileChooser = std::make_unique<FileChooser>(
					"Save Pack", Service::PresetManager::defaultdir, "*." + Service::PresetManager::extension);

				fileChooser->launchAsync(FileBrowserComponent::saveMode,
					[safeThis = juce::Component::SafePointer<PresetPanel>(this)](const FileChooser& chooser) {
						if (safeThis != nullptr) {
							const auto resultFile = chooser.getResult();
							if (resultFile != File{}) {
								safeThis->presetManager.savePack(resultFile);
								safeThis->loadPresetList();
							}
						}
					});
			}
		}

		void buttonClicked(Button* button) override
		{
			if (button == &SaveBtn)
			{
				const int currentIndex = presetManager.getCurrentPresetIndex();
				const auto allPresets = presetManager.getAllPresetsInPack();
				const juce::String currentName = (currentIndex >= 0 && currentIndex < allPresets.size()) ? allPresets[currentIndex].trim() : "";
				const bool isFromScratch = currentName.isEmpty() || currentName.startsWithIgnoreCase("Init");

				if (isFromScratch)
				{
					auto* alert = new AlertWindow("Save Preset", "Enter a name for this preset:", AlertWindow::NoIcon);
					alert->addTextEditor("presetName", "");
					alert->addButton("Save", 1, KeyPress(KeyPress::returnKey));
					alert->addButton("Cancel", 0, KeyPress(KeyPress::escapeKey));

					alert->enterModalState(true, ModalCallbackFunction::create(
						[safeThis = juce::Component::SafePointer<PresetPanel>(this), currentIndex, alert](int result) {
							if (safeThis != nullptr && result == 1) {
								juce::String newName = alert->getTextEditorContents("presetName").trim();
								if (newName.isEmpty())
									newName = "Preset " + juce::String(currentIndex + 1);

								safeThis->presetManager.savePreset(currentIndex, newName);
								safeThis->loadPresetList();
								safeThis->savePackFileIfNeeded();
							}
							delete alert;
						}));
				}
				else
				{
					presetManager.syncCurrentPresetFromParameters();
					savePackFileIfNeeded();
				}
			}
			else if (button == &LoadPackBtn)
			{
				fileChooser = std::make_unique<FileChooser>(
					"Select Pack to load", Service::PresetManager::defaultdir, "*." + Service::PresetManager::extension);

				fileChooser->launchAsync(FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles,
					[safeThis = juce::Component::SafePointer<PresetPanel>(this)](const FileChooser& chooser) {
						if (safeThis != nullptr) {
							const auto resultFile = chooser.getResult();
							if (resultFile.exists()) {
								safeThis->presetManager.loadPack(resultFile);
								safeThis->loadPresetList();
							}
						}
					});
			}
			else if (button == &Prev) {
				presetList.setSelectedItemIndex(presetManager.getPreviousPresetIndex(), sendNotificationSync);
			}
			else if (button == &Next) {
				presetList.setSelectedItemIndex(presetManager.getNextPresetIndex(), sendNotificationSync);
			}
		}

		void comboBoxChanged(ComboBox* ComboBoxThatHasChanged) override {
			if (ComboBoxThatHasChanged == &presetList) {
				const int idx = presetList.getSelectedItemIndex();
				presetManager.loadPreset(idx);
				if (onPresetSelected)
					onPresetSelected(idx);
			}
		}

		void configureButton(Button& button, const String& buttonText) {
			button.setButtonText(buttonText);
			button.addListener(this);
		}

		void valueTreePropertyChanged(ValueTree& treeWhosePropertyHasChanged, const Identifier& property) override
		{
			if (property.toString() == Service::PresetManager::packPathProperty) {
				juce::MessageManager::callAsync([safeThis = juce::Component::SafePointer<PresetPanel>(this)]() {
					if (safeThis != nullptr) {
						safeThis->loadPresetList();
					}
					});
			}
		}

		void valueTreeRedirected(ValueTree&) override
		{
			juce::MessageManager::callAsync([safeThis = juce::Component::SafePointer<PresetPanel>(this)]() {
				if (safeThis != nullptr) {
					safeThis->loadPresetList();
				}
				});
		}

		Service::PresetManager& presetManager;
		juce::AudioProcessorValueTreeState& valueTreeState;

		MyButton4LookAndFeel myButton4LookAndFeel;
		MyButton5LookAndFeel myButton5LookAndFeel;
		MyButton6LookAndFeel myButton6LookAndFeel;
		MyComboBox2LookAndFeel myComboBox2LookAndFeel;

		TextButton SaveBtn, LoadPackBtn, Prev, Next;
		ComboBox presetList;
		std::unique_ptr<FileChooser> fileChooser;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetPanel)
	};
}
