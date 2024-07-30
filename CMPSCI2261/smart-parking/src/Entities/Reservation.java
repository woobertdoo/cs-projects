package Entities;

import java.time.Duration;

public class Reservation {
    boolean active = false;
    Duration duration;
    Vehicle vehicle;
    ParkingSpace space;

    public Reservation(Vehicle vehicle, ParkingSpace space, Duration duration) {
        this.duration = duration;
        this.vehicle = vehicle;
        this.space = space;
        active = true;
    }

    public Vehicle getReservedVehicle() {
        return vehicle;
    }

    public ParkingSpace getReservedSpace() {
        return space;
    }

    public Duration getDuration() {
        return duration;
    }

    public boolean isActive() {
        return active;
    }

    public void cancel() {
        this.active = false;
    }
}
