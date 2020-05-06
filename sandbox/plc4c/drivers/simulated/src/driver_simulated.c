/*
  Licensed to the Apache Software Foundation (ASF) under one
  or more contributor license agreements.  See the NOTICE file
  distributed with this work for additional information
  regarding copyright ownership.  The ASF licenses this file
  to you under the Apache License, Version 2.0 (the
  "License"); you may not use this file except in compliance
  with the License.  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing,
  software distributed under the License is distributed on an
  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
  KIND, either express or implied.  See the License for the
  specific language governing permissions and limitations
  under the License.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <plc4c/spi/types_private.h>
#include <plc4c/driver_simulated.h>

enum plc4c_driver_simulated_field_type_t {
    RANDOM,
    STATE,
    STDOUT
};
typedef enum plc4c_driver_simulated_field_type_t plc4c_driver_simulated_field_type;

typedef enum plc4c_driver_simulated_field_datatype_t plc4c_driver_simulated_field_datatype;

// State definitions
enum read_states {
    READ_INIT,
    READ_FINISHED
};

enum write_states {
    WRITE_INIT,
    WRITE_FINISHED
};

struct plc4c_driver_simulated_item_t {
    char *name;
    plc4c_driver_simulated_field_type type;
    plc4c_data_type data_type;
    int num_elements;
};
typedef struct plc4c_driver_simulated_item_t plc4c_driver_simulated_item;

plc4c_return_code plc4c_driver_simulated_connect_machine_function(plc4c_system_task *task) {
    plc4c_connection *connection = task->context;
    if (connection == NULL) {
        return INTERNAL_ERROR;
    }
    if (connection->connected) {
        return INTERNAL_ERROR;
    }
    connection->connected = true;
    task->completed = true;
    return OK;
}

plc4c_return_code plc4c_driver_simulated_disconnect_machine_function(plc4c_system_task *task) {
    plc4c_connection *connection = task->context;
    if (connection == NULL) {
        return INTERNAL_ERROR;
    }
    if (!connection->connected) {
        return INTERNAL_ERROR;
    }
    connection->connected = false;
    task->completed = true;
    return OK;
}

plc4c_return_code plc4c_driver_simulated_read_machine_function(plc4c_system_task *task) {
    if (task->context == NULL) {
        return INTERNAL_ERROR;
    }

    plc4c_read_request_execution *read_request_execution = task->context;
    plc4c_read_request *read_request = read_request_execution->read_request;
    switch (task->state_id) {
        case READ_INIT: {
            // Create a response.
            plc4c_read_response *read_response = malloc(sizeof(plc4c_read_response));
            read_response->read_request = read_request;
            plc4c_utils_list_create(&(read_response->items));

            // Process every field in the request.
            plc4c_list_element *cur_element = plc4c_utils_list_head(read_request->items);
            while (cur_element != NULL) {
                plc4c_driver_simulated_item *cur_item = cur_element->value;
                plc4c_response_value_item *value_item = malloc(sizeof(plc4c_response_value_item));
                value_item->item = (plc4c_item *) cur_item;

                /*
                 * create the plc4c_data
                 * if this were a custom type we could set a custom destroy method
                 * we can also set a custom printf method
                 * right , now just create a new random value
                 */
                value_item->value = plc4c_data_create_uint_data(arc4random());
                value_item->response_code = PLC4C_RESPONSE_CODE_OK;

                // Add the value to the response.
                plc4c_utils_list_insert_tail_value(read_response->items, value_item);
                cur_element = cur_element->next;
            }

            read_request_execution->read_response = read_response;
            task->state_id = READ_FINISHED;
            task->completed = true;
            break;
        }
        case READ_FINISHED: {
            // Do nothing
            break;
        }
        default: {
            return INTERNAL_ERROR;
        }
    }
    return OK;
}

