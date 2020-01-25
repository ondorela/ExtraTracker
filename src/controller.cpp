#include "controller.h"
#include "driverlog.h"
#include "helper.h"
#include <Windows.h>


CSampleControllerDriver::CSampleControllerDriver()
{
    m_unObjectId = vr::k_unTrackedDeviceIndexInvalid;
    m_ulPropertyContainer = vr::k_ulInvalidPropertyContainer;

    m_sSerialNumber = "CTRL_1234";

    m_sModelNumber = "MyController";
}

vr::EVRInitError CSampleControllerDriver::Activate(vr::TrackedDeviceIndex_t unObjectId)
{
    m_unObjectId = unObjectId;
    m_ulPropertyContainer = vr::VRProperties()->TrackedDeviceToPropertyContainer(m_unObjectId);

    vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ModelNumber_String, m_sModelNumber.c_str());
    vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_RenderModelName_String, m_sModelNumber.c_str());

    // return a constant that's not 0 (invalid) or 1 (reserved for Oculus)
    vr::VRProperties()->SetUint64Property(m_ulPropertyContainer, vr::Prop_CurrentUniverseId_Uint64, 2);

    // avoid "not fullscreen" warnings from vrmonitor
    vr::VRProperties()->SetBoolProperty(m_ulPropertyContainer, vr::Prop_IsOnDesktop_Bool, false);

    // our sample device isn't actually tracked, so set this property to avoid having the icon blink in the status window
    vr::VRProperties()->SetBoolProperty(m_ulPropertyContainer, vr::Prop_NeverTracked_Bool, true);

    // even though we won't ever track we want to pretend to be the right hand so binding will work as expected
    vr::VRProperties()->SetInt32Property(m_ulPropertyContainer, vr::Prop_ControllerRoleHint_Int32, vr::TrackedControllerRole_RightHand);

    // this file tells the UI what to show the user for binding this controller as well as what default bindings should
    // be for legacy or other apps
    vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_InputProfilePath_String, "{sample}/input/mycontroller_profile.json");

    // create all the input components
    vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/a/click", &m_compA);
    vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/b/click", &m_compB);
    vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/c/click", &m_compC);

    // create our haptic component
    vr::VRDriverInput()->CreateHapticComponent(m_ulPropertyContainer, "/output/haptic", &m_compHaptic);

    return vr::VRInitError_None;
}

/** debug request from a client */
void CSampleControllerDriver::DebugRequest(const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize)
{
    if (unResponseBufferSize >= 1)
        pchResponseBuffer[0] = 0;
}

vr::DriverPose_t CSampleControllerDriver::GetPose()
{
    vr::DriverPose_t pose = {0};
    pose.poseIsValid = false;
    pose.result = vr::TrackingResult_Calibrating_OutOfRange;
    pose.deviceIsConnected = true;

    pose.qWorldFromDriverRotation = HmdQuaternion_Init(1, 0, 0, 0);
    pose.qDriverFromHeadRotation = HmdQuaternion_Init(1, 0, 0, 0);

    return pose;
}

void CSampleControllerDriver::RunFrame()
{
#if defined(_WINDOWS)
    // Your driver would read whatever hardware state is associated with its input components and pass that
    // in to UpdateBooleanComponent. This could happen in RunFrame or on a thread of your own that's reading USB
    // state. There's no need to update input state unless it changes, but it doesn't do any harm to do so.

    vr::VRDriverInput()->UpdateBooleanComponent(m_compA, (0x8000 & GetAsyncKeyState('A')) != 0, 0);
    vr::VRDriverInput()->UpdateBooleanComponent(m_compB, (0x8000 & GetAsyncKeyState('B')) != 0, 0);
    vr::VRDriverInput()->UpdateBooleanComponent(m_compC, (0x8000 & GetAsyncKeyState('C')) != 0, 0);
#endif
}

void CSampleControllerDriver::ProcessEvent(const vr::VREvent_t &vrEvent)
{
    switch (vrEvent.eventType)
    {
    case vr::VREvent_Input_HapticVibration:
    {
        if (vrEvent.data.hapticVibration.componentHandle == m_compHaptic)
        {
            // This is where you would send a signal to your hardware to trigger actual haptic feedback
            DriverLog("BUZZ!\n");
        }
    }
    break;
    }
}
