package Managers;

import Entities.ParkingSpace;
import Entities.Reservation;

public class NotificationManager {
    public static void sendNotification(String message, String recipient) {
        System.out.println(recipient + ": " + message);
    }

    public static void notifyParkingAvailable(ParkingSpace parkingSpace) {
        try {
            System.out.println("Parking space " + parkingSpace.getSpaceID() + " now available.");
        } catch (Exception e) {
            System.err.println(e.getMessage());
        }
    }

    public static void notifyReservationConfirmation(Reservation reservation) {
        try {
            System.out.println("Reservation for " + reservation.getReservedSpace().getSpaceID() + " confirmed");
        } catch (Exception e) {
            System.err.println(e.getMessage());
        }
    }
}
