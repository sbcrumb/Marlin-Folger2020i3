/**
 * Marlin 3D Printer Firmware
 * Copyright (C) 2016 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (C) 2011 Camiel Gubbels / Erik van der Zalm
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

/**
 * temperature.h - temperature controller
 */

#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include "Marlin.h"
#include "planner.h"

#if ENABLED(PID_ADD_EXTRUSION_RATE)
  #include "stepper.h"
#endif

#ifndef SOFT_PWM_SCALE
  #define SOFT_PWM_SCALE 0
#endif

class Temperature {

  public:

    static int current_temperature_raw[EXTRUDERS];
    static float current_temperature[EXTRUDERS];
    static int target_temperature[EXTRUDERS];

    static int current_temperature_bed_raw;
    static float current_temperature_bed;
    static int target_temperature_bed;

    #if ENABLED(TEMP_SENSOR_1_AS_REDUNDANT)
      static float redundant_temperature;
    #endif

    static unsigned char soft_pwm_bed;

    #if ENABLED(FAN_SOFT_PWM)
      static unsigned char fanSpeedSoftPwm[FAN_COUNT];
    #endif

    #if ENABLED(PIDTEMP) || ENABLED(PIDTEMPBED)
      #define PID_dT ((OVERSAMPLENR * 12.0)/(F_CPU / 64.0 / 256.0))
    #endif

    #if ENABLED(PIDTEMP)

      #if ENABLED(PID_PARAMS_PER_EXTRUDER)

        static float Kp[EXTRUDERS], Ki[EXTRUDERS], Kd[EXTRUDERS];
        #if ENABLED(PID_ADD_EXTRUSION_RATE)
          static float Kc[EXTRUDERS];
        #endif
        #define PID_PARAM(param, e) Temperature::param[e]

      #else

        static float Kp, Ki, Kd;
        #if ENABLED(PID_ADD_EXTRUSION_RATE)
          static float Kc;
        #endif
        #define PID_PARAM(param, e) Temperature::param

      #endif // PID_PARAMS_PER_EXTRUDER

      // Apply the scale factors to the PID values
      #define scalePID_i(i)   ( (i) * PID_dT )
      #define unscalePID_i(i) ( (i) / PID_dT )
      #define scalePID_d(d)   ( (d) / PID_dT )
      #define unscalePID_d(d) ( (d) * PID_dT )

    #endif

    #if ENABLED(PIDTEMPBED)
      static float bedKp, bedKi, bedKd;
    #endif

    #if ENABLED(BABYSTEPPING)
      static volatile int babystepsTodo[3];
    #endif

    #if ENABLED(THERMAL_PROTECTION_HOTENDS) && WATCH_TEMP_PERIOD > 0
      static int watch_target_temp[EXTRUDERS];
      static millis_t watch_heater_next_ms[EXTRUDERS];
    #endif

    #if ENABLED(THERMAL_PROTECTION_HOTENDS) && WATCH_BED_TEMP_PERIOD > 0
      static int watch_target_bed_temp;
      static millis_t watch_bed_next_ms;
    #endif

    #if ENABLED(PREVENT_DANGEROUS_EXTRUDE)
      static float extrude_min_temp;
      static bool tooColdToExtrude(uint8_t e) { return degHotend(e) < extrude_min_temp; }
    #else
      static bool tooColdToExtrude(uint8_t e) { UNUSED(e); return false; }
    #endif

  private:

    #if ENABLED(TEMP_SENSOR_1_AS_REDUNDANT)
      static int redundant_temperature_raw;
      static float redundant_temperature;
    #endif

    static volatile bool temp_meas_ready;

    #if ENABLED(PIDTEMP)
      static float temp_iState[EXTRUDERS];
      static float temp_dState[EXTRUDERS];
      static float pTerm[EXTRUDERS];
      static float iTerm[EXTRUDERS];
      static float dTerm[EXTRUDERS];

      #if ENABLED(PID_ADD_EXTRUSION_RATE)
        static float cTerm[EXTRUDERS];
        static long last_position[EXTRUDERS];
        static long lpq[LPQ_MAX_LEN];
        static int lpq_ptr;
      #endif

