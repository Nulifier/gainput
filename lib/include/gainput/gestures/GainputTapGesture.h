
#ifndef GAINPUTTAPGESTURE_H_
#define GAINPUTTAPGESTURE_H_

#ifdef GAINPUT_ENABLE_TAP_GESTURE

namespace gainput
{

/// Buttons provided by the TapGesture.
enum TapAction
{
	TapTriggered		///< The button that triggers by tapping.
};

/// A tap-to-trigger gesture.
class GAINPUT_LIBEXPORT TapGesture : public InputDevice
{
public:
	/// Initializes the gesture.
	TapGesture(InputManager& manager, DeviceId device);
	/// Uninitializes the gesture.
	~TapGesture();

	/// Sets up the gesture for operation without position checking.
	/**
	 * \param actionButtonDevice ID of the input device containing the action button.
	 * \param actionButton ID of the device button to be used as the action button.
	 * \param timeSpan Time in milliseconds the user may hold at most.
	 */
	void Initialize(DeviceId actionButtonDevice, DeviceButtonId actionButton, uint64_t timeSpan = 500);

	void Update(InputDeltaState* delta);

	/// Returns DT_GESTURE.
	DeviceType GetType() const { return DT_GESTURE; }
	DeviceState GetState() const { return DS_OK; }
	bool IsValidButtonId(DeviceButtonId deviceButton) const { return deviceButton == TapTriggered; }

	ButtonType GetButtonType(DeviceButtonId deviceButton) const { GAINPUT_ASSERT(IsValidButtonId(deviceButton)); return BT_BOOL; }

private:
	InputManager& manager_;
	DeviceButtonSpec actionButton_;

	uint64_t timeSpan_;
	uint64_t firstDownTime_;

};

}

#endif

#endif