plc4c_return_code plc4c_driver_simulated_write_machine_function(plc4c_system_task *task) {
    if (task->context == NULL) {
        return INTERNAL_ERROR;
    }

    plc4c_write_request_execution *write_request_execution = task->context;
    plc4c_write_request *write_request = write_request_execution->write_request;
    switch (task->state_id) {
        case WRITE_INIT: {
            // Create a response.
            plc4c_write_response *write_response = malloc(sizeof(plc4c_write_response));
            write_response->write_request = write_request;
            plc4c_utils_list_create(&(write_response->response_items));

            // Process every field in the request.
            plc4c_list_element *cur_element = plc4c_utils_list_head(write_request->items);
            while (cur_element != NULL) {
                plc4c_request_value_item *cur_value_item = cur_element->value;
                plc4c_driver_simulated_item *cur_item = (plc4c_driver_simulated_item *) cur_value_item->item;
                plc4c_data *write_data = cur_value_item->value;
                plc4c_response_code response_code = -1;
                switch (cur_item->type) {
                    case STDOUT: {
                        printf("");
                        if (cur_item->data_type != write_data->data_type) {
                            printf("--> Simulated Driver Write: Value is %s but Item type is %s",
                                   plc4c_data_type_name(write_data->data_type),
                                   plc4c_data_type_name(cur_item->data_type));
                            response_code = PLC4C_RESPONSE_CODE_INVALID_DATATYPE;
                            break;
                        }
                        printf("--> Simulated Driver Write: Value (%s) %s: ",
                               plc4c_data_type_name(write_data->data_type),
                               cur_item->name);
                        plc4c_data_printf(write_data);
                        printf("\n");
                        response_code = PLC4C_RESPONSE_CODE_OK;
                        break;
                    }
                    default: {
                        response_code = PLC4C_RESPONSE_CODE_INVALID_ADDRESS;
                        break;
                    }
                }

                // Create a response element and add that to the response ...
                plc4c_response_item *response_item = malloc(sizeof(plc4c_response_item));
                response_item->item = (plc4c_item *) cur_item;
                response_item->response_code = response_code;
                plc4c_utils_list_insert_tail_value(write_response->response_items, response_item);

                cur_element = cur_element->next;
            }

            write_request_execution->write_response = write_response;
            task->state_id = WRITE_FINISHED;
            task->completed = true;
            break;
        }
        case WRITE_FINISHED: {
            // Do nothing
            break;
        }
        default: {
            return INTERNAL_ERROR;
        }
    }
    return OK;
}

plc4c_item *plc4c_driver_simulated_parse_address(char *address_string) {
    plc4c_driver_simulated_field_type type = RANDOM;
    char *name = NULL;
    plc4c_data_type data_type = -1;
    int num_elements = 0;
    int start_segment_index = 0;
    char *start_segment = address_string;
    for (int i = 0; i <= strlen(address_string); i++) {
        // This marks the end of the type part.
        if (*(address_string + i) == '/') {
            char *type_str = malloc(sizeof(char) * (i + 1));
            strlcpy(type_str, start_segment, i + 1);
            if (strcmp(type_str, "RANDOM") == 0) {
                type = RANDOM;
            } else if (strcmp(type_str, "STATE") == 0) {
                type = STATE;
            } else if (strcmp(type_str, "STDOUT") == 0) {
                type = STDOUT;
            } else {
                return NULL;
            }
            start_segment = address_string + i + 1;
            start_segment_index = i + 1;
        }
        // This marks the end of the name part.
        if (*(address_string + i) == ':') {
            name = malloc(sizeof(char) * ((i - start_segment_index) + 1));
            strlcpy(name, start_segment, (i - start_segment_index) + 1);
            start_segment = address_string + i + 1;
            start_segment_index = i + 1;
        }
        // This marks the end of the data-type part if there is a size coming in addition.
        if ((i == strlen(address_string)) || (*(address_string + i) == '[')) {
            char *datatype_name = malloc(sizeof(char) * ((i - start_segment_index) + 1));
            strlcpy(datatype_name, start_segment, (i - start_segment_index) + 1);

            // Translate the string into a constant.
            if (strcmp(datatype_name, "INTEGER") == 0) {
                data_type = PLC4C_INT;
            } else if (strcmp(datatype_name, "STRING") == 0) {
                data_type = PLC4C_CONSTANT_STRING;
            } else {
                return NULL;
            }

            start_segment = address_string + i + 1;
            start_segment_index = i + 1;
            if (i == strlen(address_string)) {
                num_elements = 1;
                break;
            }
        }
        // This marks the end of the size part.
        if (*(address_string + i) == ']') {
            char *num_elements_str = malloc(sizeof(char) * ((i - start_segment_index) + 1));
            strlcpy(num_elements_str, start_segment, (i - start_segment_index) + 1);
            char *success = NULL;
            num_elements = (int) strtol(num_elements_str, &success, 10);
            break;
        }
    }

    // Create a new driver specific item.
    plc4c_driver_simulated_item *item = (plc4c_driver_simulated_item *) malloc(sizeof(plc4c_driver_simulated_item));
    item->type = type;
    item->name = name;
    item->data_type = data_type;
    item->num_elements = num_elements;

    return (plc4c_item *) item;
}


