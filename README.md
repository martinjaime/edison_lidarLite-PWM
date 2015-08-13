# edison_lidarLite-PWM

Designed for LIDAR-Lite v1 "Silver Label" but should work with LIDAR-Lite v2 using the same wiring (but add a 680uF capacitor because of the speed!). 

lidarLite_PWM_read.c reads the PWM output of PulsedLight's LidarLite using 
Intel's mraa Library and prints the distance in meters. 

Tested under Ubuntu. Compile with the following command: 

```
gcc lidarLite_PWM_read.c -lmraa -lrt
```
