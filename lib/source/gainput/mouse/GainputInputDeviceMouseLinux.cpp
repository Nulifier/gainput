
#include <gainput/gainput.h>


#if defined(GAINPUT_PLATFORM_LINUX)

#include "../GainputInputDeltaState.h"


namespace gainput
{

namespace
{
struct DeviceButtonInfo
{
	ButtonType type;
	const char* name;
};

DeviceButtonInfo deviceButtonInfos[] =
{
		{ BT_BOOL, "mouse_left" },
		{ BT_BOOL, "mouse_middle" },
		{ BT_BOOL, "mouse_right" },
		{ BT_BOOL, "mouse_3" },
		{ BT_BOOL, "mouse_4" },
		{ BT_BOOL, "mouse_5" },
		{ BT_BOOL, "mouse_6" },
		{ BT_BOOL, "mouse_7" },
		{ BT_BOOL, "mouse_8" },
		{ BT_BOOL, "mouse_9" },
		{ BT_BOOL, "mouse_10" },
		{ BT_BOOL, "mouse_11" },
		{ BT_BOOL, "mouse_12" },
		{ BT_BOOL, "mouse_13" },
		{ BT_BOOL, "mouse_14" },
		{ BT_BOOL, "mouse_15" },
		{ BT_BOOL, "mouse_16" },
		{ BT_BOOL, "mouse_17" },
		{ BT_BOOL, "mouse_18" },
		{ BT_BOOL, "mouse_19" },
		{ BT_BOOL, "mouse_20" },
		{ BT_FLOAT, "mouse_x" },
		{ BT_FLOAT, "mouse_y" }
};
}


const unsigned MouseButtonCount = 21;
const unsigned MouseAxisCount = 2;

class InputDeviceMouseImpl
{
public:
	InputDeviceMouseImpl(InputManager& manager, DeviceId device) :
		manager_(manager),
		device_(device)
	{
		const size_t size = sizeof(bool)*MouseButtonCount;
		isWheel_ = static_cast<bool*>(manager_.GetAllocator().Allocate(size));
		memset(isWheel_, 0, size);
	}

	~InputDeviceMouseImpl()
	{
		manager_.GetAllocator().Deallocate(isWheel_);
	}

	void Update(InputState& state, InputState& previousState, InputDeltaState* delta)
	{
		// Reset mouse wheel buttons
		for (unsigned i = 0; i < MouseButtonCount; ++i)
		{
			const DeviceButtonId buttonId = i;
			const bool oldValue = previousState.GetBool(buttonId);
			if (isWheel_[i] && oldValue)
			{
				const bool pressed = false;
				state.Set(buttonId, pressed);
#ifdef GAINPUT_DEBUG
				GAINPUT_LOG("Button: %i, %d\n", buttonId, pressed);
#endif
				if (delta)
				{
					if (oldValue != pressed)
					{
						delta->AddChange(device_, buttonId, oldValue, pressed);
					}
				}
			}
		}

		bool pressedThisFrame[MouseButtonCount];
		memset(pressedThisFrame, 0, sizeof(bool) * MouseButtonCount);

		const long eventMask = ButtonPressMask | ButtonReleaseMask | PointerMotionMask;
		XEvent event;
		while (XCheckMaskEvent(manager_.GetXDisplay(), eventMask, &event))
		{
			switch (event.type)
			{
			case MotionNotify:
				{
					const XMotionEvent& motionEvent = event.xmotion;
					const float x = float(motionEvent.x)/float(manager_.GetDisplayWidth());
					const float y = float(motionEvent.y)/float(manager_.GetDisplayHeight());
					state.Set(MOUSE_AXIS_X, x);
					state.Set(MOUSE_AXIS_Y, y);

					if (delta)
					{
						const float oldX = previousState.GetFloat(MOUSE_AXIS_X);
						const float oldY = previousState.GetFloat(MOUSE_AXIS_Y);
						if (oldX != x)
						{
							delta->AddChange(device_, MOUSE_AXIS_X, oldX, x);
						}
						if (oldY != y)
						{
							delta->AddChange(device_, MOUSE_AXIS_Y, oldY, y);
						}
					}
					break;
				}
			case ButtonPress:
			case ButtonRelease:
				{
					const XButtonEvent& buttonEvent = event.xbutton;
					GAINPUT_ASSERT(buttonEvent.button > 0);
					const DeviceButtonId buttonId = buttonEvent.button-1;
					GAINPUT_ASSERT(buttonId <= MOUSE_BUTTON_MAX);
					const bool pressed = event.type == ButtonPress;

					if (!pressed
							&& pressedThisFrame[buttonId])
					{
						// This is a mouse wheel button. Ignore release now, reset next frame.
						isWheel_[buttonId] = true;
					}
					else if (buttonEvent.button < MouseButtonCount)
					{
						state.Set(buttonId, pressed);

#ifdef GAINPUT_DEBUG
						GAINPUT_LOG("Button: %i, %d\n", buttonId, pressed);
#endif

						if (delta)
						{
							const bool oldValue = previousState.GetBool(buttonId);
							if (oldValue != pressed)
							{
								delta->AddChange(device_, buttonId, oldValue, pressed);
							}
						}
					}

					if (pressed)
					{
						pressedThisFrame[buttonId] = true;
					}
					break;
				}
			}
		}
	}

	DeviceId GetDevice() const { return device_; }

private:
	InputManager& manager_;
	DeviceId device_;
	bool* isWheel_;
};



InputDeviceMouse::InputDeviceMouse(InputManager& manager, DeviceId device) :
	impl_(new InputDeviceMouseImpl(manager, device))
{
	GAINPUT_ASSERT(impl_);
	state_ = new InputState(manager.GetAllocator(), MouseButtonCount + MouseAxisCount);
	GAINPUT_ASSERT(state_);
	previousState_ = new InputState(manager.GetAllocator(), MouseButtonCount + MouseAxisCount);
	GAINPUT_ASSERT(previousState_);
}

InputDeviceMouse::~InputDeviceMouse()
{
	delete state_;
	delete previousState_;
	delete impl_;
}

void
InputDeviceMouse::Update(InputDeltaState* delta)
{
	*previousState_ = *state_;
	impl_->Update(*state_, *previousState_, delta);
}

size_t
InputDeviceMouse::GetAnyButtonDown(DeviceButtonSpec* outButtons, size_t maxButtonCount) const
{
	return CheckAllButtonsDown(outButtons, maxButtonCount, MOUSE_BUTTON_0, MOUSE_BUTTON_MAX, impl_->GetDevice());
}

size_t
InputDeviceMouse::GetButtonName(DeviceButtonId deviceButton, char* buffer, size_t bufferLength) const
{
	GAINPUT_ASSERT(IsValidButtonId(deviceButton));
	strncpy(buffer, deviceButtonInfos[deviceButton].name, bufferLength);
	buffer[bufferLength-1] = 0;
	const size_t nameLen = strlen(deviceButtonInfos[deviceButton].name);
	return nameLen >= bufferLength ? bufferLength : nameLen+1;
}

ButtonType
InputDeviceMouse::GetButtonType(DeviceButtonId deviceButton) const
{
	GAINPUT_ASSERT(IsValidButtonId(deviceButton));
	return deviceButtonInfos[deviceButton].type;
}

}

#endif