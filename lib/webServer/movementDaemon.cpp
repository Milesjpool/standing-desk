#include <movementDaemon.h>

MovementDaemon::MovementDaemon(Logger &logger, DeskSerial &deskSerial)
    : deskSerial(deskSerial), logger(logger)
{
}

boolean MovementDaemon::isMoving()
{
    return deskMoving;
}

void MovementDaemon::update()
{
    HeightReading height = deskSerial.getLastHeightReading();

    boolean newState = height.isValid() && !height.isStale() && height.getDuration() < MOVEMENT_TIMEOUT;

    if (deskMoving != newState)
    {
        String transition = newState ? "started" : "stopped";
        logger.info("Desk " + transition + " moving.");
        deskMoving = newState;
    }
}