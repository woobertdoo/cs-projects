package Entities;

import java.awt.*;
import java.time.Duration;
import java.time.LocalDateTime;
import java.time.LocalTime;

public class Vehicle {

    private String plateNumber = "";
    private String vehicleClass = "";
    private final LocalDateTime entryTime = LocalDateTime.now();
    private LocalDateTime exitTime;

    public Vehicle(String plateNumber, String vehicleClass) {
        this.plateNumber = plateNumber;
        this.vehicleClass = vehicleClass;
    }

    public String getPlateNumber() {
        return plateNumber;
    }

    public String getVehicleClass() {
        return vehicleClass;
    }

    public LocalDateTime getEntryTime() {
        return entryTime;
    }

    public LocalDateTime getExitTime() {
        return exitTime;
    }

    public void setExitTime(LocalDateTime time) {
        exitTime = time;
    }

    public Duration calculateParkingDuration() {
        return Duration.between(entryTime, exitTime);
    }
}
