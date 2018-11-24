#include "controller.h"
#include "utils.h"
#include "logger.h"
#include "driver.h"

#include <assert.h>

namespace steamvrbridge {

	Controller::Controller() 
	: TrackableDevice()
	, m_overrideModel("") {
		// Initially no button maps to any enumulated touchpad action
		memset(m_psButtonIDToEmulatedTouchpadAction, k_EmulatedTrackpadAction_None, k_PSMButtonID_Count * sizeof(vr::EVRButtonId));
	}

	Controller::~Controller() {
	}

	void Controller::LoadSettings(vr::IVRSettings *pSettings) {
		TrackableDevice::LoadSettings(pSettings);

		// Get the controller override model to use, if any
		char modelString[64];
		vr::EVRSettingsError fetchError;

		pSettings->GetString(GetControllerSettingsPrefix(), "override_model", modelString, 64, &fetchError);
		if (fetchError == vr::VRSettingsError_None)
		{
			m_overrideModel = modelString;
		}
	}

	void Controller::LoadEmulatedTouchpadActions(
		vr::IVRSettings *pSettings,
		const ePSMButtonID psButtonID,
		int controllerId) {

		eEmulatedTrackpadAction vrTouchpadDirection = k_EmulatedTrackpadAction_None;

		if (pSettings != nullptr) {
			vr::EVRSettingsError fetchError;

			const char *szPSButtonName = k_PSMButtonNames[psButtonID];

			char szTouchpadSectionName[64];
			snprintf(szTouchpadSectionName, sizeof(szTouchpadSectionName), "%s_emulated_touchpad", GetControllerSettingsPrefix());

			char remapButtonToTouchpadDirectionString[32];
			pSettings->GetString(szTouchpadSectionName, szPSButtonName, remapButtonToTouchpadDirectionString, 32, &fetchError);

			if (fetchError == vr::VRSettingsError_None) {
				for (int vr_touchpad_direction_index = 0; vr_touchpad_direction_index < k_max_vr_touchpad_directions; ++vr_touchpad_direction_index) {
					if (strcasecmp(remapButtonToTouchpadDirectionString, k_VRTouchpadDirectionNames[vr_touchpad_direction_index]) == 0) {
						vrTouchpadDirection = static_cast<eEmulatedTrackpadAction>(vr_touchpad_direction_index);
						break;
					}
				}
			}

			if (controllerId >= 0 && controllerId <= 9) {
				char szTouchpadSectionControllerName[64];
				snprintf(szTouchpadSectionControllerName, sizeof(szTouchpadSectionControllerName), 
					"%s_%d", szTouchpadSectionName, controllerId);

				pSettings->GetString(
					szTouchpadSectionControllerName, szPSButtonName, remapButtonToTouchpadDirectionString, 32, &fetchError);

				if (fetchError == vr::VRSettingsError_None) {
					for (int vr_touchpad_direction_index = 0; vr_touchpad_direction_index < k_max_vr_touchpad_directions; ++vr_touchpad_direction_index) {
						if (strcasecmp(remapButtonToTouchpadDirectionString, k_VRTouchpadDirectionNames[vr_touchpad_direction_index]) == 0) {
							vrTouchpadDirection = static_cast<eEmulatedTrackpadAction>(vr_touchpad_direction_index);
							break;
						}
					}
				}
			}
		}

		// Save the mapping
		assert(psButtonID >= 0 && psButtonID < k_PSMButtonID_Count);
		m_psButtonIDToEmulatedTouchpadAction[psButtonID] = vrTouchpadDirection;
	}

	// Shared Implementation of vr::ITrackedDeviceServerDriver
	vr::EVRInitError Controller::Activate(vr::TrackedDeviceIndex_t unObjectId) {

		vr::EVRInitError result_code= TrackableDevice::Activate(unObjectId);

		if (result_code == vr::EVRInitError::VRInitError_None) {
			vr::CVRPropertyHelpers *properties = vr::VRProperties();
		
			// Configure JSON controller configuration input profile
			char szProfilePath[128];
			snprintf(szProfilePath, sizeof(szProfilePath), "{psmove}/input/%s_profile.json", GetControllerSettingsPrefix());
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_InputProfilePath_String, szProfilePath);
		}

