#include "tp.h"
#include "tpl_os.h"

#define APP_Task_Accelerate_START_SEC_CODE
#include "tpl_memmap.h"
FUNC(int, OS_APPL_CODE) main(void)
{
  initBoard();
  StartOS(OSDEFAULTAPPMODE);
  return 0;
}
// data should be passed by random generator in which is the sensors are giving the output and by this data we will actuate the via function
// same data structure will be shared among the sensors and the functions and will be updated after time intervals

int sensor_data[5] = {60, 55, 35, 45, 48}; // Example sensor data, representing obstacle distances
int acceleration=30;

TASK(Task1) {
  // Task for acceleration

    // Check if any sensor detects an obstacle within threshold distance
    bool obstacle_detected = false;
    for (int i = 0; i < 5; ++i) {
        if (sensor_data[i] < OBSTACLE_THRESHOLD) {
            obstacle_detected = true;
            acceleration=0;
            //printf("obstacle detected. not accelaerating")
        }
    }

    // If no obstacle detected, accelerate
    if (!obstacle_detected) {
        acceleration=30;    // Acceleration code
        //printf("No obstacle detected. Accelerating...\n");
    }
  TerminateTask();
}
#define APP_Task_Accelerate_STOP_SEC_CODE
#include "tpl_memmap.h"

#define APP_Task_Brake_START_SEC_CODE
#include "tpl_memmap.h"
TASK(Brake) {
   // Task for braking
    // Find the closest obstacle distance
    int closest_obstacle_distance = sensor_data[0];
    for (int i = 1; i < 5; ++i) {
        if (sensor_data[i] < closest_obstacle_distance) {
            closest_obstacle_distance = sensor_data[i];
        }
    }

    // Calculate brake sensitivity based on the closest obstacle distance
    int brake_sensitivity = (OBSTACLE_THRESHOLD * MAX_BRAKE_SENSITIVITY) / closest_obstacle_distance;
    if (brake_sensitivity > MAX_BRAKE_SENSITIVITY) {
        brake_sensitivity = MAX_BRAKE_SENSITIVITY; // Ensure sensitivity doesn't exceed the maximum
    }

    // Apply brakes with calculated sensitivity
    //printf("Applying brakes with %d%% sensitivity due to obstacle at %d meters.\n", brake_sensitivity, closest_obstacle_distance);
TerminateTask();
}
#define APP_Task_Brake_STOP_SEC_CODE
#include "tpl_memmap.h"

#define OS_START_SEC_CODE
#include "tpl_memmap.h"
/*
 * This is necessary for ST libraries
 */
FUNC(void, OS_CODE) assert_failed(uint8_t* file, uint32_t line)
{
}
#define OS_STOP_SEC_CODE
#include "tpl_memmap.h"





