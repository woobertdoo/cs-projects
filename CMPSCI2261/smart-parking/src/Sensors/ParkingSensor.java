package Sensors;

import Entities.ParkingSpace;
import Entities.ParkingSpace.ParkingStatus;
import Entities.Vehicle;

public interface ParkingSensor {
    public static final int NO_DETECTION = -1;
    public ParkingStatus getParkingStatus(ParkingSpace space);
    public boolean detectVehicle();
}
