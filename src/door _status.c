#include "door_status.h"


// In your System_Logic_Task ,return true when the RFID is not ture
u8 _Door_Status(u8 RFID_statue){
    float yaw_rate = MPU6050_Get_Yaw_Rate();
    
    // Simple logic: if the rotation speed is higher than 30 deg/s, 
    // we consider the door is being opened.
    if (fabs(yaw_rate) > 60.0f) { 
        if (RFID_statue) {
            return 1;
        } else {
            return 0;
            // Illegal Open - TRIGGER ALARM!
        }
    }
    return 0;
}

