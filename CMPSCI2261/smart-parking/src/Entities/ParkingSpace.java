package Entities;

import java.time.LocalDateTime;


public class ParkingSpace {

    public enum ParkingStatus {
        PK_STATUS_OCCUPIED,
        PK_STATUS_VACANT,
    }

    public enum ReservationStatus {
        RV_STATUS_RESERVED,
        RV_STATUS_AVAILABLE
    }

    public enum SpaceType {
        SPACE_COMPACT,
        SPACE_STANDARD,
        SPACE_HANDICAP,
        SPACE_DOUBLE
    }

    private String spaceID = "";
    private int gridX = -1, gridY = -1;
    private Vehicle occupant;
    private SpaceType spaceType = SpaceType.SPACE_STANDARD;
    private ParkingStatus pkStatus = ParkingStatus.PK_STATUS_VACANT;
    private ReservationStatus rvStatus = ReservationStatus.RV_STATUS_AVAILABLE;
    private LocalDateTime reservationExpiration;

    public ParkingSpace(String spaceID, int gridX, int gridY) {
        this.spaceID = spaceID;
        this.gridX = gridX;
        this.gridY = gridY;
    }

    public void setSpaceType(SpaceType spaceType) {
        this.spaceType = spaceType;
    }

    public SpaceType getSpaceType() {
        return this.spaceType;
    }

    public int[] getLocation() {
        return new int[]{gridX, gridY};
    }

    public ParkingStatus getParkingStatus() {
        return this.pkStatus;
    }

    private void setParkingStatus(ParkingStatus status) {
        this.pkStatus = status;
    }

    public ReservationStatus getReservationStatus() {
        return this.rvStatus;
    }

    public Vehicle getOccupant() {
        return this.occupant;
    }

    public void parkVehicle(Vehicle occupant) {
        this.occupant = occupant;
        setParkingStatus(ParkingStatus.PK_STATUS_OCCUPIED);
    }

    public void emptySpace() {
        this.occupant = new Vehicle("", "");
        setParkingStatus(ParkingStatus.PK_STATUS_VACANT);
    }

    public String getSpaceID() {

        return this.spaceID;
    }

    public void reserveSpace(LocalDateTime reservationExpiration) {
        this.reservationExpiration = reservationExpiration;
        this.rvStatus = ReservationStatus.RV_STATUS_RESERVED;
    }

    public void cancelReservation() {
        rvStatus = ReservationStatus.RV_STATUS_AVAILABLE;
        this.reservationExpiration = null;
    }
}
