#include "stdafx.h"
#include "FollowCameraController.h"
#include "PhysicsBody.h"
#include "CarnageGame.h"
#include "Pedestrian.h"
#include "TimeManager.h"

FollowCameraController::FollowCameraController()
    : mStartupCameraHeight(32.0f)
    , mFollowPedCameraHeight(20.0f)
    , mFollowPedCameraCatchSpeed(20.0f)
    , mScrollHeightOffset()
{
}

void FollowCameraController::Setup(GameCamera* gameCamera)
{
    debug_assert(gameCamera);
    mCamera = gameCamera;

    // compute aspect ratio
    float screenAspect = (mCamera->mViewportRect.h > 0) ? ((mCamera->mViewportRect.w * 1.0f) / (mCamera->mViewportRect.h * 1.0f)) : 1.0f;

    // set camera defaults
    mCamera->SetIdentity();
    mCamera->SetPerspectiveProjection(screenAspect, 55.0f, 0.1f, 1000.0f);
    
    if (mFollowPedestrian)
    {
        glm::vec3 position = mFollowPedestrian->mTransformSmooth.mPosition;
        mCamera->SetPosition({position.x, position.y + mStartupCameraHeight, position.z}); 
    }
    else
    {
        mCamera->SetPosition({0.0f, mStartupCameraHeight, 0.0f});
    }
    mCamera->SetTopDownOrientation();
}

void FollowCameraController::UpdateFrame()
{
    if (mFollowPedestrian == nullptr)
        return;

    glm::vec3 position = mFollowPedestrian->mTransformSmooth.mPosition;

    float catchSpeed = mFollowPedCameraCatchSpeed;
    // todo: temporary implementation
    if (mFollowPedestrian->IsCarPassenger())
    {
        glm::vec2 carVelocity = mFollowPedestrian->mCurrentCar->mPhysicsBody->GetLinearVelocity();
        float carSpeed = glm::length(carVelocity);
        carVelocity = glm::normalize(carVelocity);
        //The number is the percentage of "leed" the camera will have (i.e. the larger it is then the
        //more infront of the car the camera will be centered)
        position.x += (carVelocity.x * carSpeed * 0.45f); // todo: magic numbers
        position.z += (carVelocity.y * carSpeed * 0.45f); // todo: magic numbers
        catchSpeed *= 0.3f; // todo: magic numbers

        // Also, as the car moves we want to "zoom" out some to allow more of the map to be seen
        float carHeightOffset = (MAX_SCROLL_HEIGHT_OFFSET - mScrollHeightOffset) * (carSpeed / mFollowPedestrian->mCurrentCar->mCarInfo->mMaxSpeed);
        if(carHeightOffset <= 0.0f)
          carHeightOffset = 0.0f;
        position.y += mFollowPedCameraHeight + mScrollHeightOffset + carHeightOffset;
    }
    else
    {
        //Not in a car so don't zoom out any
        position.y = position.y + (mFollowPedCameraHeight + mScrollHeightOffset);
    }

    float deltaTime = gTimeManager.mGameFrameDelta;
    if (glm::length(mCamera->mPosition - position) > 0.01f)
    {
        position = mCamera->mPosition + (position - mCamera->mPosition) * catchSpeed * deltaTime;
        mCamera->SetPosition(position); 
    }
}

void FollowCameraController::InputEvent(KeyInputEvent& inputEvent)
{
}

void FollowCameraController::InputEvent(MouseButtonInputEvent& inputEvent)
{
}

void FollowCameraController::InputEvent(MouseMovedInputEvent& inputEvent)
{
}

void FollowCameraController::InputEvent(MouseScrollInputEvent& inputEvent)
{
    mScrollHeightOffset = glm::clamp(mScrollHeightOffset - inputEvent.mScrollY,
                                     MIN_SCROLL_HEIGHT_OFFSET,
                                     MAX_SCROLL_HEIGHT_OFFSET);
}

void FollowCameraController::SetFollowTarget(Pedestrian* pedestrian)
{
    mFollowPedestrian = pedestrian;
}

void FollowCameraController::InputEventLost()
{
}
