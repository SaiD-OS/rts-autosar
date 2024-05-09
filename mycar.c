#include <stdio.h>
// #include "tp.h"
#include "tpl_os.h"

#define OBSTACLE_THRESHOLD 50
#define Car_width 2.5
#define Road_width 9
#define Road_Threshold 3 
#define min_lr_road 0.5 

float lr_sensor_threshold=2.5;
float sensor_data[9]={50,30,45,15,60,3,1,3,3};     //{5 elements of 5 sensors, 2 of left and right ,2 for offroading}
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
    for (int i = 0; i < 5; ++i) {
        if (sensor_data[i] < OBSTACLE_THRESHOLD&& sensor_data[i]<min) {
            min=sensor_data[i];
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


#define APP_Task_Brake_STOP_SEC_CODE
#include "tpl_memmap.h"


#define APP_Task_Accelerate_START_SEC_CODE
#include "tpl_memmap.h"

TASK(Accelerate) {                                // Task for acceleration

    // Check if any sensor detects an obstacle within threshold distance
  float min=50;
  
    for (int i = 0; i < 5; ++i) {
        if (sensor_data[i] < OBSTACLE_THRESHOLD&& sensor_data[i]<min) {
            min=sensor_data[i];
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
    

  TerminateTask();
}



#define APP_Task_Accelerate_STOP_SEC_CODE
#include "tpl_memmap.h"


#define APP_Task_Steer_START_SEC_CODE
#include "tpl_memmap.h"

TASK(Steer) {                                //MOST CRITICAL TASK=> To Steer

    // Check if any sensor detects an obstacle within threshold distance
  float min=50;
  int obstacle_sensor[5]={0,0,0,0,0};
    for (int i = 0; i < 5;i++) {
        if (sensor_data[i] < OBSTACLE_THRESHOLD) 
            obstacle_sensor[i]=1;
        }  
    for(int i=0;i<5;i++){
        printf("%d \r\n",obstacle_sensor[i]);               //coordinates of obstacles according to the received data from sensor array
    }
    
    if((obstacle_sensor[0]==0 && obstacle_sensor[1]==0 && obstacle_sensor[2]==0 && obstacle_sensor[3]==0 && obstacle_sensor[4]==0) || (obstacle_sensor[0]==1 && obstacle_sensor[1]==1 && obstacle_sensor[2]==1 && obstacle_sensor[3]==1 && obstacle_sensor[4]==1))
        To_Steer=0;         //Not to steer if there's no obstacles OR there are obstacles in all sensors

    else{                    //if obstacles are there then to steer

            //cases when obstacles are on either left or right front of car
            if(obstacle_sensor[0]==1 && obstacle_sensor[4]==0){                  // to steer right
                int i=3;
                int c=1;
                while(i>0){
                    if(obstacle_sensor[i]==0){
                        c++;
                        i--;
                    }
                    else if(obstacle_sensor[i]==1){
                    break;
                    }
                }
                
                    if(sensor_data[6]>=lr_sensor_threshold && sensor_data[8]>min_lr_road)
                    To_Steer=max_Right_Steer/c;
            }

            // //To steer left
            
            else if(obstacle_sensor[0]==0 && obstacle_sensor[4]==1){
                    int i=1;
                    int c=1;
                    while(i<4){
                        if(obstacle_sensor[i]==0){
                            c++;
                            i++;
                        }
                        else if(obstacle_sensor[i]==1){
                        break;
                        }
                    }
                        if(sensor_data[5]>=lr_sensor_threshold)
                        To_Steer=max_left_steer/c;
                    
            }
            
            //some critical cases when obstacle is in mid front of car 
            else if(obstacle_sensor[0]==0 && obstacle_sensor[4]==0)
            {
                if(obstacle_sensor[1]==0 && obstacle_sensor[2]==1 && obstacle_sensor[3]==0){    //checking left lane doesn't have car and if offroad then take right
                    if(sensor_data[5]>=lr_sensor_threshold && sensor_data[7]>min_lr_road)       //if this it will take left amon left/right         010
                    To_Steer=-7.5;
                    else if(sensor_data[6]>=lr_sensor_threshold && sensor_data[8]>min_lr_road)
                    To_Steer=7.5;
                }
                else if(obstacle_sensor[1]==1 && obstacle_sensor[2]==0 && obstacle_sensor[3]==1){       //checking left lane doesn't have car and if offroad then take right
                    if(sensor_data[5]>=lr_sensor_threshold && sensor_data[7]>min_lr_road)       //if this it will take left among left/right        101
                    To_Steer=-10;
                    else if(sensor_data[6]>=lr_sensor_threshold && sensor_data[8]>min_lr_road)      
                    To_Steer=10;
                }
                else if(obstacle_sensor[1]==1 && obstacle_sensor[2]==1 && obstacle_sensor[3]==1){       //checking left lane doesn't have car and if offroad then take right
                    if(sensor_data[5]>=lr_sensor_threshold && sensor_data[7]>min_lr_road)       //if this it will take left among left/right        101
                    To_Steer=-10;
                    else if(sensor_data[6]>=lr_sensor_threshold && sensor_data[8]>min_lr_road)      
                    To_Steer=10;
                }



                if(sensor_data[5]>=lr_sensor_threshold && sensor_data[7]>min_lr_road){                  //cases when taking left is the only option
                        if(obstacle_sensor[1]==0 && obstacle_sensor[2]==0)
                        To_Steer=-5;
                        else if(obstacle_sensor[1]==0 && obstacle_sensor[2]==1)
                        To_Steer=-7.5;
                }
                else if(sensor_data[5]<lr_sensor_threshold && sensor_data[6]>=lr_sensor_threshold && sensor_data[8]>min_lr_road)  //extreme right
                To_Steer=10;


                else if(sensor_data[6]>=lr_sensor_threshold && sensor_data[8]>min_lr_road){             //cases when taking right is the only option
                        if(obstacle_sensor[2]==0 && obstacle_sensor[3]==0)
                            To_Steer=5;
                        else if(obstacle_sensor[2]==1 && obstacle_sensor[3]==0)
                            To_Steer=7.5;
                }
                else if(sensor_data[6]<lr_sensor_threshold && sensor_data[5]>=lr_sensor_threshold && sensor_data[7]>min_lr_road)    //extreme left
                To_Steer=-10;

            }

//For taking care of edge of the road, car should not go offroad
            if(To_Steer<0 && (sensor_data[7]<=min_lr_road))     //while steering left, not going offroad
            To_Steer=0;
            else if(To_Steer>0 && (sensor_data[8]<=min_lr_road))    //while steering right, not going offroad
            To_Steer=0; 
             
        }

    printf("We have to steer this much : %f  \r\n",To_Steer);
   
 TerminateTask();
}


#define APP_Task_Steer_STOP_SEC_CODE
#include "tpl_memmap.h"




// #define OS_START_SEC_CODE
// #include "tpl_memmap.h"
// /*
//  * This is necessary for ST libraries
//  */
// FUNC(void, OS_CODE) assert_failed(uint8_t* file, uint32_t line)
// {
// }
// #define OS_STOP_SEC_CODE
// #include "tpl_memmap.h"