		return result_code;
	}

	bool Controller::CreateButtonComponent(ePSMButtonID button_id)
	{
		if (m_buttonStates.count(button_id) == 0) {
			vr::EVRInputError result_code= vr::EVRInputError::VRInputError_InvalidParam;
			vr::VRInputComponentHandle_t buttonComponentHandle;

			if (button_id < k_PSMButtonID_Count) {
				result_code=
					vr::VRDriverInput()->CreateBooleanComponent(
						m_ulPropertyContainer, k_PSMButtonPaths[button_id], &buttonComponentHandle);

			}

			if (result_code == vr::EVRInputError::VRInputError_None)
			{
				ButtonState buttonState;
				buttonState.buttonComponentHandle= buttonComponentHandle;
				buttonState.lastButtonState= PSMButtonState_UP;

				m_buttonStates[button_id]= buttonState;
			}

			return result_code == vr::EVRInputError::VRInputError_None;
		}

		return false;
	}

	bool Controller::CreateAxisComponent(ePSMAxisID axis_id) {
		if (m_axisStates.count(axis_id) == 0) {
			vr::EVRInputError result_code= vr::EVRInputError::VRInputError_InvalidParam;
			vr::VRInputComponentHandle_t axisComponentHandle;

			if (axis_id < k_PSMButtonID_Count) {
				result_code=
					vr::VRDriverInput()->CreateScalarComponent(
						m_ulPropertyContainer, k_PSMAxisPaths[axis_id], &axisComponentHandle,
						vr::VRScalarType_Absolute, 
						k_PSMAxisTwoSided[axis_id] ? vr::VRScalarUnits_NormalizedTwoSided : vr::VRScalarUnits_NormalizedOneSided);
			}

			if (result_code == vr::EVRInputError::VRInputError_None)
			{
				AxisState axisState;
				axisState.axisComponentHandle= axisComponentHandle;
				axisState.lastAxisState= 0.f;

				m_axisStates[axis_id]= axisState;
			}

			return result_code == vr::EVRInputError::VRInputError_None;
		}

		return false;
	}

	bool Controller::CreateHapticComponent(ePSMHapicID haptic_id)
	{
		if (m_hapticStates.count(haptic_id) == 0) {
			vr::VRInputComponentHandle_t hapticComponentHandle;
			vr::EVRInputError result_code=
				vr::VRDriverInput()->CreateHapticComponent(
					m_ulPropertyContainer, k_PSMButtonPaths[haptic_id], &hapticComponentHandle);

			if (result_code == vr::EVRInputError::VRInputError_None)
			{
				HapticState hapticState;
				hapticState.hapticComponentHandle= hapticComponentHandle;
				hapticState.pendingHapticDurationSecs= DEFAULT_HAPTIC_DURATION;
				hapticState.pendingHapticAmplitude= DEFAULT_HAPTIC_AMPLITUDE;
				hapticState.pendingHapticFrequency= DEFAULT_HAPTIC_FREQUENCY;
				hapticState.lastTimeRumbleSent= std::chrono::time_point<std::chrono::high_resolution_clock>();
				hapticState.lastTimeRumbleSentValid= false;

				m_hapticStates[haptic_id]= hapticState;
			}

			return result_code == vr::EVRInputError::VRInputError_None;
		}

		return false;
	}

	void Controller::UpdateButton(ePSMButtonID button_id, PSMButtonState button_state, double time_offset)
	{
		if (m_buttonStates.count(button_id) != 0) {
			const bool is_pressed= (button_state == PSMButtonState_PRESSED) || (button_state == PSMButtonState_DOWN);

			m_buttonStates[button_id].lastButtonState= button_state;
			vr::VRDriverInput()->UpdateBooleanComponent(
				m_buttonStates[button_id].buttonComponentHandle, is_pressed, time_offset);
		}
	}

	void Controller::UpdateAxis(ePSMAxisID axis_id, float axis_value, double time_offset)
	{
		if (m_axisStates.count(axis_id) != 0) {
			m_axisStates[axis_id].lastAxisState= axis_value;
			vr::VRDriverInput()->UpdateScalarComponent(
				m_axisStates[axis_id].axisComponentHandle, axis_value, time_offset);
		}
	}

	void Controller::UpdateHaptics(const vr::VREvent_HapticVibration_t &hapticData) {
		for (auto &it : m_hapticStates)
		{
			HapticState &state= it.second;

			if (state.hapticComponentHandle == hapticData.componentHandle)
			{
				state.pendingHapticDurationSecs = hapticData.fDurationSeconds;
				state.pendingHapticAmplitude = hapticData.fAmplitude;
				state.pendingHapticFrequency = hapticData.fFrequency;

				break;
			}
		}
	}


	bool Controller::HasButton(ePSMButtonID button_id) const
	{
		return m_buttonStates.count(button_id) != 0;
	}

	bool Controller::HasAxis(ePSMAxisID axis_id) const
	{
		return m_axisStates.count(axis_id) != 0;
	}

	bool Controller::HasHapticState(ePSMHapicID haptic_id) const
	{
		return m_hapticStates.count(haptic_id) != 0;
	}

	bool Controller::GetButtonState(ePSMButtonID button_id, PSMButtonState &out_button_state) const
	{
		if (m_buttonStates.count(button_id) != 0) {
			out_button_state= m_buttonStates.at(button_id).lastButtonState;
			return true;
		}

		out_button_state= PSMButtonState_UP;
		return false;
	}

	bool Controller::GetAxisState(ePSMAxisID axis_id, float &out_axis_value) const
	{
		if (m_axisStates.count(axis_id) != 0) {
			out_axis_value= m_axisStates.at(axis_id).lastAxisState;
			return true;
		}

		out_axis_value= 0.f;
		return false;
	}

	Controller::HapticState * Controller::GetHapticState(ePSMHapicID haptic_id)
	{
		if (m_hapticStates.count(haptic_id) != 0) {
			return &m_hapticStates.at(haptic_id);
		}

		return nullptr;
	}
}