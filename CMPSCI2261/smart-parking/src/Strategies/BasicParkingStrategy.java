package Strategies;

import Entities.ParkingSpace;
import Entities.ParkingLot;
import Entities.ParkingSpace.SpaceType;

import java.util.List;

public class BasicParkingStrategy {
    public static ParkingSpace findBestSpace(ParkingLot lot, SpaceType spaceType) {
        List<ParkingSpace> sizeAppropriateSpaces = lot.getAvailableSpaces().stream().filter(space -> space.getSpaceType() == spaceType).toList();
        ParkingSpace bestSpace = sizeAppropriateSpaces.get(0);

        try {
            bestSpace = lot.getClosestSpaceToEntrance(sizeAppropriateSpaces);
        } catch(Exception e) {
            System.err.println(e.getMessage());
        }
       return bestSpace;
    }
}
