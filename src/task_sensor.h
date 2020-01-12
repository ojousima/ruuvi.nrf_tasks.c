#ifndef TASK_SENSOR_H
#define TASK_SENSOR_H
/**
 * @defgroup sensor_tasks Sensor tasks
 */
/*@{*/
/**
 * @file task_sensor.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2019-12-25
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Helper functions common to all sensors
 *
 */
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include "ruuvi_endpoints.h"

typedef struct {
  ruuvi_driver_sensor_t sensor,                       //!< Control structure for sensor.
  ruuvi_driver_sensor_init_fp init,                   //!< Initialization function.
  ruuvi_driver_sensor_configuration_t configuration,  //!< Sensor configuration.
  uint16_t nvm_file,                                  //!< NVM file of configuration.
  uint16_t nvm_record,                                //!< NVM record of configuration.
  uint8_t  sensor_handle,                             //!< Handle of sensor.
  ruuvi_driver_bus_t sensor_bus                       //!< Bus of sensor. 
}rt_sensor_ctx_t;

/** @brief Initialize sensor CTX
 *
 * To initialize a sensor, initialization function, sensor bus and sensor handle must
 * be set. After initialization, sensor control structure is ready to use,
 * initial configuration is set to actual values on sensor. 
 *
 * To configure the sensor, set the sensor configuration in struct and call
 * @ref rt_sensor_configure.
 *
 * @param[in] sensor Sensor to initialize.
 *
 * @return RUUVI_DRIVER_SUCCESS on success.
 * @return RUUVI_DRIVER_ERROR_NULL if sensor is NULL.
 * @return error code from sensor on other error.
 */
ruuvi_driver_status_t rt_sensor_initialize (rt_sensor_ctx_t * const sensor);

/** @brief Store the sensor state to NVM.
 *
 * @param[in] sensor Sensor to store.
 *
 * @return RUUVI_DRIVER_SUCCESS on success.
 * @return RUUVI_DRIVER_ERROR_NULL if sensor is NULL.
 * @return error code from sensor on other error.
 */
ruuvi_driver_status_t rt_sensor_store (rt_sensor_ctx_t * const sensor);

/** @brief Load the sensor state from NVM.
 *
 * @param[in] sensor Sensor to store.
 *
 * @return RUUVI_DRIVER_SUCCESS on success.
 * @return RUUVI_DRIVER_ERROR_NULL if sensor is NULL.
 * @return error code from sensor on other error.
 */
ruuvi_driver_status_t rt_sensor_load (rt_sensor_ctx_t * const sensor);

/** @brief Configure a sensor with given settings.
 *
 * @param[in,out] sensor In: Sensor to configure. 
                         Out: Sensor->configuration will be set to actual configuration.
 *
 * @return RUUVI_DRIVER_SUCCESS on success.
 * @return RUUVI_DRIVER_ERROR_NULL if sensor is NULL.
 * @return error code from sensor on other error.
 */
ruuvi_driver_status_t rt_sensor_configure (rt_sensor_ctx_t * const sensor);

/**
 * @brief Read sensors and encode to given buffer in Ruuvi DF5.
 *
 * @param[in] buffer uint8_t array with length of 24 bytes.
 * @return RUUVI_DRIVER_SUCCESS if data was encoded
 */
//ruuvi_endpoint_status_t task_sensor_encode_to_5 (uint8_t * const buffer);

/**
 * @brief Search for requested sensor backend in given list of sensors.
 *
 * @param[in] sensor_list Array of sensors to search the backend from.
 * @param[in] count Number of sensor backends in the list.
 * @param[in] name NULL-terminated, max 9-byte (including trailing NULL) string 
 *                 representation of sensor.
 * @return pointer to requested sensor CTX if found
 * @return NULL if requested sensor was not found
 */
rt_sensor_ctx_t * rt_sensor_find_backend (rt_sensor_ctx_t * const
        sensor_list, const size_t count, const char * const name);

/**
 * @brief Search for a sensor which can provide requested values
 *
 * @param[in] sensor_list Array of sensors to search the backend from.
 * @param[in] count Number of sensor backends in the list.
 * @param[in] values Fields which sensor must provide. 
 * @return Pointer to requested sensor CTXif found. If there are many candidates, first is 
 *         returned
 * @return NULL if requested sensor was not found.
 */
rt_sensor_ctx_t * rt_sensor_find_provider (ruuvi_driver_sensor_t * const
        sensor_list, const size_t count, ruuvi_driver_sensor_data_fields_t values);

/*@}*/
#endif
