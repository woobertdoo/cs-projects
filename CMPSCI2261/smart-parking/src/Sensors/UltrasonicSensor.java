package Sensors;

import Entities.*;
import Entities.ParkingSpace.ParkingStatus;

import java.awt.Dimension;
import java.util.ArrayList;
import java.util.List;

public class UltrasonicSensor implements ParkingSensor {
    private int distanceThreshold = 200; // maximum distance in feet that the sensor can detect
    private int posX, posY;
    private String model;
    private String manufacturer;
    private String serial;
    private String accessPIN;
    private ArrayList<ParkingSpace> detectableSpaces = new ArrayList<ParkingSpace>();
    private boolean active = true;


    public UltrasonicSensor(String model, String manufacturer, String serial, String accessPIN) {
        this.model = model;
        this.manufacturer = manufacturer;
        this.serial = serial;
        this.accessPIN = accessPIN;
    }

    public int getDistanceThreshold() {
        return this.distanceThreshold;
    }

    public void setDistanceThreshold(int thresh) {
        this.distanceThreshold = thresh;
    }

    public void setPosition(int x, int y) {
        this.posX = x;
        this.posY = y;
    }

    private void initSurveyArea(ParkingLot lot) {
        for(ArrayList<ParkingSpace> row : lot.getParkingSpaces()) {
            detectableSpaces.addAll(row.stream().filter(space -> {
                int distance = distanceThreshold + 1;
                try {
                    distance =   distanceFrom(space);
                } catch (Exception e) {
                    System.err.println(e.getMessage());
                }
                return distance <= distanceThreshold;
            }).toList());

        }
    }

    @Override
    public ParkingStatus getParkingStatus(ParkingSpace space) {
        return space.getParkingStatus();
    }

    @Override
    public boolean detectVehicle() {
        return measureDistance() > 0;
    }

    public int measureDistance() {
        List<ParkingSpace> occupiedSpacesInRange = detectableSpaces.stream()
                .filter(space -> space.getParkingStatus() == ParkingStatus.PK_STATUS_OCCUPIED)
                .toList();

        ParkingSpace closestOccupied = occupiedSpacesInRange.stream().reduce((space1, space2) -> {
            ParkingSpace res = space1;
            res =distanceFrom(space1) < distanceFrom(space2) ? space1 : space2;
            return res;
        }).get();

        try {
            return distanceFrom(closestOccupied);
        } catch(Exception e) {
            System.err.println(e.getMessage());
        }
        return ParkingSensor.NO_DETECTION;
    }

    private int distanceFrom(ParkingSpace space) {
        int[] spaceLocation = space.getLocation();
        int xDiff = Math.abs(posX - spaceLocation[0]);
        int yDiff = Math.abs(posY - spaceLocation[1]);
        return (int)Math.floor(Math.sqrt(Math.pow(xDiff, 2) + Math.pow(yDiff, 2)));
    }



    public void powerOn() {
        active = true;
    }

    public void powerOff() {
        active = false;
    }

    public boolean isActive() {
        return active;
    }
}

