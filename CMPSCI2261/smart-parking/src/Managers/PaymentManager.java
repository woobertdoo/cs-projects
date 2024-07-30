package Managers;

import java.time.Duration;
import java.util.HashMap;

public class PaymentManager {
    private static final HashMap<String, Float> vehiclePrices = new HashMap<String, Float>();

    public static float calculateParkingFee(Duration duration, String vehicleType) {
        if(!vehiclePrices.containsKey("car")) {
            addVehiclePrice("car", 15.00f);
        }

        if(!vehiclePrices.containsKey(vehicleType)){
            System.err.println("Error retrieving key " + vehicleType + " from vehicle price map.");
            System.err.println("Defaulting to price of car");
            return duration.toHours() * vehiclePrices.get("car");
        }
        return duration.toHours() * vehiclePrices.get(vehicleType);
    }

    public static void addVehiclePrice(String vehicleType, float price) {
        vehiclePrices.put(vehicleType.toLowerCase(), price);
    }

    public static void updateVehiclePrice(String vehicleType, float price) {
        if(!vehiclePrices.containsKey(vehicleType)) {
            addVehiclePrice(vehicleType, price);
            return;
        }

        vehiclePrices.replace(vehicleType, price);
    }

    public static String processPayment(float amount, String paymentMethod) {
        return "Payment for $" + amount + " paid for by " + paymentMethod + "\n";
    }
}
