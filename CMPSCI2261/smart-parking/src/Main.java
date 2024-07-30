import Entities.*;
import Managers.*;
import Sensors.*;
import Strategies.BasicParkingStrategy;

import java.awt.*;
import java.time.Duration;
import java.time.LocalTime;
import java.util.Scanner;

public class Main {

    ParkingLot parkingLot;
    Scanner input = new Scanner(System.in);

    public static void main(String args[]) {
        Main mainObj = new Main();
        mainObj.initialize();
        mainObj.displayMainMenu();
    }

    public void initialize() {
        parkingLot = new ParkingLot(LocalTime.of(7, 30), LocalTime.of(9, 30), 5, 0);
        parkingLot.initLot(5, 8);
        parkingLot.refreshOccupationStats();
        for(ParkingSpace space : parkingLot.getAvailableSpaces()) {
            if(space.getLocation()[0] < 2 || space.getLocation()[0] > 5) {
                space.setSpaceType(ParkingSpace.SpaceType.SPACE_COMPACT);
            }
        }

        for(ParkingSpace space : parkingLot.getParkingSpaces().get(0)) {
            space.setSpaceType(ParkingSpace.SpaceType.SPACE_HANDICAP);
        }


        Dimension cameraRes = new Dimension(1920, 1080);
        CameraSensor cameraSensor = new CameraSensor("CameraPro", "BestCam", "AFI8204", "ZF!Re", cameraRes, 29.97f);
        cameraSensor.setPosition(4, 8);
        parkingLot.addSensor(cameraSensor);
        cameraSensor = new CameraSensor("CameraPro", "BestCam", "AER8442", "ZF!Re", cameraRes, 29.97f);
        cameraSensor.setPosition(10, 6);
        UltrasonicSensor ultraSensor = new UltrasonicSensor("SoundBoss", "BestCam", "UST2917", "ZF!Re");
        ultraSensor.setPosition(14, 3);
        ultraSensor.setDistanceThreshold(7);
    }

    public void displayMainMenu() {
        System.out.println("------- SMART PARK ------");
        System.out.println("Choose an option: ");
        System.out.println("1. View Available Spots");
        System.out.println("2. Reserve a Spot");
        System.out.println("3. Manage Reservations");
        System.out.println("4. Park Vehicle");
        System.out.println("5. Exit Lot");
        System.out.println("6. Log Out");

        int option = input.nextInt();

            switch (option) {
                case 1:
                    displayViewMenu();
                    break;
                case 2:
                    displayNewReservationMenu();
                    break;
                case 3:
                    displayReservationsMenu();
                    break;
                case 4:
                    displayParkMenu();
                    break;
                case 5:
                    displayExitMenu();
                    break;
                case 6:
                    displayLogoutMessage();
                    break;
                default:
                    System.out.println("Sorry, that's not a valid option.");
                    break;
            }
    }

    private void displayLogoutMessage() {
        System.out.println("Thank you, we hope to see you again soon!");
        System.exit(0);
    }

    private void displayExitMenu() {
        parkingLot.refreshOccupationStats();
        ParkingSpace occupiedSpace = null;
        while(occupiedSpace == null) {
            System.out.println("Please enter your space number: ");
            String spaceID = input.next();
            for(ParkingSpace space : parkingLot.getOccupiedSpaces()) {
                if(space.getSpaceID().equals(spaceID)) {
                    occupiedSpace = space;
                }
            }
            if(occupiedSpace != null) break;
            System.out.println("That space either does not exist or is currently marked as open.");
            System.out.println("Please make sure you put in the correct space number");
        }
        occupiedSpace.emptySpace();
        System.out.println("We hope you enjoyed your time!");
    }

    private void displayParkMenu() {
        parkingLot.refreshOccupationStats();
        System.out.println("====== PARKING ======");
        System.out.println("Please enter your license plate number: ");
        String plateNum = input.next();
        System.out.println("Please enter your vehicle type: ");
        String vehicleType = input.next();
        System.out.println("Please choose a space type: ");
        System.out.println("1. Standard");
        System.out.println("2. Compact");
        System.out.println("3. Handicap");
        int option = input.nextInt();
        ParkingSpace space = null;
        while(space == null) {
            switch(option) {
                case 1:
                    space = BasicParkingStrategy.findBestSpace(parkingLot, ParkingSpace.SpaceType.SPACE_STANDARD);
                    break;
                case 2:
                    space = BasicParkingStrategy.findBestSpace(parkingLot, ParkingSpace.SpaceType.SPACE_COMPACT);
                    break;
                case 3:
                    space = BasicParkingStrategy.findBestSpace(parkingLot, ParkingSpace.SpaceType.SPACE_HANDICAP);
                    break;
                default:
                    System.out.println("Sorry, that's not a valid option.");
                    break;
            }
        }
        System.out.println("Please park in space " + space.getSpaceID());
        space.parkVehicle(new Vehicle(plateNum, vehicleType));
        System.out.println("Successfully recorded parking");
        displayMainMenu();
    }