      static float pid_error[EXTRUDERS];
      static float temp_iState_min[EXTRUDERS];
      static float temp_iState_max[EXTRUDERS];
      static bool pid_reset[EXTRUDERS];
    #endif

    #if ENABLED(PIDTEMPBED)
      static float temp_iState_bed;
      static float temp_dState_bed;
      static float pTerm_bed;
      static float iTerm_bed;
      static float dTerm_bed;
      static float pid_error_bed;
      static float temp_iState_min_bed;
      static float temp_iState_max_bed;
    #else
      static millis_t next_bed_check_ms;
    #endif

    static unsigned long raw_temp_value[4];
    static unsigned long raw_temp_bed_value;

    // Init min and max temp with extreme values to prevent false errors during startup
    static int minttemp_raw[EXTRUDERS];
    static int maxttemp_raw[EXTRUDERS];
    static int minttemp[EXTRUDERS];
    static int maxttemp[EXTRUDERS];

    #ifdef BED_MINTEMP
      static int bed_minttemp_raw;
    #endif

    #ifdef BED_MAXTEMP
      static int bed_maxttemp_raw;
    #endif

    #if ENABLED(FILAMENT_WIDTH_SENSOR)
      static int meas_shift_index;  // Index of a delayed sample in buffer
    #endif

    #if HAS_AUTO_FAN
      static millis_t next_auto_fan_check_ms;
    #endif

    static unsigned char soft_pwm[EXTRUDERS];

    #if ENABLED(FAN_SOFT_PWM)
      static unsigned char soft_pwm_fan[FAN_COUNT];
    #endif

    #if ENABLED(FILAMENT_WIDTH_SENSOR)
      static int current_raw_filwidth;  //Holds measured filament diameter - one extruder only
    #endif

  public:

    /**
     * Instance Methods
     */

    Temperature();

    void init();

    /**
     * Static (class) methods
     */
    static float analog2temp(int raw, uint8_t e);
    static float analog2tempBed(int raw);

    /**
     * Called from the Temperature ISR
     */
    static void isr();

    /**
     * Call periodically to manage heaters
     */
    static void manage_heater();

    #if ENABLED(FILAMENT_WIDTH_SENSOR)
      static float analog2widthFil(); // Convert raw Filament Width to millimeters
      static int widthFil_to_size_ratio(); // Convert raw Filament Width to an extrusion ratio
    #endif


    //high level conversion routines, for use outside of temperature.cpp
    //inline so that there is no performance decrease.
    //deg=degreeCelsius

    static float degHotend(uint8_t extruder) { return current_temperature[extruder]; }
    static float degBed() { return current_temperature_bed; }

    #if ENABLED(SHOW_TEMP_ADC_VALUES)
    static float rawHotendTemp(uint8_t extruder) { return current_temperature_raw[extruder]; }
    static float rawBedTemp() { return current_temperature_bed_raw; }
    #endif

    static float degTargetHotend(uint8_t extruder) { return target_temperature[extruder]; }
    static float degTargetBed() { return target_temperature_bed; }

    #if ENABLED(THERMAL_PROTECTION_HOTENDS) && WATCH_TEMP_PERIOD > 0
      static void start_watching_heater(int e = 0);
    #endif

    #if ENABLED(THERMAL_PROTECTION_BED) && WATCH_BED_TEMP_PERIOD > 0
      static void start_watching_bed();
    #endif

    static void setTargetHotend(const float& celsius, uint8_t extruder) {
      target_temperature[extruder] = celsius;
      #if ENABLED(THERMAL_PROTECTION_HOTENDS) && WATCH_TEMP_PERIOD > 0
        start_watching_heater(extruder);
      #endif
    }

    static void setTargetBed(const float& celsius) {
      target_temperature_bed = celsius;
      #if ENABLED(THERMAL_PROTECTION_BED) && WATCH_BED_TEMP_PERIOD > 0
        start_watching_bed();
      #endif
    }