plc4c_return_code plc4c_driver_simulated_connect_function(plc4c_connection *connection,
                                                          plc4c_system_task **task) {
    plc4c_system_task *new_task = malloc(sizeof(plc4c_system_task));
    new_task->context = connection;
    // There's nothing to do here, so no need for a state-machine.
    new_task->state_id = -1;
    new_task->state_machine_function = &plc4c_driver_simulated_connect_machine_function;
    new_task->connection = connection;
    *task = new_task;
    return OK;
}

plc4c_return_code plc4c_driver_simulated_disconnect_function(plc4c_connection *connection,
                                                             plc4c_system_task **task) {
    plc4c_system_task *new_task = malloc(sizeof(plc4c_system_task));
    new_task->context = connection;
    // There's nothing to do here, so no need for a state-machine.
    new_task->state_id = -1;
    new_task->state_machine_function = &plc4c_driver_simulated_disconnect_machine_function;
    new_task->connection = connection;
    *task = new_task;
    return OK;
}

plc4c_return_code plc4c_driver_simulated_read_function(plc4c_connection *connection,
                                                       plc4c_system_task **task) {
    plc4c_system_task *new_task = malloc(sizeof(plc4c_system_task));
    new_task->state_id = READ_INIT;
    new_task->state_machine_function = &plc4c_driver_simulated_read_machine_function;
    new_task->completed = false;
    new_task->connection = connection;
    *task = new_task;
    return OK;
}

plc4c_return_code plc4c_driver_simulated_write_function(plc4c_connection *connection,
                                                        plc4c_system_task **task) {
    plc4c_system_task *new_task = malloc(sizeof(plc4c_system_task));
    new_task->state_id = WRITE_INIT;
    new_task->state_machine_function = &plc4c_driver_simulated_write_machine_function;
    new_task->completed = false;
    new_task->connection = connection;
    *task = new_task;
    return OK;
}

void free_read_response_item(plc4c_list_element *read_item_element) {
    plc4c_response_value_item *value_item = (plc4c_response_value_item *) read_item_element->value;
    plc4c_data_delete(value_item->value);
    value_item->value = NULL;
}

void plc4c_driver_simulated_free_read_response(plc4c_read_response *response) {
    // the request will be cleaned up elsewhere
    plc4c_utils_list_delete_elements(response->items, &free_read_response_item);
}

void free_write_response_item(plc4c_list_element *write_item_element) {
    plc4c_response_value_item *value_item = (plc4c_response_value_item *) write_item_element->value;
    // do not delete the plc4c_item
    // we also, in THIS case don't delete the random value which isn't really
    // a pointer
    //free(value_item->value);
    value_item->value = NULL;
}

void plc4c_driver_simulated_free_write_response(plc4c_write_response *response) {
    // the request will be cleaned up elsewhere
    plc4c_utils_list_delete_elements(response->response_items, &free_write_response_item);
}


plc4c_driver *plc4c_driver_simulated_create() {
    plc4c_driver *driver = (plc4c_driver *) malloc(sizeof(plc4c_driver));
    driver->protocol_code = "simulated";
    driver->protocol_name = "Simulated PLC4X Datasource";
    driver->default_transport_code = "dummy";
    driver->parse_address_function = &plc4c_driver_simulated_parse_address;
    driver->connect_function = &plc4c_driver_simulated_connect_function;
    driver->disconnect_function = &plc4c_driver_simulated_disconnect_function;
    driver->read_function = &plc4c_driver_simulated_read_function;
    driver->write_function = &plc4c_driver_simulated_write_function;
    driver->free_read_response_function = &plc4c_driver_simulated_free_read_response;
    driver->free_write_response_function = &plc4c_driver_simulated_free_write_response;
    return driver;
}
