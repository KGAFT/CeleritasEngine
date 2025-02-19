//
// Created by Daniil on 26.12.2022.
//
#pragma once

#include "Window/Window.hpp"
#include "Camera.h"
class CameraResizeCallBack : public IWindowResizeCallback{
private:
    Camera* currentCamera;
public:
    CameraResizeCallBack(Camera *currentCamera) : currentCamera(currentCamera) {}

    void resized(int width, int height) override {
        currentCamera->setViewPortWidth(width);
        currentCamera->setViewPortHeight(height);
    }

    Camera *getCurrentCamera() const {
        return currentCamera;
    }

    void setCurrentCamera(Camera *currentCamera) {
        CameraResizeCallBack::currentCamera = currentCamera;
    }
};