    static bool isHeatingHotend(uint8_t extruder) { return target_temperature[extruder] > current_temperature[extruder]; }
    static bool isHeatingBed() { return target_temperature_bed > current_temperature_bed; }

    static bool isCoolingHotend(uint8_t extruder) { return target_temperature[extruder] < current_temperature[extruder]; }
    static bool isCoolingBed() { return target_temperature_bed < current_temperature_bed; }

    /**
     * The software PWM power for a heater
     */
    static int getHeaterPower(int heater);

    /**
     * Switch off all heaters, set all target temperatures to 0
     */
    static void disable_all_heaters();

    /**
     * Perform auto-tuning for hotend or bed in response to M303
     */
    #if HAS_PID_HEATING
      static void PID_autotune(float temp, int extruder, int ncycles, bool set_result=false);
    #endif

    /**
     * Update the temp manager when PID values change
     */
    static void updatePID();

    static void autotempShutdown() {
      #if ENABLED(AUTOTEMP)
        if (planner.autotemp_enabled) {
          planner.autotemp_enabled = false;
          if (degTargetHotend(active_extruder) > planner.autotemp_min)
            setTargetHotend(0, active_extruder);
        }
      #endif
    }

    #if ENABLED(BABYSTEPPING)

      static void babystep_axis(AxisEnum axis, int distance) {
        #if ENABLED(COREXY) || ENABLED(COREXZ) || ENABLED(COREYZ)
          #if ENABLED(BABYSTEP_XY)
            switch (axis) {
              case CORE_AXIS_1: // X on CoreXY and CoreXZ, Y on CoreYZ
                babystepsTodo[CORE_AXIS_1] += distance * 2;
                babystepsTodo[CORE_AXIS_2] += distance * 2;
                break;
              case CORE_AXIS_2: // Y on CoreXY, Z on CoreXZ and CoreYZ
                babystepsTodo[CORE_AXIS_1] += distance * 2;
                babystepsTodo[CORE_AXIS_2] -= distance * 2;
                break;
              case NORMAL_AXIS: // Z on CoreXY, Y on CoreXZ, X on CoreYZ
                babystepsTodo[NORMAL_AXIS] += distance;
                break;
            }
          #elif ENABLED(COREXZ) || ENABLED(COREYZ)
            // Only Z stepping needs to be handled here
            babystepsTodo[CORE_AXIS_1] += distance * 2;
            babystepsTodo[CORE_AXIS_2] -= distance * 2;
          #else
            babystepsTodo[Z_AXIS] += distance;
          #endif
        #else
          babystepsTodo[axis] += distance;
        #endif
      }

    #endif // BABYSTEPPING

  private:

    static void set_current_temp_raw();

    static void updateTemperaturesFromRawValues();

    #if ENABLED(HEATER_0_USES_MAX6675)
      static int read_max6675();
    #endif

    static void checkExtruderAutoFans();

    static float get_pid_output(int e);

    #if ENABLED(PIDTEMPBED)
      static float get_pid_output_bed();
    #endif

    static void _temp_error(int e, const char* serial_msg, const char* lcd_msg);
    static void min_temp_error(uint8_t e);
    static void max_temp_error(uint8_t e);

    #if ENABLED(THERMAL_PROTECTION_HOTENDS) || HAS_THERMALLY_PROTECTED_BED

      typedef enum TRState { TRInactive, TRFirstHeating, TRStable, TRRunaway } TRstate;

      static void thermal_runaway_protection(TRState* state, millis_t* timer, float temperature, float target_temperature, int heater_id, int period_seconds, int hysteresis_degc);

      #if ENABLED(THERMAL_PROTECTION_HOTENDS)
        static TRState thermal_runaway_state_machine[EXTRUDERS];
        static millis_t thermal_runaway_timer[EXTRUDERS];
      #endif

      #if HAS_THERMALLY_PROTECTED_BED
        static TRState thermal_runaway_bed_state_machine;
        static millis_t thermal_runaway_bed_timer;
      #endif

    #endif // THERMAL_PROTECTION

};

extern Temperature thermalManager;

#endif // TEMPERATURE_H
