#include <movementDaemon.h>

MovementDaemon::MovementDaemon(Logger &logger, DeskSerial &deskSerial)
    : deskSerial(deskSerial), logger(logger), heightAnchor(0, 0, 0)
{
}

boolean MovementDaemon::isMoving()
{
    return deskMoving;
}

unsigned long MovementDaemon::getHeightDuration(HeightReading &currentHeight)
{
    if (heightAnchor.getHeight() != currentHeight.getHeight())
    {
        heightAnchor = currentHeight;
        return 0;
    }

    if (currentHeight.recorded_ms < heightAnchor.recorded_ms)
    {
        return ULONG_MAX - heightAnchor.recorded_ms + currentHeight.recorded_ms;
    }
    else
    {
        return currentHeight.recorded_ms - heightAnchor.recorded_ms;
    }
}

void MovementDaemon::update()
{
    HeightReading currentHeight = deskSerial.getLastHeightReading();

    boolean newState = currentHeight.isValid() && !currentHeight.isStale() && getHeightDuration(currentHeight) < MOVEMENT_TIMEOUT;

    if (deskMoving != newState)
    {
        String transition = newState ? "started" : "stopped";
        logger.info("Desk " + transition + " moving.");
        deskMoving = newState;
    }
}