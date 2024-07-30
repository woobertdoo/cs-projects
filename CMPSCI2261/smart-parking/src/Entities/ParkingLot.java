package Entities;


import java.lang.reflect.Array;
import java.time.Duration;
import java.time.LocalDateTime;
import java.time.LocalTime;
import java.util.ArrayList;
import java.util.List;

import Entities.ParkingSpace.ParkingStatus;
import Entities.ParkingSpace.ReservationStatus;
import Managers.ReservationManager;
import Sensors.ParkingSensor;

public class ParkingLot {
    private final ArrayList<ArrayList<ParkingSpace>> parkingSpaces;
    private LocalTime openHour, closeHour;
    private int totalSpaces;
    private final ArrayList<ParkingSpace> availableSpaces = new ArrayList<ParkingSpace>();
    private final ArrayList<ParkingSpace> occupiedSpaces = new ArrayList<ParkingSpace>();
    private final ArrayList<ParkingSensor> sensors = new ArrayList<ParkingSensor>();
    private final int entranceX;
    private final int entranceY;

    public ParkingLot(LocalTime openHour, LocalTime closeHour, int entranceX, int entranceY) {
        this.openHour = openHour;
        this.closeHour = closeHour;
        this.entranceX = entranceX;
        this.entranceY = entranceY;
        this.parkingSpaces = new ArrayList<ArrayList<ParkingSpace>>();
    }

    public void initLot(int rows, int cols) {
        this.totalSpaces = rows * cols;
        for(int i = 0; i < rows; i++) {
            String spaceIDStart = getRowIdentifier(i);
            ArrayList<ParkingSpace> row = new ArrayList<ParkingSpace>();
            for(int j = 0; j < cols; j++) {
                String spaceID = spaceIDStart + j;
                row.add(new ParkingSpace(spaceID, j, i));
            }
            parkingSpaces.add(row);
        }

    }

    public ArrayList<ParkingSensor> getSensors() {
        return sensors;
    }
    public void addSensor(ParkingSensor sensor) {
        this.sensors.add(sensor);
    }

    public void refreshOccupationStats() {
        availableSpaces.clear();
        occupiedSpaces.clear();
        for(ArrayList<ParkingSpace> row : parkingSpaces) {
                for(ParkingSpace space : row) {
                    if(space.getParkingStatus() == ParkingStatus.PK_STATUS_OCCUPIED) {
                        occupiedSpaces.add(space);
                    } else if(space.getReservationStatus() == ReservationStatus.RV_STATUS_AVAILABLE) {
                        availableSpaces.add(space);
                    }
                }
        }
    }

    public ParkingSpace getClosestSpaceToEntrance(List<ParkingSpace> spaces){
        ParkingSpace closestSpace = spaces.get(0);
        int closestDist = Integer.MAX_VALUE;
        for (ParkingSpace space : spaces) {
            int xDist = Math.abs(entranceX - space.getLocation()[0]);
            int yDist = Math.abs(entranceY - space.getLocation()[1]);
            int spaceDist = (int) Math.sqrt(Math.pow(xDist, 2) + Math.pow(yDist, 2));
            if(spaceDist < closestDist) {
                closestDist = spaceDist;
                closestSpace = space;
            }
        }
        return closestSpace;
    }

    /*
    *  Used to turn the row number into one or more uppercase letters.
    *  Designed to facilitate any size parking lot, even those with more than 26 rows
    * */
    private String getRowIdentifier(int rowNum) {
        String rowIdentifier  = "";
        do {
            rowIdentifier = rowIdentifier.concat(String.valueOf((char) (rowNum % 26 + 65)));
            rowNum /= 26;
        } while(rowNum / 26 > 0);

        return rowIdentifier;
    }

    public LocalTime[] getOperationTimeRange() {
        return new LocalTime[]{ this.openHour, this.closeHour };
    }

    public ArrayList<ArrayList<ParkingSpace>> getParkingSpaces() {
        return parkingSpaces;
    }

    public ArrayList<ParkingSpace> getAvailableSpaces() {
        return availableSpaces;
    }

    public ArrayList<ParkingSpace> getOccupiedSpaces() {
        return occupiedSpaces;
    }

    public void setOperationTime(LocalTime openHour, LocalTime closeHour) {
        this.openHour = openHour;
        this.closeHour = closeHour;
    }

    public int getTotalSpaces() {
        return totalSpaces;
    }
}

