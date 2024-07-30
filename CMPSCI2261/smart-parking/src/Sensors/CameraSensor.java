package Sensors;

import Entities.ParkingLot;
import Entities.ParkingSpace;
import Entities.ParkingSpace.ParkingStatus;

import java.awt.Dimension;
import java.util.ArrayList;
import java.util.List;

public class CameraSensor implements ParkingSensor {
    private String model;
    private String manufacturer;
    private String serial;
    private String accessPIN;
    private Dimension resolution;
    private int posX, posY;
    private int fieldSize;
    private float frameRate;
    private boolean active = true;
    private ArrayList<ParkingSpace> detectableSpaces = new ArrayList<ParkingSpace>();

    public CameraSensor(String model, String manufacturer, String serial, String accessPIN, Dimension resolution, float frameRate) {
        this.model = model;
        this.manufacturer = manufacturer;
        this.serial = serial;
        this.accessPIN = accessPIN;
        this.resolution = resolution;
        this.frameRate = frameRate;
    }

    public void setFieldSize(int fieldSize) {
        this.fieldSize = fieldSize;
    }

    public void initSurveyArea(ParkingLot lot) {

        for(ArrayList<ParkingSpace> row : lot.getParkingSpaces()) {
            for(ParkingSpace space : row) {
                try {
                    boolean inRange = isInRange(space);
                    if (inRange) detectableSpaces.add(space);
                } catch (Exception e) {
                        System.err.println(e.getMessage());
                }
            }
        }
    }

    private List<ParkingSpace> getOccupiedSpaces() {
        return detectableSpaces.stream().filter(space -> space.getParkingStatus() == ParkingStatus.PK_STATUS_OCCUPIED).toList();
    }

    public void setPosition(int x, int y) {
        this.posX = x;
        this.posY = y;
    }

    public void captureImage() {
        System.out.println("Image Captured by camera " + this.serial);
    }

    public void processImage() {
        List<ParkingSpace> occupiedSpaces = getOccupiedSpaces();
        for(ParkingSpace space : occupiedSpaces) {
            String spaceID = "";
            try {
                spaceID = space.getSpaceID();
            } catch (Exception e) {
                System.err.println(e.getMessage());
            }
            System.out.println("License Plate " + space.getOccupant().getPlateNumber() + " parked in space " + spaceID);
        }
    }

    private boolean isInRange(ParkingSpace space) throws Exception {
        int xMin = posX - fieldSize;
        int xMax = posX + fieldSize;
        int yMin = posY - fieldSize;
        int yMax = posY + fieldSize;
        if(space.getLocation()[0] < xMin || space.getLocation()[0] > xMax) return false;
        return space.getLocation()[1] >= yMin && space.getLocation()[1] <= yMax;
    }

    @Override
    public boolean detectVehicle() {
        for(ParkingSpace space : detectableSpaces) {
            if (space.getParkingStatus() == ParkingStatus.PK_STATUS_OCCUPIED) return true;
        }
        return false;
    }

    @Override
    public ParkingStatus getParkingStatus(ParkingSpace space) {
        return space.getParkingStatus();
    }
}