    private void displayReservationsMenu() {
        System.out.println("====== MANAGE RESERVATIONS ======");
        System.out.println("Please enter your license plate number: ");
        String plateNum = input.next();
        Reservation reservation = null;
        try {
            reservation = ReservationManager.getReservationFromLicensePlate(plateNum);
        } catch (Exception e) {
            System.err.println(e.getMessage());
        }

        if(reservation == null) {
            displayMainMenu();
        }
        System.out.println("You have a reservation in spot " + reservation.getReservedSpace().getSpaceID() + " for " + reservation.getDuration().toHours() + " hours");
        System.out.println("Would you like to cancel? (y/n)");
        String choice = input.next().toLowerCase();
        while(!choice.equals("y") && !choice.equals("n")) {
            System.out.println("Invalid Option");
            choice = input.next().toLowerCase();
        }
        if(choice.equals("y")) {
            reservation.cancel();
            System.out.println("Reservation cancelled");
        }

        displayMainMenu();
    }

    public void displayNewReservationMenu() {
        parkingLot.refreshOccupationStats();
        String plateNum = "";
        System.out.println("====== NEW RESERVATION ======");
        System.out.println("Please enter license plate number: ");
        while(plateNum.equals("")) {
            plateNum = input.nextLine();
        }
        System.out.println("Please enter vehicle type: ");
        String vehicleType = input.nextLine();
        Vehicle vehicle = new Vehicle(plateNum, vehicleType);
        System.out.println("Please choose a space type: ");
        System.out.println("1. Standard");
        System.out.println("2. Compact");
        System.out.println("3. Handicap");
        int option = input.nextInt();
        ParkingSpace space = null;
        while(space == null) {
            switch(option) {
            case 1:
                space = BasicParkingStrategy.findBestSpace(parkingLot, ParkingSpace.SpaceType.SPACE_STANDARD);
                break;
            case 2:
                space = BasicParkingStrategy.findBestSpace(parkingLot, ParkingSpace.SpaceType.SPACE_COMPACT);
                break;
            case 3:
                space = BasicParkingStrategy.findBestSpace(parkingLot, ParkingSpace.SpaceType.SPACE_HANDICAP);
                break;
            default:
                System.out.println("Sorry, that's not a valid option.");
                break;
            }
        }
        System.out.println("Please enter how long you would like to reserve your spot to the nearest hour: ");
        int hours = input.nextInt();
        Duration reservationTime = Duration.ofHours(hours);
        NotificationManager.notifyReservationConfirmation(ReservationManager.makeReservation(vehicle, space, reservationTime));
        displayMainMenu();
    }

    public void displayViewMenu() {
        parkingLot.refreshOccupationStats();
        System.out.println("====== VIEW AVAILABLE SPACES ======");
        System.out.println("Please choose a space type or press 0 to quit: ");
        System.out.println("1. Standard");
        System.out.println("2. Compact");
        System.out.println("3. Handicap");
        System.out.println("0. Back");
        ParkingSpace.SpaceType spaceType = null;
        int option = input.nextInt();
        while (spaceType == null) {
            switch (option) {
                case 1:
                    spaceType = ParkingSpace.SpaceType.SPACE_STANDARD;
                    break;
                case 2:
                    spaceType = ParkingSpace.SpaceType.SPACE_COMPACT;
                    break;
                case 3:
                    spaceType = ParkingSpace.SpaceType.SPACE_HANDICAP;
                    break;
                case 0:
                    displayMainMenu();
                    break;
                default:
                    System.out.println("Sorry, that's not a valid option.");
                    break;
            }
        }
        for (ParkingSpace space : parkingLot.getAvailableSpaces()) {
            if (space.getSpaceType() == spaceType) {
                System.out.println("Space " + space.getSpaceID());
            }
        }
        displayMainMenu();
    }
}

