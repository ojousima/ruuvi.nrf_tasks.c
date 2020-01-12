#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include "ruuvi_interface_log.h"
#include "task_sensor.h"

#include <string.h>

#ifndef TASK_SENSOR_LOG_LEVEL
#define TASK_SENSOR_LOG_LEVEL RUUVI_INTERFACE_LOG_INFO
#endif

static inline void LOG (const char * const msg)
{
    ruuvi_interface_log (TASK_SENSOR_LOG_LEVEL, msg);
}

static inline void LOGD (const char * const msg)
{
    ruuvi_interface_log (RUUVI_INTERFACE_LOG_DEBUG, msg);
}

static inline void LOGHEX (const char * const msg, const size_t len)
{
    ruuvi_interface_log_hex(TASK_SENSOR_LOG_LEVEL, msg, len):
}

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
ruuvi_driver_status_t rt_sensor_initialize (rt_sensor_ctx_t * const sensor)
{
    ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;

    if((NULL == sensor) || (NULL == sensor->init))
    {
        err_code |= RUUVI_DRIVER_ERROR_NULL;
    }
    else
    {
        err_code = sensor->init (&(sensor->sensor), sensor->bus, sensor->handle);
    }
    
    return err_code;
}

/** @brief Store the sensor state to NVM.
 *
 * @param[in] sensor Sensor to store.
 *
 * @return RUUVI_DRIVER_SUCCESS on success.
 * @return RUUVI_DRIVER_ERROR_NULL if sensor is NULL.
 * @return error code from sensor on other error.
 */
ruuvi_driver_status_t rt_sensor_store (rt_sensor_ctx_t * const sensor)
{
    if(NULL == sensor)
    {
        err_code |= RUUVI_DRIVER_ERROR_NULL;
    }
    else if (task_flash_busy())
    {
        err_code |= RUUVI_DRIVER_ERROR_BUSY;
    }
    else
    {
        err_code |= task_flash_store (sensor->nvm_file, sensor->nvm_record,
                                      &(sensor->configuration),
                                      sizeof (sensor->configuration));
    }
    return err_code;
}

/** @brief Load the sensor state from NVM.
 *
 * @param[in] sensor Sensor to store.
 *
 * @return RUUVI_DRIVER_SUCCESS on success.
 * @return RUUVI_DRIVER_ERROR_NULL if sensor is NULL.
 * @return error code from sensor on other error.
 */
ruuvi_driver_status_t rt_sensor_load (rt_sensor_ctx_t * const sensor)
{
    ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;

    if(NULL == sensor)
    {
        err_code |= RUUVI_DRIVER_ERROR_NULL;
    }
    else if (task_flash_busy())
    {
        err_code |= RUUVI_DRIVER_ERROR_BUSY;
    }
    else
    {
        err_code |= task_flash_load (sensor->nvm_file, sensor->nvm_record,
                                     &(sensor->configuration),
                                     sizeof (sensor->configuration));
    }
    return err_code;
}

/** @brief Configure a sensor with given settings.
 *
 * @param[in,out] sensor In: Sensor to configure. 
                         Out: Sensor->configuration will be set to actual configuration.
 *
 * @return RUUVI_DRIVER_SUCCESS on success.
 * @return RUUVI_DRIVER_ERROR_NULL if sensor is NULL.
 * @return error code from sensor on other error.
 */
ruuvi_driver_status_t rt_sensor_configure (rt_sensor_ctx_t * const sensor)
{
    ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
    if (NULL == sensor)
    {
        err_code |= RUUVI_DRIVER_ERROR_NULL;
    }
    else if(NULL == sensor->sensor->configuration_set)
    {
        err_code |= RUUVI_DRIVER_ERROR_INVALID_STATE;
    }
    else
    {
    LOG ("\r\nAttempting to configure ");
    LOG (sensor->sensor->name);
    LOG (" with:\r\n");
    ruuvi_interface_log_sensor_configuration (TASK_SENSOR_LOG_LEVEL, 
        sensor->configuration);
    err_code |= sensor->sensor->configuration_set (sensor->sensor, sensor->configuration);
    LOG ("Actual configuration:\r\n");
    ruuvi_interface_log_sensor_configuration (TASK_SENSOR_LOG_LEVEL, 
        sensor->configuration);
    }
    return err_code;
}

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
rt_sensor_ctx_t * rt_sensor_find_backend (rt_sensor_ctx_t * const sensor_list,
        const size_t count, const char * const name)
{
    ruuvi_driver_sensor_t * p_sensor = NULL;

    for (size_t ii = 0; (count > ii) && (NULL == p_sensor); ii++)
    {
        if (0 == strcmp (sensor_list[ii].sensor.name, name))
        {
            p_sensor = & (sensor_list[ii]);
        }
    }

    return p_sensor;
}

/**
 * @brief Search for a sensor which can provide requested values
 *
 * @param[in] sensor_list Array of sensors to search the backend from.
 * @param[in] count Number of sensor backends in the list.
 * @param[in] values Fields which sensor must provide. 
 * @return Pointer to requested sensor if found. If there are many candidates, first is 
 *         returned
 * @return NULL if requested sensor was not found.
 */
ruuvi_driver_sensor_t * rt_sensor_find_provider (ruuvi_driver_sensor_t * const
        sensor_list, const size_t count, ruuvi_driver_sensor_data_fields_t values)
{
    ruuvi_driver_sensor_t * p_sensor = NULL;

    for (size_t ii = 0; (count > ii) && (NULL == p_sensor); ii++)
    {
        if ((values & sensor_list[ii].sensor.provides) == values)
        {
            p_sensor = & (sensor_list[ii]);
        }
    }

    return p_sensor;
}
