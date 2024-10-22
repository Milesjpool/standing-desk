#pragma once
#include <Arduino.h>
#include <logger.h>
#include <deskSerial.h>

const uint MOVEMENT_TIMEOUT = 250; // Max height duration on a fresh reading, before considering the desk stopped.

class MovementDaemon {
private:
    Logger &logger;
    DeskSerial &deskSerial;

    boolean deskMoving = false;
public:
    MovementDaemon(Logger &logger, DeskSerial &deskSerial);
    void update();
    boolean isMoving();
};
