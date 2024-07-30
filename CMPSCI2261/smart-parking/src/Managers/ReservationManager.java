package Managers;

import Entities.ParkingSpace;
import Entities.ParkingSpace.ReservationStatus;
import Entities.Vehicle;
import Entities.Reservation;

import java.time.Duration;
import java.time.LocalDateTime;
import java.util.ArrayList;

public class ReservationManager {
    static ArrayList<Reservation> reservations = new ArrayList<>();

    public static Reservation makeReservation(Vehicle vehicle, ParkingSpace space, Duration duration) {
        Reservation reservation = new Reservation(vehicle, space,duration);
        reservations.add(reservation);
        return  reservation;
    }

    public static void cancelReservation(Reservation res) {
        res.cancel();
    }

    public static Vehicle getReservedVehicle(Reservation res) {
        return res.getReservedVehicle();
    }

    public static ParkingSpace getReservedSpace(Reservation res) {
        return res.getReservedSpace();
    }

    public static Reservation getReservationFromLicensePlate(String plateNumber) throws ReservationNotFoundException {
        for (Reservation reservation : reservations) {
            if (reservation.getReservedVehicle().getPlateNumber().equals(plateNumber) && reservation.isActive()) {
                return reservation;
            }
        }

        throw new ReservationNotFoundException("That license plate does not have a reservation");
    }
}

class ReservationNotFoundException extends Exception {
    public ReservationNotFoundException(String message) {
        super(message);
    }
}
