#include <stdio.h>
// #include "tp.h"
#include "tpl_os.h"

#define OBSTACLE_THRESHOLD 50
#define Car_width 2.5
#define Road_width 9
#define Road_Threshold 3 
#define min_lr_road 0.5 

float lr_sensor_threshold=2.5;
float sensor_data[][9]={{50,30,40,15,60,1,3,3,3.5}};     //{5 elements of 5 sensors, 2 of left and right ,2 for offroading}
float To_Steer=0;
float max_left_steer=-10;
float max_Right_Steer=10;

float braking=0;
float max_braking=10;
float acceleration=10;
int max_aceleration=10;

#define APP_Task_Brake_START_SEC_CODE
#include "tpl_memmap.h"
int main(void)
{
  // initBoard();
  StartOS(OSDEFAULTAPPMODE);
  return 0;
}

TASK(Brake) {                                // Task for acceleration

    // Check if any sensor detects an obstacle within threshold distance
  printf("Hello you are now in the Automated car\r\n");
  float min=50;
  for(int j=0;j<1;j++){
    for (int i = 0; i < 5; ++i) {
        if (sensor_data[j][i] < OBSTACLE_THRESHOLD&& sensor_data[j][i]<min) {
            min=sensor_data[j][i];
        }
    }
    printf("closest obstacle is %f units far \r\n",min);

    if(min<10){
      braking=10.0F;
    }
    else{    
    braking= max_braking-((min*max_braking)/OBSTACLE_THRESHOLD);
    if (braking > max_braking) {
        braking = max_braking; // Ensure sensitivity doesn't exceed the maximum
    }


    printf("Braking sensitivity is : %f  \r\n",braking);
    }

  TerminateTask();
}
}


#define APP_Task_Brake_STOP_SEC_CODE
#include "tpl_memmap.h"

#define APP_Task_Accelerate_START_SEC_CODE
#include "tpl_memmap.h"

TASK(Accelerate) {                                // Task for acceleration

    // Check if any sensor detects an obstacle within threshold distance
  float min=50;
  for(int j=0;j<1;j++){
    for (int i = 0; i < 5; ++i) {
        if (sensor_data[j][i] < OBSTACLE_THRESHOLD&& sensor_data[j][i]<min) {
            min=sensor_data[j][i];
        }
    }
    if(min<10){
      acceleration=0.0F;
    }
    else{    
    acceleration=(max_aceleration*min)/OBSTACLE_THRESHOLD;    // if float value is allowed for the acceleration then it will work
    }

     if (acceleration > max_aceleration) {
        acceleration = max_aceleration; // Ensure sensitivity doesn't exceed the maximum
    }
    printf("Accelerationn intensity is : %f  \r\n",acceleration);
    }

  TerminateTask();
}



#define APP_Task_Accelerate_STOP_SEC_CODE
#include "tpl_memmap.h"

#define APP_Task_Brake_START_SEC_CODE
#include "tpl_memmap.h"

TASK(Steer) {                                // Task for acceleration

    // Check if any sensor detects an obstacle within threshold distance
  float min=50;
  int obstacle_sensor[5]={0,0,0,0,0};
  for(int j=0;j<1;j++){
    for (int i = 0; i < 5; ++i) {
        if (sensor_data[j][i] < OBSTACLE_THRESHOLD) 
            obstacle_sensor[i]=1;
        }  
    }
    if((obstacle_sensor[0]==0 && obstacle_sensor[1]==0 && obstacle_sensor[2]==0 && obstacle_sensor[3]==0 && obstacle_sensor[4]==0) || (obstacle_sensor[0]==1 && obstacle_sensor[1]==1 && obstacle_sensor[2]==1 && obstacle_sensor[3]==1 && obstacle_sensor[4]==1))
    To_Steer=0;

    // to steer right
    else if(sensor_data[0][0]==1&&sensor_data[0][4]==0){
    int i=3;
    int c=1;
    while(i>0){
        if(sensor_data[0][i]==0){
            c++;
            i--;
        }
        else{
        break;
        }
    }
    
        if(sensor_data[0][6]>=lr_sensor_threshold && sensor_data[0][8]>min_lr_road)
        To_Steer=max_Right_Steer/c;
    
    }

    //To steer left
    else if(sensor_data[0][4]==1 && sensor_data[0][0]==0){
    int i=1;
    int c=1;
    while(i<4){
        if(sensor_data[0][i]==0){
            c++;
        }
        else{
        break;
        }
    }
        if(sensor_data[0][5]>=lr_sensor_threshold && sensor_data[0][7]>min_lr_road)
        To_Steer=max_left_steer/c;
    
    }
    
else if(obstacle_sensor[0]==0 && obstacle_sensor[4]==0)
    {
        if(sensor_data[0][5]>=lr_sensor_threshold && sensor_data[0][7]>min_lr_road)
        To_Steer=-10;
        else if(sensor_data[0][6]>=lr_sensor_threshold && sensor_data[0][8]>min_lr_road)
        To_Steer=10;

    }

    //For edge of the road case
    if(To_Steer!=0 &&  (sensor_data[0][7]<=min_lr_road ||sensor_data[0][8]<=min_lr_road))
    To_Steer=0; 

    printf("steering senstivity : %f  \r\n",To_Steer);
    

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

