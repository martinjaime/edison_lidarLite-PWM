/*
 * lidarLite_PWM_read.c will read the PWM output of PulsedLight's LidarLite.
 * 
 * Copyright (C) 2015 Martin Jaime
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <err.h>
#include <stdbool.h>

#include "mraa.h"

#define GPIO14 36
#define GPIO15 48
#define HIGH 1
#define LOW  0 
#define OFFSET 0.165349936

bool keepRunning = true;

double pulseIn(mraa_gpio_context, int, double);
void intHandler(int);
double duration(struct timespec, struct timespec);

int main()
{

    double pulsewidth;
    float length;
    mraa_gpio_context lidar_pin;
    mraa_gpio_context power_en_pin;
    mraa_result_print(mraa_init());

    /* Initiate lidar_pin GPIO pin */ 
    lidar_pin = mraa_gpio_init(GPIO14);
    if (lidar_pin == NULL)
        err(1, "mraa_gpio_init returned NULL for GPIO14");
    /* Set lidar_pin GPIO direction */ 
    mraa_gpio_dir(lidar_pin, MRAA_GPIO_IN);

    /* Initiate power_en_pin GPIO pi */ 
    power_en_pin = mraa_gpio_init(GPIO15);
    if (power_en_pin == NULL)
        err(1, "mraa_gpio_init returned NULL for GPIO15");
    /* Set power_en_pin GPIO direction */ 
    mraa_gpio_dir(power_en_pin, MRAA_GPIO_IN);

    /* Enable LidarLite */ 
    mraa_gpio_write(power_en_pin, HIGH);

    /* Listen for Ctrl-C signal */ 
    if (SIG_ERR == signal(SIGINT, intHandler))
        err(1, "signal returned an error");

    /* Main loop */
    while(keepRunning)
    {
        sleep(1);

        pulsewidth = pulseIn(lidar_pin, HIGH, 1.0);
        if (pulsewidth > 0)
        {
            length = (pulsewidth * 1E6) / 1000.0;
            length -= OFFSET;
            printf(">>> Length: %f <<<\n", length);
        }
        else /* There was an error. Reinitiate LidarLite */ 
        {
            mraa_gpio_write(power_en_pin, LOW); /* Turn off sensor */ 
            usleep(1000); 
            mraa_gpio_write(power_en_pin, HIGH); /* Turn on sensor */ 
            usleep(1000); /* Wait for power up  */ 
        }
    }

    /* Clean up */ 
    mraa_gpio_isr_exit(lidar_pin);
    mraa_gpio_close(lidar_pin);
    mraa_deinit();

    printf("Terminating\n");

    return 0;
}

double pulseIn(mraa_gpio_context pin, int value, double timeout)
{
    struct timespec start, end;

    /* start clock to wait for pulse, or timeout */ 
    clock_gettime(CLOCK_MONOTONIC, &start);

    /* Wait for pin to reach desired pulse or time out. */ 
    while(value == LOW ? mraa_gpio_read(pin) == HIGH : mraa_gpio_read(pin) == LOW)
    {   
        clock_gettime(CLOCK_MONOTONIC, &end);
        if(duration(start, end) > timeout)
            return 0.0;
    }   

    clock_gettime(CLOCK_MONOTONIC, &start);

    while (value == mraa_gpio_read(pin));

    clock_gettime(CLOCK_MONOTONIC, &end);

    return duration(start, end);
     
}

double duration(struct timespec start, struct timespec end)
{
    return (double)((end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0);
}

void intHandler(int signalNum) 
{
    keepRunning = false;
}
